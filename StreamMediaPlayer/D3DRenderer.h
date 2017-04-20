//D3DRenderer.h:

#pragma once

#include "Databuffer.h"

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#include <atlbase.h> //使用com接口类需使用智能指针

class D3DRenderer
{
public:
	D3DRenderer();
	~D3DRenderer();

public:
	bool SetRendererWnd(HWND hWnd);
	HWND GetRendererWnd() const { return m_hWnd; }
	int RenderSample(FrameYUVBuffer* pFrameYUV);
	void TransRect(const RECT& rcSrc, RECT& rcDst);
	int DrawRect(const RECT& rc);

	int RedrawFrame();

protected:
	bool InitD3D();
	bool CreateDevice();
	bool ResetDevice();
	int RenderFrame(FrameYUVBuffer* pFrameYUV);
	int RenderDataToOffscreenSurface(FrameYUVBuffer* pFrameYUV);
	void ReleaseD3D();

private:
	HWND m_hWnd;
	RECT m_rcWnd;
	RECT m_rcDraw;
	FrameYUVBuffer* m_pFrameYUV;

	CComPtr<IDirect3D9> m_pDD;
	CComPtr<IDirect3DDevice9> m_pDev;
	CComPtr<IDirect3DSurface9> m_pOffscreenSurf;
};