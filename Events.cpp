#include "DXUT.h"
#include "Events.h"


void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
	float x = 0, y = 0, z = 0;

	if( bKeyDown )
	{
		switch( nChar )
		{
		case VK_F1:
			help = !help;
			break;

		case VK_F2:
			zw = !zw;
			break;
		case VK_F3:
			yw = !yw;
			break;
		case VK_F4:
			xw = !xw;
			break;
		case VK_F5:
			::D3DXMatrixIdentity(&zw_rotation);
			zw_time = 0;
			zw = false;
			break;

		case VK_F6:
			::D3DXMatrixIdentity(&yw_rotation);
			yw_time = 0;
			yw = false;
			break;
		case VK_F7:
			::D3DXMatrixIdentity(&xw_rotation);
			xw_time = 0;
			xw = false;
			break;

		case VK_F8:
			shader = !shader;
			break;


		case VK_F9:
			refresh = !refresh;
			break;

		case VK_F10:
			rendering++;
			rendering %= 2;
			break;
		case VK_F11:
			DXUTToggleFullScreen(); 
			break;
		case VK_F12:
			{
			DWORD t = ::GetTickCount();
			wchar_t buf[256];
			wsprintf(buf, L"c:\\%d.bmp", t);

			ScreenGrab(g_pd3dDevice, buf);
			break;
			}


		case VK_DOWN: y = -0.1f; break;
		case VK_UP: y = 0.1f; break;
		case VK_RIGHT: x = 0.1f; break;
		case VK_LEFT: x = -0.1f; break;
		case VK_CONTROL: z = 0.1f; break;
		case VK_SHIFT: z = -0.1f; break;


		}
	}

	D3DXVECTOR3 eye = *g_Camera.GetEyePt();
	D3DXVECTOR3 look = *g_Camera.GetLookAtPt();

	eye.x += x;
	eye.y += y;
	eye.z += z;

	g_Camera.SetViewParams(&eye, &look);

	///
	//g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );
	///
}



//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
								   void* pUserContext )
{
	HRESULT hr;

	if( g_pFont )
		V_RETURN( g_pFont->OnResetDevice() );
	if( g_pEffect )
		V_RETURN( g_pEffect->OnResetDevice() );

	//V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

	float aspect = 
		float(pBackBufferSurfaceDesc->Width) / 
		float(pBackBufferSurfaceDesc->Height);

	D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI/4,
		aspect, 2.0f, 2000.0f );

	pd3dDevice->SetTransform(D3DTS_PROJECTION, &g_mProj);

	float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 2.0f, 4000.0f );
	g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
	g_Camera.SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );

	return S_OK;
}





//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{    
	g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D9LostDevice( void* pUserContext )
{    
	if( g_pFont )
		g_pFont->OnLostDevice();
	if( g_pEffect )
		g_pEffect->OnLostDevice();

	SAFE_RELEASE( g_pTextSprite );
}




//--------------------------------------------------------------------------------------
// Handle messages to the application 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
						 bool* pbNoFurtherProcessing, void* pUserContext )
{
	g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

	return 0;
}
