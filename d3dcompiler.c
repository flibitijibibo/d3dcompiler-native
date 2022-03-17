/* d3dcompiler-native - Wine d3dcompiler Repurposed for Native Applications
 * Copyright (c) 2022 Ethan "flibitijibibo" Lee
 * Copyright (c) 2009 CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#define COBJMACROS
#include <d3dcommon.h>
#include <vkd3d_shader.h>
#include <stdlib.h> /* malloc, free */

#define D3DCOMPILE_DEBUG 0x00000001

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define TRACE(fmt, ...)
#define WARN(fmt, ...)
#define FIXME(fmt, ...)

/* ID3DBlob Implementation */

typedef struct CompilerBlob
{
    ID3D10BlobVtbl* lpVtbl;
    ULONG refcount;
    LPVOID blob;
    SIZE_T size;
} CompilerBlob;

static HRESULT STDMETHODCALLTYPE CompilerBlob_QueryInterface(
    ID3D10Blob *This,
    REFIID riid,
    void **ppvObject
) {
    return S_OK; /* FIXME */
}

static ULONG STDMETHODCALLTYPE CompilerBlob_AddRef(ID3D10Blob *This)
{
    CompilerBlob *blob = (CompilerBlob*) This;
    return ++blob->refcount;
}

static ULONG STDMETHODCALLTYPE CompilerBlob_Release(ID3D10Blob *This)
{
    CompilerBlob *blob = (CompilerBlob*) This;
    if (blob->refcount > 1)
    {
        return --blob->refcount;
    }
    free(blob->blob);
    free(blob);
    return 0;
}

static void* STDMETHODCALLTYPE CompilerBlob_GetBufferPointer(ID3D10Blob *This)
{
    CompilerBlob *blob = (CompilerBlob*) This;
    return blob->blob;
}

static SIZE_T STDMETHODCALLTYPE CompilerBlob_GetBufferSize(ID3D10Blob *This)
{
    CompilerBlob *blob = (CompilerBlob*) This;
    return blob->size;
}

static HRESULT D3DCreateBlob(SIZE_T Size, ID3DBlob **ppBlob)
{
    CompilerBlob *blob;

    if (ppBlob == NULL)
        return E_INVALIDARG;

    blob = (CompilerBlob*) malloc(sizeof(CompilerBlob));
    if (blob == NULL)
        return E_OUTOFMEMORY;

    /* ID3DBlob */
    blob->lpVtbl->QueryInterface = CompilerBlob_QueryInterface;
    blob->lpVtbl->AddRef = CompilerBlob_AddRef;
    blob->lpVtbl->Release = CompilerBlob_Release;
    blob->lpVtbl->GetBufferPointer = CompilerBlob_GetBufferPointer;
    blob->lpVtbl->GetBufferSize = CompilerBlob_GetBufferSize;

    /* CompilerBlob */
    blob->refcount = 1;
    blob->blob = malloc(Size);
    if (blob->blob == NULL)
        return E_OUTOFMEMORY;
    blob->size = Size;

    *ppBlob = (ID3DBlob*) blob;
    return S_OK;
}

/* Wine's D3DCompile implementation */

static HRESULT hresult_from_vkd3d_result(int vkd3d_result)
{
    switch (vkd3d_result)
    {
        case VKD3D_OK:
            return S_OK;
        case VKD3D_ERROR_INVALID_SHADER:
            WARN("Invalid shader bytecode.\n");
            /* fall-through */
        case VKD3D_ERROR:
            return E_FAIL;
        case VKD3D_ERROR_OUT_OF_MEMORY:
            return E_OUTOFMEMORY;
        case VKD3D_ERROR_INVALID_ARGUMENT:
            return E_INVALIDARG;
        case VKD3D_ERROR_NOT_IMPLEMENTED:
            return E_NOTIMPL;
        default:
            FIXME("Unhandled vkd3d result %d.\n", vkd3d_result);
            return E_FAIL;
    }
}

