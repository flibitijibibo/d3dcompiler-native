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

#ifdef SPRITEBATCHTEST

/* Fake D3DCompile for SpriteBatchTest */

const char *fakevertexsrc =
"cbuffer ShaderFunction3_Uniforms : register(b0)\n"
"{\n"
"	float4 uniforms_float4[4];\n"
"};\n"
"\n"
"struct ShaderFunction3_Input\n"
"{\n"
"	float4 m_v0 : COLOR0;\n"
"	float4 m_v1 : TEXCOORD0;\n"
"	float4 m_v2 : POSITION0;\n"
"};\n"
"\n"
"struct ShaderFunction3_Output\n"
"{\n"
"	float4 m_oD0 : COLOR0;\n"
"	float4 m_oT0 : TEXCOORD0;\n"
"	float4 m_oPos : SV_Position;\n"
"};\n"
"\n"
"ShaderFunction3_Output ShaderFunction3(ShaderFunction3_Input input)\n"
"{\n"
"	ShaderFunction3_Output output = (ShaderFunction3_Output) 0;\n"
"	#define c0 uniforms_float4[0]\n"
"	#define c1 uniforms_float4[1]\n"
"	#define c2 uniforms_float4[2]\n"
"	#define c3 uniforms_float4[3]\n"
"	#define v0 input.m_v0\n"
"	#define v1 input.m_v1\n"
"	#define v2 input.m_v2\n"
"	#define oPos output.m_oPos\n"
"	#define oD0 output.m_oD0\n"
"	#define oT0 output.m_oT0\n"
"	oPos.x = dot(v2, c0);\n"
"	oPos.y = dot(v2, c1);\n"
"	oPos.z = dot(v2, c2);\n"
"	oPos.w = dot(v2, c3);\n"
"	oD0 = v0;\n"
"	oT0.xy = v1.xy;\n"
"	#undef c0\n"
"	#undef c1\n"
"	#undef c2\n"
"	#undef c3\n"
"	#undef v0\n"
"	#undef v1\n"
"	#undef v2\n"
"	#undef oPos\n"
"	#undef oD0\n"
"	#undef oT0\n"
"	return output;\n"
"}\n\n";

