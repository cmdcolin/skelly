#pragma once

#include "Main.h"

HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, 
                 ID3DXMesh** ppMesh, D3DMATERIAL9 ** materials, LPDIRECT3DTEXTURE9 * textures,
                 LPDWORD subsets );

HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, 
                 ID3DXMesh** ppMesh );

HRESULT CreateMappedSphere( IDirect3DDevice9* pd3dDevice, 
                           float fRad, UINT slices, UINT stacks, 
                           ID3DXMesh ** ppMesh );
