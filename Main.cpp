#include "DXUT.h"
#include "Main.h"
#include "Mesh.h"


float g_d;
class timer
{

public:

    timer()
    {
        ::QueryPerformanceFrequency(&freq);
        ::QueryPerformanceCounter(&now);
        ::QueryPerformanceCounter(&start);
    }

    void tick()
    {
        LARGE_INTEGER prev = now;
        ::QueryPerformanceCounter(&now);
    }

	void reset()
	{
        ::QueryPerformanceFrequency(&freq);
        ::QueryPerformanceCounter(&now);
        ::QueryPerformanceCounter(&start);
	}

    double elapsed()
    {
		tick();
        return double(now.QuadPart - start.QuadPart) / double(freq.QuadPart);
    }

private:

    LARGE_INTEGER now, start, freq;

} t; 

ID3DXMesh*                  g_pSkull = NULL;
ID3DXMesh*                  g_pMesh = NULL;
ID3DXSprite*				g_pTextSprite = NULL;

IDirect3DTexture9*          g_pMeshTexture = NULL;
ID3DXFont*                  g_pFont = NULL;
ID3DXEffect*                g_pEffect = NULL;
ID3DXEffect*                g_pPostEffect = NULL;

IDirect3DVertexDeclaration9*    g_pVertDeclPP = NULL; // Vertex decl for post-processing

// Off-screen render target
IDirect3DTexture9* g_pRenderTargetTexture;
IDirect3DTexture9* g_pCopyTexture;
IDirect3DTexture9* g_pCopyTextureDefault;
IDirect3DSurface9* g_pRenderTargetSurface;
IDirect3DSurface9* g_pFrameBufferSurface;

IDirect3DDevice9 * g_pd3dDevice;

D3DXMATRIXA16               g_mWorld;
D3DXMATRIXA16               g_mProj;
D3DXMATRIXA16               g_mView;
D3DXMATRIXA16               g_mCenterWorld;


D3DXMATRIXA16               zw_rotation;
D3DXMATRIXA16				yw_rotation;
D3DXMATRIXA16				xw_rotation;
D3DXMATRIXA16				xy_rotation;
D3DXMATRIXA16				yz_rotation;
D3DXMATRIXA16				xz_rotation;

bool zw, yw, xw, xy, yz, xz;
double zw_time, yw_time, xw_time;
double g_fTime = 0;

bool help = false;
bool refresh = true;
bool shader = true;

WCHAR szFilename[256];


CModelViewerCamera          g_Camera; 
D3DXVECTOR3 g_vPos;
D3DXVECTOR3 g_vLook;
D3DXVECTOR3 g_vUp;


D3DXVECTOR3 g_lightDir;
D3DXCOLOR g_lightColor;

D3DMATERIAL9 * g_materials;
LPDIRECT3DTEXTURE9 g_textures;
DWORD g_subsets;

const int DEFAULT_SCREEN_WIDTH = 800;
const int DEFAULT_SCREEN_HEIGHT = 600;

ID3DXMesh * renderArray[2];
DWORD rendering = 0;



//--------------------------------------------------------------------------------------
// This is the vertex format used with the quad during post-process.
struct PPVERT
{
    float x, y, z, rhw;
    float tu, tv;       // Texcoord for post-process source
    float tu2, tv2;     // Texcoord for the original scene

    const static D3DVERTEXELEMENT9 Decl[4];
};

