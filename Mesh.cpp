#include "DXUT.h"
#include "SDKmisc.h"
#include "resource.h"


//--------------------------------------------------------------------------------------
// This function loads the mesh and ensures the mesh has normals; it also optimizes the 
// mesh for the graphics card's vertex cache, which improves performance by organizing 
// the internal triangle list for less cache misses.
//--------------------------------------------------------------------------------------
HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, 
				 ID3DXMesh** ppMesh, D3DMATERIAL9 ** materials, LPDIRECT3DTEXTURE9 * textures,
				 LPDWORD subsets )
{
	ID3DXMesh* pMesh = NULL;
	WCHAR str[MAX_PATH];
	HRESULT hr;

	LPD3DXBUFFER materialBuffer;
	DWORD numMaterials;

	// Load the mesh with D3DX and get back a ID3DXMesh*.  For this
	// sample we'll ignore the X file's embedded materials since we know 
	// exactly the model we're loading.  See the mesh samples such as
	// "OptimizedMesh" for a more generic mesh loading example.
	/*
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, strFileName ) );
	V_RETURN( D3DXLoadMeshFromX( str, D3DXMESH_MANAGED, pd3dDevice, 
	NULL, &materialBuffer, NULL, &numMaterials, &pMesh ) );*/

	V_RETURN( D3DXLoadMeshFromXResource
		( 0, 
		MAKEINTRESOURCEA( IDR_SKULLTEX ), 
		MAKEINTRESOURCEA( 10 ), 
		D3DXMESH_MANAGED, 
		pd3dDevice, 
		NULL, &materialBuffer, NULL, &numMaterials, &pMesh ) );

	DWORD* rgdwAdjacency = NULL;

	// Make sure there are normals which are required for lighting
	if( !( pMesh->GetFVF() & D3DFVF_NORMAL ) )
	{
		ID3DXMesh* pTempMesh;
		V( pMesh->CloneMeshFVF( pMesh->GetOptions(),
			pMesh->GetFVF() | D3DFVF_NORMAL,
			pd3dDevice, &pTempMesh ) );
		V( D3DXComputeNormals( pTempMesh, NULL ) );

		SAFE_RELEASE( pMesh );
		pMesh = pTempMesh;
	}

	// Optimize the mesh for this graphics card's vertex cache 
	// so when rendering the mesh's triangle list the vertices will 
	// cache hit more often so it won't have to re-execute the vertex shader 
	// on those vertices so it will improve perf.     
	rgdwAdjacency = new DWORD[pMesh->GetNumFaces() * 3];
	if( rgdwAdjacency == NULL )
		return E_OUTOFMEMORY;
	V( pMesh->GenerateAdjacency( 1e-6f, rgdwAdjacency ) );
	V( pMesh->OptimizeInplace( D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL ) );
	delete []rgdwAdjacency;

	*ppMesh = pMesh;

	D3DMATERIAL9 * meshMaterials = new D3DMATERIAL9[numMaterials];
	LPDIRECT3DTEXTURE9 * meshTextures  = new LPDIRECT3DTEXTURE9[numMaterials];

	D3DXMATERIAL * d3dxMaterials = (D3DXMATERIAL *) materialBuffer->GetBufferPointer();

	for(DWORD i = 0; i < numMaterials; i++)
	{
		meshMaterials[i] = d3dxMaterials[i].MatD3D;
		meshMaterials[i].Ambient = meshMaterials[i].Diffuse;

		// Create the texture if it exists - it may not
		int buffSize = MultiByteToWideChar(
			CP_ACP, 0, 
			d3dxMaterials[i].pTextureFilename, // no need to copy the src string
			-1, // -1 means src string is nul terminated
			NULL, // not used when calc'ing size
			0);

		wchar_t * gah = new wchar_t[buffSize + 1]; // translated chars plus trailing
		MultiByteToWideChar(
			CP_ACP,
			0,
			d3dxMaterials[i].pTextureFilename,
			-1,
			gah,
			buffSize);

		gah[buffSize] = 0; // make it a C-string

		meshTextures[i] = NULL;
		if (d3dxMaterials[i].pTextureFilename)
			D3DXCreateTextureFromFile(pd3dDevice, gah, &meshTextures[i]);

	}

	*textures = *meshTextures;
	*materials = meshMaterials;

	materialBuffer->Release();

	return S_OK;
}

