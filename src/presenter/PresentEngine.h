//ofxWMFVideoPlayer addon written by Philippe Laulheret for Second Story (secondstory.com)
//Based upon Windows SDK samples
//MIT Licensing


//////////////////////////////////////////////////////////////////////////
//
// PresentEngine.h: Defines the D3DPresentEngine object.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

//#include "glu.h"
#include "GL/glew.h"
#include "GL/wglew.h"
//#include "GLFW/glfw3.h"

//-----------------------------------------------------------------------------
// D3DPresentEngine class
//
// This class creates the Direct3D device, allocates Direct3D surfaces for
// rendering, and presents the surfaces. This class also owns the Direct3D
// device manager and provides the IDirect3DDeviceManager9 interface via
// GetService.
//
// The goal of this class is to isolate the EVRCustomPresenter class from
// the details of Direct3D as much as possible.
//-----------------------------------------------------------------------------

const DWORD PRESENTER_BUFFER_COUNT = 3;


#pragma comment (lib,"Evr.lib")
#pragma comment(lib,"D3d9.lib")
#pragma comment(lib,"Dxva2.lib")

class D3DPresentEngine : public SchedulerCallback
{
public:

    // State of the Direct3D device.
    enum DeviceState
    {
        DeviceOK,
        DeviceReset,    // The device was reset OR re-created.
        DeviceRemoved,  // The device was removed.
    };

    D3DPresentEngine(HRESULT& hr);
    virtual ~D3DPresentEngine();

    // GetService: Returns the IDirect3DDeviceManager9 interface.
    // (The signature is identical to IMFGetService::GetService but 
    // this object does not derive from IUnknown.)
    virtual HRESULT GetService(REFGUID guidService, REFIID riid, void** ppv);
    virtual HRESULT CheckFormat(D3DFORMAT format);

    // Video window / destination rectangle:
    // This object implements a sub-set of the functions defined by the 
    // IMFVideoDisplayControl interface. However, some of the method signatures 
    // are different. The presenter's implementation of IMFVideoDisplayControl 
    // calls these methods.
    HRESULT SetVideoWindow(HWND hwnd);
    HWND    GetVideoWindow() const { return m_hwnd; }
    HRESULT SetDestinationRect(const RECT& rcDest);
    RECT    GetDestinationRect() const { return m_rcDestRect; };

    HRESULT CreateVideoSamples(IMFMediaType *pFormat, VideoSampleList& videoSampleQueue);
    void    ReleaseResources();

    HRESULT CheckDeviceState(DeviceState *pState);
    HRESULT PresentSample(IMFSample* pSample, LONGLONG llTarget); 

    UINT    RefreshRate() const { return m_DisplayMode.RefreshRate; }

protected:
    HRESULT InitializeD3D();
    HRESULT GetSwapChainPresentParameters(IMFMediaType *pType, D3DPRESENT_PARAMETERS* pPP);
    HRESULT CreateD3DDevice();
    HRESULT CreateD3DSample(IDirect3DSwapChain9 *pSwapChain, IMFSample **ppVideoSample);
    HRESULT UpdateDestRect();

    // A derived class can override these handlers to allocate any additional D3D resources.
    virtual HRESULT OnCreateVideoSamples(D3DPRESENT_PARAMETERS& pp) { return S_OK; }
   // virtual void    OnReleaseResources() ;

    virtual HRESULT PresentSwapChain(IDirect3DSwapChain9* pSwapChain, IDirect3DSurface9* pSurface);
    virtual void    PaintFrameWithGDI();

protected:
    UINT                        m_DeviceResetToken;     // Reset token for the D3D device manager.

    HWND                        m_hwnd;                 // Application-provided destination window.
    RECT                        m_rcDestRect;           // Destination rectangle.
    D3DDISPLAYMODE              m_DisplayMode;          // Adapter's display mode.

    CritSec                     m_ObjectLock;           // Thread lock for the D3D device.

    // COM interfaces
    IDirect3D9Ex                *m_pD3D9;
    IDirect3DDevice9Ex          *m_pDevice;
    IDirect3DDeviceManager9     *m_pDeviceManager;        // Direct3D device manager.
    IDirect3DSurface9           *m_pSurfaceRepaint;       // Surface for repaint requests.

protected:
	HANDLE gl_handleD3D;
	HANDLE d3d_shared_handle;
	
	GLuint gl_name;
	HANDLE gl_handle;

	DWORD _shared_handle_val;
	IDirect3DSurface9 *d3d_shared_surface;
	IDirect3DTexture9 *d3d_shared_texture;

	int _w,_h;

public:

	HANDLE getSharedDeviceHandle() { return gl_handleD3D;}

	virtual void OnReleaseResources()
	{
		

	}
	bool createSharedTexture(int w, int h, int textureID);

	void releaseSharedTexture();
	bool lockSharedTexture();

	bool unlockSharedTexture();
};