const BYTE fakevertex[] =
{
     68,  88,  66,  67, 149, 149,
    106, 146, 237, 224, 100,  10,
    188,   4,  15, 200, 129,  40,
     38,  70,   1,   0,   0,   0,
    176,   3,   0,   0,   5,   0,
      0,   0,  52,   0,   0,   0,
     24,   1,   0,   0, 136,   1,
      0,   0, 252,   1,   0,   0,
     52,   3,   0,   0,  82,  68,
     69,  70, 220,   0,   0,   0,
      1,   0,   0,   0,  88,   0,
      0,   0,   1,   0,   0,   0,
     28,   0,   0,   0,   0,   4,
    254, 255,   0,   1,   0,   0,
    168,   0,   0,   0,  60,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   1,   0,
      0,   0,   1,   0,   0,   0,
     83, 104,  97, 100, 101, 114,
     70, 117, 110,  99, 116, 105,
    111, 110,  51,  95,  85, 110,
    105, 102, 111, 114, 109, 115,
      0, 171, 171, 171,  60,   0,
      0,   0,   1,   0,   0,   0,
    112,   0,   0,   0,  64,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0, 136,   0,
      0,   0,   0,   0,   0,   0,
     64,   0,   0,   0,   2,   0,
      0,   0, 152,   0,   0,   0,
      0,   0,   0,   0, 117, 110,
    105, 102, 111, 114, 109, 115,
     95, 102, 108, 111,  97, 116,
     52,   0,   1,   0,   3,   0,
      1,   0,   4,   0,   4,   0,
      0,   0,   0,   0,   0,   0,
     77, 105,  99, 114, 111, 115,
    111, 102, 116,  32,  40,  82,
     41,  32,  72,  76,  83,  76,
     32,  83, 104,  97, 100, 101,
    114,  32,  67, 111, 109, 112,
    105, 108, 101, 114,  32,  57,
     46,  50,  57,  46,  57,  53,
     50,  46,  51,  49,  49,  49,
      0, 171, 171, 171,  73,  83,
     71,  78, 104,   0,   0,   0,
      3,   0,   0,   0,   8,   0,
      0,   0,  80,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   3,   0,   0,   0,
      0,   0,   0,   0,  15,  15,
      0,   0,  86,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   3,   0,   0,   0,
      1,   0,   0,   0,  15,   3,
      0,   0,  95,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   3,   0,   0,   0,
      2,   0,   0,   0,  15,  15,
      0,   0,  67,  79,  76,  79,
     82,   0,  84,  69,  88,  67,
     79,  79,  82,  68,   0,  80,
     79,  83,  73,  84,  73,  79,
     78,   0,  79,  83,  71,  78,
    108,   0,   0,   0,   3,   0,
      0,   0,   8,   0,   0,   0,
     80,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      3,   0,   0,   0,   0,   0,
      0,   0,  15,   0,   0,   0,
     86,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      3,   0,   0,   0,   1,   0,
      0,   0,  15,   0,   0,   0,
     95,   0,   0,   0,   0,   0,
      0,   0,   1,   0,   0,   0,
      3,   0,   0,   0,   2,   0,
      0,   0,  15,   0,   0,   0,
     67,  79,  76,  79,  82,   0,
     84,  69,  88,  67,  79,  79,
     82,  68,   0,  83,  86,  95,
     80, 111, 115, 105, 116, 105,
    111, 110,   0, 171,  83,  72,
     68,  82,  48,   1,   0,   0,
     64,   0,   1,   0,  76,   0,
      0,   0,  89,   0,   0,   4,
     70, 142,  32,   0,   0,   0,
      0,   0,   4,   0,   0,   0,
     95,   0,   0,   3, 242,  16,
     16,   0,   0,   0,   0,   0,
     95,   0,   0,   3,  50,  16,
     16,   0,   1,   0,   0,   0,
     95,   0,   0,   3, 242,  16,
     16,   0,   2,   0,   0,   0,
    101,   0,   0,   3, 242,  32,
     16,   0,   0,   0,   0,   0,
    101,   0,   0,   3, 242,  32,
     16,   0,   1,   0,   0,   0,
    103,   0,   0,   4, 242,  32,
     16,   0,   2,   0,   0,   0,
      1,   0,   0,   0,  54,   0,
      0,   5, 242,  32,  16,   0,
      0,   0,   0,   0,  70,  30,
     16,   0,   0,   0,   0,   0,
     54,   0,   0,   5,  50,  32,
     16,   0,   1,   0,   0,   0,
     70,  16,  16,   0,   1,   0,
      0,   0,  54,   0,   0,   8,
    194,  32,  16,   0,   1,   0,
      0,   0,   2,  64,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,  17,   0,
      0,   8,  18,  32,  16,   0,
      2,   0,   0,   0,  70,  30,
     16,   0,   2,   0,   0,   0,
     70, 142,  32,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
     17,   0,   0,   8,  34,  32,
     16,   0,   2,   0,   0,   0,
     70,  30,  16,   0,   2,   0,
      0,   0,  70, 142,  32,   0,
      0,   0,   0,   0,   1,   0,
      0,   0,  17,   0,   0,   8,
     66,  32,  16,   0,   2,   0,
      0,   0,  70,  30,  16,   0,
      2,   0,   0,   0,  70, 142,
     32,   0,   0,   0,   0,   0,
      2,   0,   0,   0,  17,   0,
      0,   8, 130,  32,  16,   0,
      2,   0,   0,   0,  70,  30,
     16,   0,   2,   0,   0,   0,
     70, 142,  32,   0,   0,   0,
      0,   0,   3,   0,   0,   0,
     62,   0,   0,   1,  83,  84,
     65,  84, 116,   0,   0,   0,
      8,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      6,   0,   0,   0,   4,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   1,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   3,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0
};

const char *fakepixelsrc =
"Texture2D s0_texture : register(t0);\n"
"SamplerState s0 : register(s0);\n"
"\n"
"struct ShaderFunction4_Input\n"
"{\n"
"	float4 m_oD0 : COLOR0;\n"
"	float4 m_oT0 : TEXCOORD0;\n"
"	float4 m_oPos : SV_Position;\n"
"};\n"
"\n"
"struct ShaderFunction4_Output\n"
"{\n"
"	float4 m_oC0 : SV_Target0;\n"
"};\n"
"\n"
"ShaderFunction4_Output ShaderFunction4(ShaderFunction4_Input input)\n"
"{\n"
"	ShaderFunction4_Output output = (ShaderFunction4_Output) 0;\n"
"	float4 r0;\n"
"	#define oD0 input.m_oD0\n"
"	#define oT0 input.m_oT0\n"
"	#define oC0 output.m_oC0\n"
"	r0 = s0_texture.Sample(s0, oT0.xy);\n"
"	r0 = r0 * oD0;\n"
"	oC0 = r0;\n"
"	#undef oD0\n"
"	#undef oT0\n"
"	#undef oC0\n"
"	return output;\n"
"}\n\n";

