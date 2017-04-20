//D3DRenderer.cpp:

#include "stdafx.h"
#include "D3DRenderer.h"

D3DRenderer::D3DRenderer()
: m_hWnd(NULL)
, m_pFrameYUV(NULL)
, m_pDD(NULL)
, m_pDev(NULL)
, m_pOffscreenSurf(NULL)
{
	InitD3D();
	m_pFrameYUV = new FrameYUVBuffer;
	memset(&m_rcWnd, 0, sizeof(RECT));
	memset(&m_rcDraw, 0, sizeof(RECT));
}

D3DRenderer::~D3DRenderer()
{
	if (m_pFrameYUV)
	{
		delete m_pFrameYUV;
		m_pFrameYUV = NULL;
	}
	ReleaseD3D();
}

bool D3DRenderer::SetRendererWnd(HWND hWnd)
{
	if (!::IsWindow(hWnd))
		return false;

	m_hWnd = hWnd;

	::GetClientRect(m_hWnd, &m_rcWnd);

	return ResetDevice();
}

int D3DRenderer::RenderSample(FrameYUVBuffer* pFrameYUV)
{
	m_pFrameYUV->Copy(pFrameYUV);

	return RenderFrame(pFrameYUV);	
}

int D3DRenderer::RedrawFrame()
{
	if (!m_pFrameYUV)
		return -1;

	return RenderFrame(m_pFrameYUV);
}

bool D3DRenderer::InitD3D()
{
	if (m_pDD)
	{
		m_pDD.Release();
		m_pDD = NULL;
	}

	m_pDD = Direct3DCreate9(D3D_SDK_VERSION);
	if (!m_pDD)
		return false;

	return true;
}

bool D3DRenderer::CreateDevice()
{
	if (!m_pDD)
		m_pDD = Direct3DCreate9(D3D_SDK_VERSION);

	if (!m_pDD)
		return false;

	if (m_pDev)
	{
		m_pDev.Release();
		m_pDev = NULL;
	}
	
	D3DPRESENT_PARAMETERS d3dpp;
	memset(&d3dpp, 0, sizeof(D3DPRESENT_PARAMETERS));
	d3dpp.Windowed = TRUE;
	d3dpp.hDeviceWindow = m_hWnd;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	if ( m_pDD->CreateDevice(D3DADAPTER_DEFAULT, 
		D3DDEVTYPE_HAL, 
		m_hWnd, 
		D3DCREATE_SOFTWARE_VERTEXPROCESSING|D3DCREATE_MULTITHREADED,
		&d3dpp,
		&m_pDev) < 0 )
	{
		return false;
	}

	return true;
}

bool D3DRenderer::ResetDevice()
{
	ReleaseD3D();

	return CreateDevice();
}

