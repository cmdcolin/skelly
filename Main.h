#ifndef _MAIN_H_
#define _MAIN_H_

#define _CRT_NON_CONFORMING_SWPRINTFS

#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTgui.h"
#include "DXUTsettingsdlg.h"
#include "SDKmisc.h"
#include "resource.h"



HRESULT ScreenGrab( IDirect3DDevice9* pd3dDevice, LPCTSTR pDestFile);

bool CALLBACK IsD3D9DeviceAcceptable( D3DCAPS9* pCaps, 
									 D3DFORMAT AdapterFormat, 
									 D3DFORMAT BackBufferFormat,
									 bool bWindowed, 
									 void* pUserContext );

bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, 
								   void* pUserContext );

HRESULT CALLBACK OnD3D9CreateDevice( IDirect3DDevice9* pd3dDevice, 
									const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
									void* pUserContext );

HRESULT CALLBACK OnD3D9ResetDevice( IDirect3DDevice9* pd3dDevice, 
								   const D3DSURFACE_DESC* 
								   pBackBufferSurfaceDesc,
								   void* pUserContext );

void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnD3D9LostDevice( void* pUserContext );
void CALLBACK OnD3D9DestroyDevice( void* pUserContext );

LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
						 bool* pbNoFurtherProcessing, void* pUserContext );

void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );




extern ID3DXMesh*                  g_pSkull;
extern ID3DXMesh*                  g_pSphere;
extern ID3DXMesh*                  g_pMesh;
extern ID3DXSprite*				g_pTextSprite;

extern IDirect3DTexture9*          g_pMeshTexture;
extern ID3DXFont*                  g_pFont;
extern ID3DXEffect*                g_pEffect;

extern D3DXMATRIXA16               g_mWorld;
extern D3DXMATRIXA16               g_mProj;
extern D3DXMATRIXA16               g_mView;
extern D3DXMATRIXA16               g_mCenterWorld;
extern D3DXVECTOR3 g_vPos;
extern D3DXVECTOR3 g_vLook;
extern D3DXVECTOR3 g_vUp;

extern CModelViewerCamera          g_Camera; 

extern D3DXVECTOR3 g_lightDir;
extern D3DXCOLOR g_lightColor;

extern D3DMATERIAL9 * g_materials;
extern LPDIRECT3DTEXTURE9 g_textures;
extern DWORD g_subsets;

extern IDirect3DTexture9* g_pRenderTargetTexture;
extern IDirect3DSurface9* g_pRenderTargetSurface;
extern IDirect3DSurface9* g_pFrameBufferSurface;

extern IDirect3DDevice9 * g_pd3dDevice;

extern const int DEFAULT_SCREEN_WIDTH;
extern const int DEFAULT_SCREEN_HEIGHT;



extern D3DXMATRIXA16               zw_rotation;
extern D3DXMATRIXA16				yw_rotation;
extern D3DXMATRIXA16				xw_rotation;
extern D3DXMATRIXA16				xy_rotation;
extern D3DXMATRIXA16				yz_rotation;
extern D3DXMATRIXA16				xz_rotation;


extern double zw_time, yw_time, xw_time;
extern bool zw, yw, xw;

extern ID3DXMesh * renderArray[2];
extern DWORD rendering;
extern IDirect3DDevice9* g_pd3dDevice;


extern bool help;
extern bool refresh;
extern bool shader;

#endif