const BYTE fakepixel[] =
{
     68,  88,  66,  67, 207, 213,
    220,  27,  74, 162, 116,   6,
     13, 183,  50, 147, 249, 241,
    222, 192,   1,   0,   0,   0,
    152,   2,   0,   0,   5,   0,
      0,   0,  52,   0,   0,   0,
    216,   0,   0,   0,  76,   1,
      0,   0, 128,   1,   0,   0,
     28,   2,   0,   0,  82,  68,
     69,  70, 156,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   2,   0,   0,   0,
     28,   0,   0,   0,   0,   4,
    255, 255,   0,   1,   0,   0,
    106,   0,   0,   0,  92,   0,
      0,   0,   3,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   1,   0,
      0,   0,   1,   0,   0,   0,
     95,   0,   0,   0,   2,   0,
      0,   0,   5,   0,   0,   0,
      4,   0,   0,   0, 255, 255,
    255, 255,   0,   0,   0,   0,
      1,   0,   0,   0,  13,   0,
      0,   0, 115,  48,   0, 115,
     48,  95, 116, 101, 120, 116,
    117, 114, 101,   0,  77, 105,
     99, 114, 111, 115, 111, 102,
    116,  32,  40,  82,  41,  32,
     72,  76,  83,  76,  32,  83,
    104,  97, 100, 101, 114,  32,
     67, 111, 109, 112, 105, 108,
    101, 114,  32,  57,  46,  50,
     57,  46,  57,  53,  50,  46,
     51,  49,  49,  49,   0, 171,
     73,  83,  71,  78, 108,   0,
      0,   0,   3,   0,   0,   0,
      8,   0,   0,   0,  80,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   3,   0,
      0,   0,   0,   0,   0,   0,
     15,  15,   0,   0,  86,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   3,   0,
      0,   0,   1,   0,   0,   0,
     15,   3,   0,   0,  95,   0,
      0,   0,   0,   0,   0,   0,
      1,   0,   0,   0,   3,   0,
      0,   0,   2,   0,   0,   0,
     15,   0,   0,   0,  67,  79,
     76,  79,  82,   0,  84,  69,
     88,  67,  79,  79,  82,  68,
      0,  83,  86,  95,  80, 111,
    115, 105, 116, 105, 111, 110,
      0, 171,  79,  83,  71,  78,
     44,   0,   0,   0,   1,   0,
      0,   0,   8,   0,   0,   0,
     32,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      3,   0,   0,   0,   0,   0,
      0,   0,  15,   0,   0,   0,
     83,  86,  95,  84,  97, 114,
    103, 101, 116,   0, 171, 171,
     83,  72,  68,  82, 148,   0,
      0,   0,  64,   0,   0,   0,
     37,   0,   0,   0,  90,   0,
      0,   3,   0,  96,  16,   0,
      0,   0,   0,   0,  88,  24,
      0,   4,   0, 112,  16,   0,
      0,   0,   0,   0,  85,  85,
      0,   0,  98,  16,   0,   3,
    242,  16,  16,   0,   0,   0,
      0,   0,  98,  16,   0,   3,
     50,  16,  16,   0,   1,   0,
      0,   0, 101,   0,   0,   3,
    242,  32,  16,   0,   0,   0,
      0,   0, 104,   0,   0,   2,
      1,   0,   0,   0,  69,   0,
      0,   9, 242,   0,  16,   0,
      0,   0,   0,   0,  70,  16,
     16,   0,   1,   0,   0,   0,
     70, 126,  16,   0,   0,   0,
      0,   0,   0,  96,  16,   0,
      0,   0,   0,   0,  56,   0,
      0,   7, 242,  32,  16,   0,
      0,   0,   0,   0,  70,  14,
     16,   0,   0,   0,   0,   0,
     70,  30,  16,   0,   0,   0,
      0,   0,  62,   0,   0,   1,
     83,  84,  65,  84, 116,   0,
      0,   0,   3,   0,   0,   0,
      1,   0,   0,   0,   0,   0,
      0,   0,   3,   0,   0,   0,
      1,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      1,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   1,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,
      0,   0,   0,   0
};

HRESULT WINAPI D3DCompile(
    LPCVOID pSrcData,
    SIZE_T SrcDataSize,
    LPCSTR pSourceName,
    const D3D_SHADER_MACRO *pDefines,
    ID3DInclude *pInclude,
    LPCSTR pEntrypoint,
    LPCSTR pTarget,
    UINT Flags1,
    UINT Flags2,
    ID3DBlob **ppCode,
    ID3DBlob **ppErrorMsgs
) {
    if (strcmp((const char*) pSrcData, fakevertexsrc) == 0)
    {
        D3DCreateBlob(sizeof(fakevertex), ppCode);
        memcpy(ID3D10Blob_GetBufferPointer(*ppCode), fakevertex, sizeof(fakevertex));
        return 0;
    }
    if (strcmp((const char*) pSrcData, fakepixelsrc) == 0)
    {
        D3DCreateBlob(sizeof(fakepixel), ppCode);
        memcpy(ID3D10Blob_GetBufferPointer(*ppCode), fakepixel, sizeof(fakepixel));
        return 0;
    }
    return -1;
}

#else

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

#endif