int D3DRenderer::RenderFrame(FrameYUVBuffer* pFrameYUV)
{
	if (!pFrameYUV)
		return -1;

	if (!m_pDev)
	{
		if (!ResetDevice())
			return -2;
	}

	if (!::IsWindow(m_hWnd))
		return -3;

	RECT rc;
	::GetClientRect(m_hWnd, &rc);
	if ((rc.right - rc.left) != (m_rcWnd.right - m_rcWnd.left)
		|| (rc.bottom - rc.top) != (m_rcWnd.bottom - m_rcWnd.top))
	{
		if (!ResetDevice())
			return -2;
		memcpy(&m_rcWnd, &rc, sizeof(RECT));
	}

	HRESULT hr = m_pDev->TestCooperativeLevel();
	switch ( hr )
	{
	case D3DERR_DEVICENOTRESET:
		if (!ResetDevice()) 
			return -2;
		break;
	case D3D_OK:
		break;
	default:
		return -4;
	}

	if (m_pOffscreenSurf)
	{
		D3DSURFACE_DESC desc;
		if ( FAILED(m_pOffscreenSurf->GetDesc(&desc)) )
			return -5;
		if ( desc.Width != pFrameYUV->nWidth || desc.Height != pFrameYUV->nHeight )
		{
			m_pOffscreenSurf.Release();
			m_pOffscreenSurf = NULL;
		}
	}

	if (!m_pOffscreenSurf)
	{
		if ( FAILED(m_pDev->CreateOffscreenPlainSurface(pFrameYUV->nWidth, 
			pFrameYUV->nHeight,
			(D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'),
			D3DPOOL_DEFAULT,
			&m_pOffscreenSurf,
			NULL)) )
		{
			return -6;
		}
	}

	m_pDev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0);

	if (FAILED(RenderDataToOffscreenSurface(pFrameYUV)))
		return -7;

	m_pDev->BeginScene();

	CComPtr<IDirect3DSurface9> pBackBuffer = NULL;
	if (FAILED(m_pDev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
		return -8;

	if (FAILED(m_pDev->StretchRect(m_pOffscreenSurf, NULL, pBackBuffer, NULL, D3DTEXF_LINEAR)))
		return -9;

	DrawRect(m_rcDraw);

	m_pDev->EndScene();

	m_pDev->Present(NULL, NULL, NULL, NULL);

	return 0;
}

int D3DRenderer::RenderDataToOffscreenSurface(FrameYUVBuffer* pFrameYUV)
{
	if (!pFrameYUV || !m_pOffscreenSurf)
		return -1;

	D3DLOCKED_RECT d3drc;
	if ( FAILED(m_pOffscreenSurf->LockRect(&d3drc, NULL, D3DLOCK_DISCARD)) )
		return -2;

	//Y
	char* pSrc = (char*)pFrameYUV->GetData();
	char* pDst = (char*)d3drc.pBits;
	for (int i = 0; i < pFrameYUV->nHeight; i++)
	{
		memcpy(pDst, pSrc, pFrameYUV->nWidth);
		pDst += d3drc.Pitch;
		pSrc += pFrameYUV->nPitch;
	}

	//U
	pDst += d3drc.Pitch * pFrameYUV->nHeight / 4;
	for (int i = 0; i < pFrameYUV->nHeight/2; i++)
	{
		memcpy(pDst, pSrc, pFrameYUV->nWidth/2);
		pDst += d3drc.Pitch/2;
		pSrc += pFrameYUV->nPitch/2;
	}

	//V
	pDst -= d3drc.Pitch * pFrameYUV->nHeight / 2;
	for (int i = 0; i < pFrameYUV->nHeight/2; i++)
	{
		memcpy(pDst, pSrc, pFrameYUV->nWidth/2);
		pDst += d3drc.Pitch/2;
		pSrc += pFrameYUV->nPitch/2;
	}

	if ( FAILED(m_pOffscreenSurf->UnlockRect()) )
		return -2;

	return 0;
}

void D3DRenderer::ReleaseD3D()
{
	if (m_pOffscreenSurf)
	{
		m_pOffscreenSurf.Release();
		m_pOffscreenSurf = NULL;
	}

	if (m_pDev)
	{
		m_pDev.Release();
		m_pDev = NULL;
	}

	if (m_pDD)
	{
		m_pDD.Release();
		m_pDD = NULL;
	}
}

void D3DRenderer::TransRect(const RECT& rcSrc, RECT& rcDst)
{
	if (!m_pOffscreenSurf)
		return;

	D3DSURFACE_DESC desc;
	m_pOffscreenSurf->GetDesc(&desc);

	float x_scale = desc.Width / (float)(m_rcWnd.right - m_rcWnd.left);
	float y_scale = desc.Height / (float)(m_rcWnd.bottom - m_rcWnd.top);

	rcDst.left  = rcSrc.left  * x_scale;
	rcDst.top   = rcSrc.top   * y_scale;
	rcDst.right = rcSrc.right * x_scale;
	rcDst.bottom = rcSrc.bottom * y_scale;

	memcpy(&m_rcDraw, &rcSrc, sizeof(RECT));
}

int D3DRenderer::DrawRect(const RECT& rc)
{
	if (rc.left == rc.right || rc.top == rc.bottom)
		return -1;

	CComPtr<ID3DXLine> pDXLine;
	if (D3DXCreateLine(m_pDev, &pDXLine) < 0)
		return -2;

	D3DXVECTOR2 vec[5];
	vec[0].x = rc.left;		vec[0].y = rc.top;
	vec[1].x = rc.left;		vec[1].y = rc.bottom;
	vec[2].x = rc.right;	vec[2].y = rc.bottom;
	vec[3].x = rc.right;	vec[3].y = rc.top;
	vec[4].x = rc.left;		vec[4].y = rc.top;

	pDXLine->SetWidth(2);
	pDXLine->Begin();
	pDXLine->Draw(vec, 5, 0xffff0000);
	pDXLine->End();
	pDXLine.Release();

	return 0;
}