HRESULT LoadMesh( IDirect3DDevice9* pd3dDevice, WCHAR* strFileName, 
				 ID3DXMesh** ppMesh )
{
	ID3DXMesh* pMesh = NULL;
	WCHAR str[MAX_PATH];
	HRESULT hr;


	// Load the mesh with D3DX and get back a ID3DXMesh*.  For this
	// sample we'll ignore the X file's embedded materials since we know 
	// exactly the model we're loading.  See the mesh samples such as
	// "OptimizedMesh" for a more generic mesh loading example.

	//V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, strFileName ) );

	//::D3DXLoadMeshFromX( strFileName, D3DXMESH_MANAGED, pd3dDevice, 
	//	NULL, NULL, NULL, NULL, &pMesh );

	V_RETURN( D3DXLoadMeshFromXResource( 0, MAKEINTRESOURCEA( IDR_SKULLTEX ), MAKEINTRESOURCEA( 10), D3DXMESH_MANAGED, pd3dDevice, 
		NULL, NULL, NULL, NULL, &pMesh ) );



	DWORD* rgdwAdjacency = NULL;

	// Make sure there are normals which are required for lighting
	if( !( pMesh->GetFVF() & D3DFVF_NORMAL ) )
	{
		ID3DXMesh* pTempMesh;
		V( pMesh->CloneMeshFVF( pMesh->GetOptions(),
			pMesh->GetFVF() | D3DFVF_NORMAL,
			pd3dDevice, &pTempMesh ) );
		V( D3DXComputeNormals( pTempMesh, NULL ) );

		SAFE_RELEASE( pMesh );
		pMesh = pTempMesh;
	}

	// Optimize the mesh for this graphics card's vertex cache 
	// so when rendering the mesh's triangle list the vertices will 
	// cache hit more often so it won't have to re-execute the vertex shader 
	// on those vertices so it will improve perf.     

	rgdwAdjacency = new DWORD[pMesh->GetNumFaces() * 3];
	if( rgdwAdjacency == NULL )
		return E_OUTOFMEMORY;
	V( pMesh->GenerateAdjacency( 1e-6f, rgdwAdjacency ) );
	V( pMesh->OptimizeInplace( D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL ) );
	delete [] rgdwAdjacency;


	*ppMesh = pMesh;
	return S_OK;
}



typedef struct _VERTEX
{
	D3DXVECTOR3 pos;     // vertex position
	D3DXVECTOR3 norm;    // vertex normal
	float tu;            // texture coordinates
	float tv;
} VERTEX,*LPVERTEX;

#define FVF_VERTEX    D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1

HRESULT CreateMappedSphere( IDirect3DDevice9* pd3dDevice, 
						   float fRad, UINT slices, UINT stacks, 
						   ID3DXMesh ** ppMesh )
{
	// create the sphere
	LPD3DXMESH mesh;
	if (FAILED(D3DXCreateSphere(pd3dDevice, fRad, slices, stacks, &mesh, NULL)))
		return S_FALSE;

	// create a copy of the mesh with texture coordinates,
	// since the D3DX function doesn't include them
	LPD3DXMESH texMesh;
	if (FAILED(mesh->CloneMeshFVF(D3DXMESH_SYSTEMMEM, FVF_VERTEX, pd3dDevice, &texMesh)))
		return S_FALSE;

	// finished with the original mesh, release it
	mesh->Release();

	LPVERTEX pVerts;
	if (SUCCEEDED(texMesh->LockVertexBuffer(0,(LPVOID *)(BYTE **) &pVerts))) {

		// get vertex count
		int numVerts=texMesh->GetNumVertices();

		// loop through the vertices
		for (int i=0;i<numVerts;i++) {

			// calculate texture coordinates
			pVerts->tu=asinf(pVerts->norm.x) / D3DX_PI+0.5f;
			pVerts->tv=asinf(pVerts->norm.y) / D3DX_PI+0.5f;

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





