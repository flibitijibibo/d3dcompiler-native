# Makefile for d3dcompiler-native
# Written by Ethan "flibitijibibo" Lee

CFLAGS ?= -O3

DXVK_NATIVE_INC = \
	-I../dxvk-native/include/native/windows \
	-I../dxvk-native/include/native/directx \
	-I../dxvk-native/include/native/wsi

VKD3D_INC = `pkg-config --cflags libvkd3d-shader`
VKD3D_LIB = `pkg-config --libs libvkd3d-shader`

all:
	cc -fpic -fPIC -shared -o libd3dcompiler.so $(CFLAGS) $(DXVK_NATIVE_INC) $(VKD3D_INC) d3dcompiler.c $(VKD3D_LIB)

clean:
	rm -f libd3dcompiler.so