// Vertex declaration for post-processing
const D3DVERTEXELEMENT9 PPVERT::Decl[4] =
{
    { 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
    { 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0 },
    { 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  1 },
    D3DDECL_END()
};


void RenderText()
{
	CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

	txtHelper.Begin();
	txtHelper.SetInsertionPos( 5, 5 );
	txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );

	//txtHelper.DrawTextLine( DXUTGetFrameStats() );
	//txtHelper.DrawTextLine( DXUTGetDeviceStats() );




	if(help)
		txtHelper.DrawFormattedTextLine( L"FPS: %f %f", DXUTGetFPS(), g_d );

	if(help)
		txtHelper.DrawFormattedTextLine( 
		L"F1 - Toggle Help\n\
F2 - Toggle ZW rotation\n\
F3 - Toggle YW rotation\n\
F4 - Toggle XW rotation\n\
F5 - Reset ZW\n\
F6 - Reset YW\n\
F7 - Reset XW\n\
F8 - Toggle Shader\n\
F9 - Toggle Refresh\n\
F10 - Change Mesh\n\
F11 - Toggle Fullscreen\n\
F12 - Screengrab to C:\\" );


	txtHelper.End();
}


HRESULT ScreenGrab( IDirect3DDevice9* pd3dDevice, LPCTSTR pDestFile)
{
	D3DDISPLAYMODE displayMode;
	LPDIRECT3DSURFACE9 pd3dsFront = NULL;


	pd3dDevice->GetDisplayMode(0, &displayMode);

	// Create a surface

	pd3dDevice->CreateOffscreenPlainSurface(displayMode.Width, displayMode.Height,
		D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &pd3dsFront, NULL);

	// Get the front buffer and save it to a file

	if( SUCCEEDED( pd3dDevice->GetFrontBufferData(0, pd3dsFront) ) )
		D3DXSaveSurfaceToFile(pDestFile, D3DXIFF_BMP, pd3dsFront, NULL, NULL);

	// Release the surface

	if( pd3dsFront ) pd3dsFront->Release();
	return S_OK;
}

