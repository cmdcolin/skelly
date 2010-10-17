#pragma once

#include "Main.h"
#include "Mesh.h"


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
