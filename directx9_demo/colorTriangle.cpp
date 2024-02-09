//////////////////////////////////////////////////////////////////////////////////////////////////
// 
// File: colorTriangle.cpp
// 
// Author: Frank Luna (C) All Rights Reserved
//
// System: AMD Athlon 1800+ XP, 512 DDR, Geforce 3, Windows XP, MSVC++ 7.0 
//
// Desc: Renders two colored triangles, one shaded with flat shading and the
//       other shaded with Gouraud shading.  Demontrates vertex colors and,
//       the shading render states.
//          
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include "resource.h"

//
// Globals
//

IDirect3DDevice9* Device = 0; 

const int Width  = 1280;
const int Height = 800;

D3DXMATRIX WorldMatrix;

IDirect3DVertexBuffer9* Triangle = 0;
IDirect3DVertexBuffer9* TrianglePyramid = 0;

LPDIRECT3DINDEXBUFFER9 indexBuffer = NULL;

ID3DXFont* g_pFont = NULL;

HINSTANCE g_hinstImguiLib = NULL;

//
// Classes and Structures
//
struct ColorVertex
{
	ColorVertex(){}

	ColorVertex(float x, float y, float z, D3DCOLOR c)
	{
		_x = x;	 _y = y;  _z = z;  _color = c;
	}

	float _x, _y, _z;
	D3DCOLOR _color;

	static const DWORD FVF;
};
const DWORD ColorVertex::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

// �������㻺�������������׶�Ķ�������
struct Vertex
{
	float x, y, z;
	DWORD color;

	static const DWORD FVF;
};
const DWORD Vertex::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

//
// Framework Functions
//
bool Setup()
{
	//
	// Create the vertex buffer.
	//

	Device->CreateVertexBuffer(
		3 * sizeof(ColorVertex), 
		D3DUSAGE_WRITEONLY,
		ColorVertex::FVF,
		D3DPOOL_MANAGED,
		&Triangle,
		0);

	Device->CreateVertexBuffer(4 * sizeof(Vertex), 0, Vertex::FVF, D3DPOOL_MANAGED, &TrianglePyramid, NULL);

	Vertex* vertices = NULL;
	TrianglePyramid->Lock(0, 0, (void**)&vertices, 0);
	vertices[0] = { 0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 0) }; // ����
	vertices[1] = { -1.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(0, 255, 0) }; // ���涥��1
	vertices[2] = { 1.0f, 0.0f, -1.0f, D3DCOLOR_XRGB(0, 0, 255) }; // ���涥��2
	vertices[3] = { 0.0f, 0.0f, 1.0f, D3DCOLOR_XRGB(255, 255, 0) }; // ���涥��3
	TrianglePyramid->Unlock();

	// ��������������
	WORD indices[] =
	{
		0, 1, 2, // ǰ����
		0, 2, 3, // �����
		0, 3, 4, // �����
		0, 4, 1, // �Ҳ���
		1, 4, 2, // �ײ�
		2, 4, 3  // �ײ�
	};

	Device->CreateIndexBuffer(sizeof(indices), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &indexBuffer, NULL);

	void* pIndices;
	indexBuffer->Lock(0, sizeof(indices), (void**)&pIndices, 0);

	memcpy(pIndices, indices, sizeof(indices));
	indexBuffer->Unlock();

	HRESULT hr = D3DXCreateFont(
		Device, // ָ��IDirect3DDevice9��ָ��
		24,           // ����߶ȣ�����
		0,            // �����ȣ�0��ʾ�Զ����㣩
		FW_BOLD,      // ����Ӵ�
		1,            // Mip����
		FALSE,        // �Ƿ�б��
		DEFAULT_CHARSET, // �ַ���
		OUT_DEFAULT_PRECIS, // �������
		ANTIALIASED_QUALITY, // Ʒ��
		DEFAULT_PITCH | FF_DONTCARE, // ����ϵ�кͼ��
		"Arial",    // ��������
		&g_pFont);   // ����������

	if (FAILED(hr))
	{
		::MessageBox(NULL, "ceate font error", "error", MB_OK);
	}

	//
	// Fill the buffer with the triangle data.
	//

	ColorVertex* v;
	Triangle->Lock(0, 0, (void**)&v, 0);

	v[0] = ColorVertex(-1.0f, 0.0f, 2.0f, D3DCOLOR_XRGB(255,   0,   0));
	v[1] = ColorVertex( 0.0f, 1.0f, 2.0f, D3DCOLOR_XRGB(  0, 255,   0));
	v[2] = ColorVertex( 1.0f, 0.0f, 2.0f, D3DCOLOR_XRGB(  0,   0, 255));

	Triangle->Unlock();

	//
	// Set the projection matrix.
	//

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
			&proj,
			D3DX_PI * 0.5f, // 90 - degree
			(float)Width / (float)Height,
			1.0f,
			1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	//
	// Turn off lighting.
	//

	Device->SetRenderState(D3DRS_LIGHTING, false);

	return true;
}