void PerformRotations(double fTime, double fElapsedTime)
{		
	HRESULT hr;

	// Get the projection & view matrix from the camera class
		D3DXMATRIXA16 mWorld = g_mCenterWorld * *g_Camera.GetWorldMatrix();
		D3DXMATRIXA16 mProj = *g_Camera.GetProjMatrix();
		D3DXMATRIXA16 mView = *g_Camera.GetViewMatrix();

		g_mWorld = mWorld;
		g_mProj = mProj;
		g_mView = mView;

		D3DXMATRIXA16 mWorldViewProjection = g_mWorld * g_mView * g_mProj;
		D3DXMATRIXA16 out;

		D3DXMatrixInverse(&out, 0, &mWorldViewProjection);
		D3DXMatrixTranspose(&out, &mWorldViewProjection);

		g_fTime += fElapsedTime;

		if(zw)
			zw_time  += fElapsedTime/4;

		zw_rotation = D3DXMATRIXA16(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, cos(zw_time), -sin(zw_time),
			0, 0, sin(zw_time), cos(zw_time));

		if(yw)
			yw_time  += fElapsedTime/4;

		yw_rotation = D3DXMATRIXA16(
			1,	0,	0,	0,
			0,	cos(yw_time),	0,	-sin(yw_time),
			0,	0,	1,	0,
			0,	sin(yw_time),	0,	cos(yw_time));

		if(xw)
			xw_time += fElapsedTime/4;

		xw_rotation = D3DXMATRIXA16(
			cos(xw_time), 0, 0, sin(xw_time),
			0, 1, 0, 0,
			0, 0, 1, 0,
			-sin(xw_time), 0, 0, cos(xw_time));
		



		V( g_pEffect->SetMatrix( "worldview", &mWorldViewProjection ) );

		V( g_pEffect->SetMatrix( "zw_rotation", &zw_rotation ) );
		V( g_pEffect->SetMatrix( "yw_rotation", &yw_rotation ) );
		V( g_pEffect->SetMatrix( "xw_rotation", &xw_rotation ) );

		V( g_pEffect->SetMatrix( "world", &g_mWorld ) );
		V( g_pEffect->SetValue( "view_eye", &g_vPos, sizeof( D3DXVECTOR3 ) ) );
		V( g_pEffect->SetMatrix( "wv_inv_trans", &out ) );
		V( g_pEffect->SetValue( "light_direction", &g_lightDir, sizeof( D3DXVECTOR3 ) ) );
		V( g_pEffect->SetValue( "ftime", &g_fTime, sizeof( float ) ) );
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9FrameRender( IDirect3DDevice9* pd3dDevice, 
								double fTime, 
								float fElapsedTime, 
								void* pUserContext )
{
	HRESULT hr;
	
	g_pd3dDevice = pd3dDevice;
	V( pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, 
		                     D3DCOLOR_XRGB(0x00, 0x0, 0x0), 1.0f, 0) );


    // Save render target 0 so we can restore it later
    IDirect3DSurface9* pOldRT;
    pd3dDevice->GetRenderTarget( 0, &pOldRT );


	if( SUCCEEDED( pd3dDevice->BeginScene() ) )
	{
		PerformRotations(fTime, fElapsedTime);

		V( g_pEffect->SetTechnique( "RenderSceneCHya" ) );


		UINT uPasses;
		g_pEffect->Begin( &uPasses, 0 );

		pd3dDevice->SetRenderTarget(0, g_pRenderTargetSurface);

		if(refresh)
		{
			V( pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, 
				D3DCOLOR_XRGB(0x00, 0x0, 0x0), 1.0f, 0) );
		}


		for( UINT uPass = 0; uPass < uPasses; ++uPass )
		{
			g_pEffect->BeginPass( uPass );
			V( g_pMesh->DrawSubset( 0 ) );
			g_pEffect->EndPass();
		}

		g_pEffect->End();
	}


	V( pd3dDevice->EndScene() );


	V( pd3dDevice->SetRenderTarget(0, g_pFrameBufferSurface) );

	// Restore the render target to the frame buffer to output to the screen
	V( pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, 
		                     D3DCOLOR_XRGB(0x00, 0x0, 0x0), 1.0f, 0) );

	// Get back buffer descriptor to set up screen, the following is a 2D rendering
    const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();


    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
		// Triangulate the full screen texture - from SDK PostProcessing sample
		// Shift -0.5 is a texel to pixel conversion
		// tu an tv values map out a (0, 0) to (1, 1) square

        PPVERT quad[4] =
        {
            { -0.5f, -0.5f, 0.5f, 1.0f, 
					0.0f, 0.0f, 0.0f, 0.0f },
            { pd3dsdBackBuffer->Width - 0.5f, -0.5f, 
					0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f },
            { -0.5f, pd3dsdBackBuffer->Height - 0.5f, 0.5f, 
					1.0f, 0.0f, 1.0f, 0.0f, 0.0f },
            { pd3dsdBackBuffer->Width - 0.5f, pd3dsdBackBuffer->Height - 0.5f, 
					0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f }
        };



		V( pd3dDevice->SetVertexDeclaration( g_pVertDeclPP ) );

		if(shader)
		{
			t.reset();
			V( g_pEffect->SetTechnique( "RenderPostProcess" ) );
			V( g_pEffect->SetTexture( "g_scrTexture", g_pRenderTargetTexture ) );
		}
		else
		{

			LPDIRECT3DSURFACE9 source = NULL;
			g_pRenderTargetTexture->GetSurfaceLevel(0, &source);

			LPDIRECT3DSURFACE9 dest = NULL;
			g_pCopyTexture->GetSurfaceLevel(0, &dest);


			LPDIRECT3DSURFACE9 dest_default = NULL;
			g_pCopyTextureDefault->GetSurfaceLevel(0, &dest_default);

			pd3dDevice->GetRenderTargetData(source, dest);


			t.reset();
			D3DLOCKED_RECT rect, rect2;
			V (dest->LockRect(&rect, NULL, 0));
			V (dest_default->LockRect(&rect2, NULL, 0));


			DWORD* dummy = (DWORD*)rect.pBits;
			DWORD* dummy2 = (DWORD*)rect2.pBits;


			for(int i = 0; i < pd3dsdBackBuffer->Width; i++)
			{
				for(int j = 0; j < pd3dsdBackBuffer->Height; j++)
				{
					//0->Blue, 1->Green, 2->Red, 3->Alpha
					int pixel = dummy[j*rect.Pitch/sizeof(DWORD) + i];

					int a = (pixel & 0xFF000000) >> 24;
					int r = (pixel & 0x00FF0000) >> 16;
					int g = (pixel & 0x0000FF00) >> 8;
					int b = (pixel & 0x000000FF);

					//a /= 3;
					//r /= 3;
					//b /= 3;
					//g /= 3;

					if (r<25 || r>220) r = 0; else r = 0xFF;
					if (g<25 || g>220) g = 0; else g = 0xFF;
					if (b<25 || b>220) b = 0; else b = 0xFF;

					int pixel2 = (a << 24) + (r << 16) + (g << 8) + b;

					dummy2[j*rect2.Pitch/sizeof(DWORD) + i] = pixel2;
				}
			}

			dest->UnlockRect();
			dest_default->UnlockRect();

			double d = t.elapsed();
			g_d = d;


			source->Release();
			dest->Release();
			dest_default->Release();



			V( g_pEffect->SetTechnique( "RenderWithout" ) );
			V( g_pEffect->SetTexture( "g_scrTexture", g_pCopyTextureDefault ) );
		}

		UINT cPasses;
		V( g_pEffect->Begin( &cPasses, 0 ) );
		for( UINT p = 0; p < cPasses; ++p )
		{
			V( g_pEffect->BeginPass( p ) );
			V( pd3dDevice->DrawPrimitiveUP
				( D3DPT_TRIANGLESTRIP, 2, quad, sizeof( PPVERT ) ) );
			V( g_pEffect->EndPass() );
		}
		V( g_pEffect->End() );


		RenderText();

        V( pd3dDevice->EndScene() );
    }

	//// Create a seperate copy of the texture
	//for(DWORD level = 0; level < g_pRenderTargetTexture->GetLevelCount(); level++)
	//{
	//	LPDIRECT3DSURFACE9 source = NULL;
	//	g_pRenderTargetTexture->GetSurfaceLevel(level, &source);

	//	LPDIRECT3DSURFACE9 dest = NULL;
	//	g_pCopyTexture->GetSurfaceLevel(level, &dest);

	//	V(pd3dDevice->StretchRect(source, NULL, dest, NULL, D3DTEXF_NONE));

	//	source->Release();
	//	dest->Release();
	//}





    // Restore old render target 0 (back buffer)
    V( pd3dDevice->SetRenderTarget( 0, pOldRT ) );
    SAFE_RELEASE( pOldRT );

	
}








