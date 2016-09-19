#include "DXUT.h"
#include "SDKmisc.h"
#include "resource.h"



HRESULT LoadMesh(IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh, D3DMATERIAL9 ** materials, LPDIRECT3DTEXTURE9 * textures, LPDWORD subsets) {
    ID3DXMesh* pMesh = NULL;
    WCHAR str[MAX_PATH];
    HRESULT hr;

    LPD3DXBUFFER materialBuffer;
    DWORD numMaterials;

    V_RETURN(D3DXLoadMeshFromXResource(0, MAKEINTRESOURCEA(IDR_SKULLTEX), MAKEINTRESOURCEA(10),  D3DXMESH_MANAGED, pd3dDevice, NULL, &materialBuffer, NULL, &numMaterials, &pMesh));

    DWORD* rgdwAdjacency = NULL;

    // Make sure there are normals which are required for lighting
    if (!(pMesh->GetFVF() & D3DFVF_NORMAL)) {
        ID3DXMesh* pTempMesh;
        V(pMesh->CloneMeshFVF(pMesh->GetOptions(), pMesh->GetFVF() | D3DFVF_NORMAL, pd3dDevice, &pTempMesh));
        V(D3DXComputeNormals(pTempMesh, NULL));

        SAFE_RELEASE(pMesh);
        pMesh = pTempMesh;
    }
    rgdwAdjacency = new DWORD[pMesh->GetNumFaces() * 3];
    if (rgdwAdjacency == NULL) {
        return E_OUTOFMEMORY;
    }
    V(pMesh->GenerateAdjacency(1e-6f, rgdwAdjacency));
    V(pMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL));
    delete []rgdwAdjacency;

    *ppMesh = pMesh;

    D3DMATERIAL9 * meshMaterials = new D3DMATERIAL9[numMaterials];
    LPDIRECT3DTEXTURE9 * meshTextures = new LPDIRECT3DTEXTURE9[numMaterials];

    D3DXMATERIAL * d3dxMaterials = (D3DXMATERIAL *) materialBuffer->GetBufferPointer();

    for(DWORD i = 0; i < numMaterials; i++) {
        meshMaterials[i] = d3dxMaterials[i].MatD3D;
        meshMaterials[i].Ambient = meshMaterials[i].Diffuse;

        // Create the texture if it exists - it may not
        int buffSize = MultiByteToWideChar(CP_ACP, 0, d3dxMaterials[i].pTextureFilename, -1, NULL, 0);
        wchar_t * meshName = new wchar_t[buffSize + 1]; // translated chars plus trailing
        MultiByteToWideChar(CP_ACP, 0, d3dxMaterials[i].pTextureFilename, -1, meshName, buffSize);
        meshName[buffSize] = 0; // make it a C-string

        meshTextures[i] = NULL;
        if (d3dxMaterials[i].pTextureFilename) {
            D3DXCreateTextureFromFile(pd3dDevice, meshName, &meshTextures[i]);
        }
    }

    *textures = *meshTextures;
    *materials = meshMaterials;

    materialBuffer->Release();

    return S_OK;
}

HRESULT LoadMesh(IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, ID3DXMesh** ppMesh) {
    ID3DXMesh* pMesh = NULL;
    WCHAR str[MAX_PATH];
    HRESULT hr;


    V_RETURN(D3DXLoadMeshFromXResource(0, MAKEINTRESOURCEA(IDR_SKULLTEX), MAKEINTRESOURCEA(10), D3DXMESH_MANAGED, pd3dDevice,  NULL, NULL, NULL, NULL, &pMesh));



    DWORD* rgdwAdjacency = NULL;

    // Make sure there are normals which are required for lighting
    if (!(pMesh->GetFVF() & D3DFVF_NORMAL)) {
        ID3DXMesh* pTempMesh;
        V(pMesh->CloneMeshFVF(pMesh->GetOptions(), pMesh->GetFVF() | D3DFVF_NORMAL, pd3dDevice, &pTempMesh));
        V(D3DXComputeNormals(pTempMesh, NULL));

        SAFE_RELEASE(pMesh);
        pMesh = pTempMesh;
    }

    rgdwAdjacency = new DWORD[pMesh->GetNumFaces() * 3];
    if (rgdwAdjacency == NULL) {
        return E_OUTOFMEMORY;
    }
    V(pMesh->GenerateAdjacency(1e-6f, rgdwAdjacency));
    V(pMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL));
    delete []rgdwAdjacency;


    *ppMesh = pMesh;
    return S_OK;
}



typedef struct _VERTEX
{
    D3DXVECTOR3 pos;
    D3DXVECTOR3 norm;
    float tu;
    float tv;
} VERTEX,*LPVERTEX;


HRESULT CreateMappedSphere(IDirect3DDevice9* pd3dDevice, float fRad, UINT slices, UINT stacks, ID3DXMesh** ppMesh ) {
    // create the sphere
    LPD3DXMESH mesh;
    if (FAILED(D3DXCreateSphere(pd3dDevice, fRad, slices, stacks, &mesh, NULL))) {
        return S_FALSE;
    }

    // create a copy of the mesh with texture coordinates,
    // since the D3DX function doesn't include them
    LPD3DXMESH texMesh;
    if (FAILED(mesh->CloneMeshFVF(D3DXMESH_SYSTEMMEM, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1, pd3dDevice, &texMesh))) {
        return S_FALSE;
    }

    // finished with the original mesh, release it
    mesh->Release();

    LPVERTEX pVerts;
    if (SUCCEEDED(texMesh->LockVertexBuffer(0,(LPVOID *)(BYTE **) &pVerts))) {

        // get vertex count
        int numVerts = texMesh->GetNumVertices();

        // loop through the vertices
        for (int i = 0; i < numVerts; i++) {

            // calculate texture coordinates
            pVerts->tu = asinf(pVerts->norm.x) / D3DX_PI + 0.5f;
            pVerts->tv = asinf(pVerts->norm.y) / D3DX_PI + 0.5f;

            // go to next vertex
            pVerts++;
        }

        // unlock the vertex buffer
        texMesh->UnlockVertexBuffer();
    }

    *ppMesh = texMesh;
    // return pointer to caller
    return S_OK;
}