void Cleanup()
{
	if (g_hinstImguiLib != NULL)
	{
		FreeLibrary(g_hinstImguiLib);
	}

	d3d::Release<IDirect3DVertexBuffer9*>(Triangle);

	d3d::Release<IDirect3DVertexBuffer9*>(TrianglePyramid);

	d3d::Release<LPDIRECT3DINDEXBUFFER9>(indexBuffer);

	d3d::Release<ID3DXFont*>(g_pFont);
}

bool Display(float timeDelta)
{
	if( Device )
	{
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
		Device->BeginScene();

		Device->SetFVF(ColorVertex::FVF);
		Device->SetStreamSource(0, Triangle, 0, sizeof(ColorVertex));

		// draw the triangle to the left with flat shading
		D3DXMatrixTranslation(&WorldMatrix, -1.25f * 3, 0.0f, 0.0f);
		Device->SetTransform(D3DTS_WORLD, &WorldMatrix);

		Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
		Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);

		// draw the triangle to the right with gouraud shading
		D3DXMatrixTranslation(&WorldMatrix, 1.25f * 3, 0.0f, 0.0f);
		Device->SetTransform(D3DTS_WORLD, &WorldMatrix);

		Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
		Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);



		D3DXMatrixTranslation(&WorldMatrix, 0.0f, 0.0f, 10.0f);
		Device->SetTransform(D3DTS_WORLD, &WorldMatrix);

		D3DXVECTOR3 eye(0.0f, 0.0f, -1.0f); // �����λ��
		D3DXVECTOR3 at(0.0f, 0.0f, 0.0f); // Ŀ���
		D3DXVECTOR3 up(0.0f, 1.0f, 0.0f); // ��������

		// ����۲����
		D3DXMATRIX viewMatrix;
		D3DXMatrixLookAtLH(&viewMatrix, &eye, &at, &up);
		Device->SetTransform(D3DTS_VIEW, &viewMatrix);

		//// ������ת����
		D3DXMATRIX matRotation;
		D3DXMatrixRotationY(&matRotation, timeGetTime() / 1000.0f); // ��ʱ����ת
		Device->SetTransform(D3DTS_WORLD, &matRotation);

		// ��������׶
		Device->SetStreamSource(0, TrianglePyramid, 0, sizeof(Vertex));
		Device->SetFVF(Vertex::FVF);
		Device->SetIndices(indexBuffer);

		// ��������׶
		Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 5, 0, 6);

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
	}
	return true;
}


//
// WndProc
//
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// �����˵�ѡ��:
		switch (wmId)
		{
		case ID_LOAD_IMGUI:
			g_hinstImguiLib = LoadLibrary("directx9_demo.dll");
			if (NULL == g_hinstImguiLib)
			{
				::MessageBox(NULL, "Load Imgui Lib failure", "error", MB_OK);
			}
			break;
		}
	}
	break;

	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
		
	case WM_KEYDOWN:
		//if( wParam == VK_ESCAPE )
		//	::DestroyWindow(hwnd);
		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{
	wchar_t exePath[MAX_PATH];
	GetModuleFileNameW(NULL, exePath, MAX_PATH);
	wchar_t* lastSlash = wcsrchr(exePath, L'\\');
	if (lastSlash)
	{
		*lastSlash = L'\0';
	}

	_wchdir(exePath);

	if(!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
		
	if(!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop( Display );

	Cleanup();

	int ref = Device->Release();
	if (ref != 0)
	{
		::MessageBoxA(NULL, "Device reference !=0 when exitApp", "error", MB_OK);
	}

	return 0;
}