//--------------------------------------------------------------------------------------
// Initialize everything and go into a render loop
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nRet )
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif


	int argc;
	LPWSTR * szArglist = ::CommandLineToArgvW(GetCommandLineW(), &argc);

	if( NULL == szArglist )
	{
		wprintf(L"CommandLineToArgvW failed\n");
		return 0;
	}


	if(argc > 1)
		lstrcpy(szFilename, szArglist[1]);
	else
		lstrcpy(szFilename, L"effect.fx");

	LocalFree(szArglist);


	DXUTSetCallbackD3D9DeviceAcceptable( IsD3D9DeviceAcceptable );
	DXUTSetCallbackD3D9DeviceCreated( OnD3D9CreateDevice );
	DXUTSetCallbackD3D9DeviceReset( OnD3D9ResetDevice );
	DXUTSetCallbackD3D9FrameRender( OnD3D9FrameRender );
	DXUTSetCallbackD3D9DeviceLost( OnD3D9LostDevice );
	DXUTSetCallbackD3D9DeviceDestroyed( OnD3D9DestroyDevice );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackKeyboard( KeyboardProc );
	DXUTSetCallbackFrameMove( OnFrameMove );


	DXUTInit( true, true ); // Parse the command line and show msgboxes
	DXUTSetHotkeyHandling( true, true, true );  // handle the default hotkeys
	DXUTSetCursorSettings( true, true ); // Show the cursor and clip it when in full screen
	DXUTCreateWindow( L"" );
	DXUTCreateDevice( true, DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT );

	DXUTMainLoop();
	return DXUTGetExitCode();
}