HRESULT WINAPI D3DCompile2(const void *data, SIZE_T data_size, const char *filename,
        const D3D_SHADER_MACRO *macros, ID3DInclude *include, const char *entry_point,
        const char *profile, UINT flags, UINT effect_flags, UINT secondary_flags,
        const void *secondary_data, SIZE_T secondary_data_size, ID3DBlob **shader_blob,
        ID3DBlob **messages_blob)
{
#if 0 /* FIXME: Include support */
    struct d3dcompiler_include_from_file include_from_file;
#endif
    struct vkd3d_shader_preprocess_info preprocess_info;
    struct vkd3d_shader_hlsl_source_info hlsl_info;
    struct vkd3d_shader_compile_option options[2];
    struct vkd3d_shader_compile_info compile_info;
    struct vkd3d_shader_compile_option *option;
    struct vkd3d_shader_code byte_code;
    const D3D_SHADER_MACRO *macro;
    size_t profile_len, i;
    char *messages;
    HRESULT hr;
    int ret;

    static const char * const d3dbc_profiles[] =
    {
        "fx_2_",

        "ps.1.",
        "ps.2.",
        "ps.3.",

        "ps_1_",
        "ps_2_",
        "ps_3_",

        "vs.1.",
        "vs.2.",
        "vs.3.",

        "vs_1_",
        "vs_2_",
        "vs_3_",

        "tx_1_",
    };

    TRACE("data %p, data_size %Iu, filename %s, macros %p, include %p, entry_point %s, "
            "profile %s, flags %#x, effect_flags %#x, secondary_flags %#x, secondary_data %p, "
            "secondary_data_size %Iu, shader_blob %p, messages_blob %p.\n",
            data, data_size, debugstr_a(filename), macros, include, debugstr_a(entry_point),
            debugstr_a(profile), flags, effect_flags, secondary_flags, secondary_data,
            secondary_data_size, shader_blob, messages_blob);

#if 0 /* FIXME: Include support */
    if (include == D3D_COMPILE_STANDARD_FILE_INCLUDE)
    {
        include_from_file.ID3DInclude_iface.lpVtbl = &d3dcompiler_include_from_file_vtbl;
        include = &include_from_file.ID3DInclude_iface;
    }
#endif

    if (flags & ~D3DCOMPILE_DEBUG)
        FIXME("Ignoring flags %#x.\n", flags);
    if (effect_flags)
        FIXME("Ignoring effect flags %#x.\n", effect_flags);
    if (secondary_flags)
        FIXME("Ignoring secondary flags %#x.\n", secondary_flags);

    if (messages_blob)
        *messages_blob = NULL;

    option = &options[0];
    option->name = VKD3D_SHADER_COMPILE_OPTION_API_VERSION;
    option->value = VKD3D_SHADER_API_VERSION_1_3;

    compile_info.type = VKD3D_SHADER_STRUCTURE_TYPE_COMPILE_INFO;
    compile_info.next = &preprocess_info;
    compile_info.source.code = data;
    compile_info.source.size = data_size;
    compile_info.source_type = VKD3D_SHADER_SOURCE_HLSL;
    compile_info.target_type = VKD3D_SHADER_TARGET_DXBC_TPF;
    compile_info.options = options;
    compile_info.option_count = 1;
    compile_info.log_level = VKD3D_SHADER_LOG_INFO;
    compile_info.source_name = filename;

    profile_len = strlen(profile);
    for (i = 0; i < ARRAY_SIZE(d3dbc_profiles); ++i)
    {
        size_t len = strlen(d3dbc_profiles[i]);

        if (len <= profile_len && !memcmp(profile, d3dbc_profiles[i], len))
        {
            compile_info.target_type = VKD3D_SHADER_TARGET_D3D_BYTECODE;
            break;
        }
    }

    preprocess_info.type = VKD3D_SHADER_STRUCTURE_TYPE_PREPROCESS_INFO;
    preprocess_info.next = &hlsl_info;
    preprocess_info.macros = (const struct vkd3d_shader_macro *)macros;
    preprocess_info.macro_count = 0;
    if (macros)
    {
        for (macro = macros; macro->Name; ++macro)
            ++preprocess_info.macro_count;
    }
#if 0 /* FIXME: Include support */
    preprocess_info.pfn_open_include = open_include;
    preprocess_info.pfn_close_include = close_include;
    preprocess_info.include_context = include;
#else
    preprocess_info.pfn_open_include = NULL;
    preprocess_info.pfn_close_include = NULL;
    preprocess_info.include_context = NULL;
#endif

    hlsl_info.type = VKD3D_SHADER_STRUCTURE_TYPE_HLSL_SOURCE_INFO;
    hlsl_info.next = NULL;
    hlsl_info.profile = profile;
    hlsl_info.entry_point = entry_point;
    hlsl_info.secondary_code.code = secondary_data;
    hlsl_info.secondary_code.size = secondary_data_size;

    if (!(flags & D3DCOMPILE_DEBUG))
    {
        option = &options[compile_info.option_count++];
        option->name = VKD3D_SHADER_COMPILE_OPTION_STRIP_DEBUG;
        option->value = true;
    }

    ret = vkd3d_shader_compile(&compile_info, &byte_code, &messages);
    if (messages)
    {
        if (messages_blob)
        {
            size_t size = strlen(messages);
            if (FAILED(hr = D3DCreateBlob(size, messages_blob)))
            {
                vkd3d_shader_free_messages(messages);
                vkd3d_shader_free_shader_code(&byte_code);
                return hr;
            }
            memcpy(ID3D10Blob_GetBufferPointer(*messages_blob), messages, size);
        }
        else
            vkd3d_shader_free_messages(messages);
    }

    if (!ret)
    {
        if (FAILED(hr = D3DCreateBlob(byte_code.size, shader_blob)))
        {
            vkd3d_shader_free_shader_code(&byte_code);
            return hr;
        }
        memcpy(ID3D10Blob_GetBufferPointer(*shader_blob), byte_code.code, byte_code.size);
    }

    return hresult_from_vkd3d_result(ret);
}

HRESULT WINAPI D3DCompile(const void *data, SIZE_T data_size, const char *filename,
        const D3D_SHADER_MACRO *defines, ID3DInclude *include, const char *entrypoint,
        const char *target, UINT sflags, UINT eflags, ID3DBlob **shader, ID3DBlob **error_messages)
{
    TRACE("data %p, data_size %Iu, filename %s, defines %p, include %p, entrypoint %s, "
            "target %s, sflags %#x, eflags %#x, shader %p, error_messages %p.\n",
            data, data_size, debugstr_a(filename), defines, include, debugstr_a(entrypoint),
            debugstr_a(target), sflags, eflags, shader, error_messages);

    return D3DCompile2(data, data_size, filename, defines, include, entrypoint, target, sflags,
            eflags, 0, NULL, 0, shader, error_messages);
}