//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat,
									 bool bWindowed, void* pUserContext )
{
	if( pCaps->PixelShaderVersion < D3DPS_VERSION(2,0) )
		return false;

	// Typically want to skip back buffer formats that don't support alpha blending
	IDirect3D9* pD3D = DXUTGetD3D9Object();
	if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
		AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
		D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
	{
		return false;
	}

	return true;
}


//--------------------------------------------------------------------------------------
// Before a device is created, modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
#ifdef DEBUG_VS
	if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
	{
		pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
		pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
		pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
#endif
#ifdef DEBUG_PS
	pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif

	return true;
}

HRESULT LoadMedia( IDirect3DDevice9* pd3dDevice )
{

	HRESULT hr;
	V_RETURN( D3DXCreateFont( pd3dDevice, 12, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		L"Courier", &g_pFont ) );

	V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );
	V_RETURN( LoadMesh( pd3dDevice, L"misc\\skullocc.x", &g_pMesh ) );

	//WCHAR str[MAX_PATH];
	//V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"misc\\rainbow.dds" ) );
	//V_RETURN( D3DXCreateTextureFromFile( pd3dDevice, str, &g_pMeshTexture) );

	V_RETURN( D3DXCreateTextureFromResource( pd3dDevice, 0, MAKEINTRESOURCE( IDR_RAINBOW ), &g_pMeshTexture ) );


	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;

#if defined( DEBUG ) || defined( _DEBUG )
	dwShaderFlags |= D3DXSHADER_DEBUG;
#endif



	LPD3DXBUFFER pBufferErrors = NULL;

	hr = D3DXCreateEffectFromFile(
		pd3dDevice, szFilename, 0, 0, dwShaderFlags, 0, &g_pEffect, 
		                           &pBufferErrors );

	if( FAILED(hr) )
	{
		LPVOID pCompilErrors = pBufferErrors->GetBufferPointer();
		MessageBoxA(NULL, (LPCSTR)pCompilErrors, "Fx Compile Error",
			MB_OK|MB_ICONEXCLAMATION);

		SAFE_RELEASE(pBufferErrors);	
	}

	/*hr = D3DXCreateEffectFromFile(
		pd3dDevice, L"posteffect.fx", 0, 0, dwShaderFlags, 0, &g_pPostEffect, &pBufferErrors );

	if( FAILED(hr) )
	{
		LPVOID pCompilErrors = pBufferErrors->GetBufferPointer();
		MessageBoxA(NULL, (LPCSTR)pCompilErrors, "Fx Compile Error",
			MB_OK|MB_ICONEXCLAMATION);

		SAFE_RELEASE(pBufferErrors);	
	}*/


	return hr;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
									void* pUserContext )
{    
	HRESULT hr;
	V_RETURN( LoadMedia( pd3dDevice ) );

	D3DXVECTOR3* pData;
	D3DXVECTOR3 vCenter;
	FLOAT fObjectRadius;

	DWORD dwFVF = g_pMesh->GetFVF();
	UINT uVertexSize = D3DXGetFVFVertexSize(dwFVF);
	DWORD dwVertices = g_pMesh->GetNumVertices();

	V( g_pMesh->LockVertexBuffer( 0, ( LPVOID* )&pData ) );
	V( D3DXComputeBoundingSphere( pData, dwVertices, uVertexSize, &vCenter, &fObjectRadius ) );
	V( g_pMesh->UnlockVertexBuffer() );

	D3DXMatrixIdentity(&g_mCenterWorld);

	// Set effect variables as needed
	D3DXCOLOR diffuse( 0.1f, 0.2f, 0.2f, 1.0f );
	D3DXCOLOR ambient( 0.1f, 0.1f, 0.1f, 1.0f );
	D3DXCOLOR spex( 0.1f, 0.1f, 0.1f, 1.0f );


	V_RETURN( g_pEffect->SetValue( "diffuse", &ambient, sizeof( D3DXCOLOR ) ) );
	V_RETURN( g_pEffect->SetValue( "ambient", &diffuse, sizeof( D3DXCOLOR ) ) );
	V_RETURN( g_pEffect->SetValue( "specular", &spex, sizeof( D3DXCOLOR ) ) );
	V_RETURN( g_pEffect->SetTexture( "mesh_texture", g_pMeshTexture ) );


	g_lightDir = D3DXVECTOR3  ( 1.0f, 0.0f, 0.0f );
	g_lightColor = D3DXCOLOR  ( 0.0f, 1.0f, -0.0f, 0.0f );

	D3DXVECTOR3 vecEye( 0.0f, 3.0f, -6.0f );
	D3DXVECTOR3 vecAt ( 0.0f, 0.0f, -0.0f );

	g_Camera.SetViewParams( &vecEye, &vecAt );
	g_Camera.SetRadius( fObjectRadius * 3.0f, fObjectRadius * 0.5f);//, fObjectRadius * 10.0f );


	// Retrieve info from the current frame buffer
	V_RETURN( pd3dDevice->GetRenderTarget(0, &g_pFrameBufferSurface) );

	// get the description for the texture we want to filter
	D3DSURFACE_DESC ddsd;
	
	// Get screen descriptor (current frame buffer = screen)
	V_RETURN( g_pFrameBufferSurface->GetDesc(&ddsd) );

	// Create render-to-texture
	V_RETURN( pd3dDevice->CreateTexture(ddsd.Width, ddsd.Height,
		1, D3DUSAGE_RENDERTARGET, ddsd.Format, D3DPOOL_DEFAULT,
		&g_pRenderTargetTexture, 0));

	V_RETURN( pd3dDevice->CreateTexture(ddsd.Width, ddsd.Height,
		1, D3DUSAGE_DYNAMIC, ddsd.Format, D3DPOOL_SYSTEMMEM,
		&g_pCopyTexture, 0));

	
	V_RETURN( pd3dDevice->CreateTexture(ddsd.Width, ddsd.Height,
		1, D3DUSAGE_DYNAMIC, ddsd.Format, D3DPOOL_DEFAULT,
		&g_pCopyTextureDefault, 0));

	// Create render-to-texture surface
	V_RETURN( g_pRenderTargetTexture->GetSurfaceLevel(0, &g_pRenderTargetSurface) );

    // Create vertex declaration for post-process
    if( FAILED( hr = pd3dDevice->CreateVertexDeclaration( PPVERT::Decl, &g_pVertDeclPP ) ) )
    {
        return hr;
    }


	return S_OK;
}




//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9DestroyDevice( void* pUserContext )
{    
	SAFE_RELEASE( g_pTextSprite );
	SAFE_RELEASE( g_pEffect );
	SAFE_RELEASE( g_pFont );
	SAFE_RELEASE( g_pSkull );
	SAFE_RELEASE( g_pMesh );
	SAFE_RELEASE( g_pMeshTexture );
	SAFE_RELEASE( g_pRenderTargetTexture );
	SAFE_RELEASE( g_pCopyTexture );
	SAFE_RELEASE( g_pCopyTextureDefault );
	SAFE_RELEASE( g_pRenderTargetSurface );
	SAFE_RELEASE( g_pFrameBufferSurface );
    SAFE_RELEASE( g_pVertDeclPP );
}
