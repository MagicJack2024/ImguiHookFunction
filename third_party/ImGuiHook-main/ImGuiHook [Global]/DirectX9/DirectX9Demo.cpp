#include "DirectX9.h"
#include "DirectX9Demo.h"

#include <d3dx9tex.h>

#include "../Directories/ImGui/imgui.h"
#include "../Directories/ImGui/backends/imgui_impl_dx9.h"
#include "../Directories/ImGui/backends/imgui_impl_win32.h"


#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "d3d9.lib")
#include <gdiplus.h>
using namespace Gdiplus;

#include <tuple>
#include <vector>

bool ShowMenu = false;
bool ImGui_Initialised = false;

namespace Process {
	DWORD ID;
	HANDLE Handle;
	HWND Hwnd;
	HMODULE Module;
	WNDPROC WndProc;
	int WindowWidth;
	int WindowHeight;
	LPCSTR Title;
	LPCSTR ClassName;
	LPCSTR Path;
}

ULONG_PTR g_gdiplusToken;
Image* g_Image = nullptr;
GUID* g_pDimensionIDs = nullptr;

//gif
int currentFrame = 0;
std::vector<std::tuple<IDirect3DTexture9*, UINT, UINT>> gifTexturesTuple;
std::vector<float> frameDelays;
UINT frameDelay = 0;
UINT nFrameCount = 0;

std::tuple<IDirect3DTexture9*, UINT, UINT> ImageToTexture(Image* image, IDirect3DDevice9* device);

LPDIRECT3DTEXTURE9 g_png_texture = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
	{
		return ERROR_SUCCESS;
	}

	const ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse && (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP || uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONUP || uMsg == WM_MOUSEWHEEL || uMsg == WM_MOUSEMOVE)) {
		return ERROR_SUCCESS;
	}

	return CallWindowProc(Process::WndProc, hwnd, uMsg, wParam, lParam);
}

HRESULT APIENTRY MJEndScene(IDirect3DDevice9* pDevice) {
	if (!ImGui_Initialised) {

		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiWindowFlags_NoSavedSettings;

		ImGui_ImplWin32_Init(Process::Hwnd);
		ImGui_ImplDX9_Init(pDevice);
		ImGui_ImplDX9_CreateDeviceObjects();
		//ImGui::GetIO().ImeWindowHandle = Process::Hwnd;

		Process::WndProc = (WNDPROC)SetWindowLongPtr(Process::Hwnd, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);
		ImGui_Initialised = true;

		//Init GDI+
		GdiplusStartupInput gdiplusStartupInput;
		
		Status state = GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);


		//load gif 
		g_Image = Image::FromFile(L"testg.gif");
		if (Ok != g_Image->GetLastStatus())
		{
			MessageBox(NULL, "Failed to create texture from GDI+ image.", "Error", MB_ICONERROR | MB_OK);

			DisableAll();
			return S_FALSE;
		}

		UINT count = g_Image->GetFrameDimensionsCount();
		g_pDimensionIDs = new GUID[count];
		g_Image->GetFrameDimensionsList(g_pDimensionIDs, count);

		WCHAR strGuid[39];
		StringFromGUID2(g_pDimensionIDs[0], strGuid, 39);
		nFrameCount = g_Image->GetFrameCount(&g_pDimensionIDs[0]);

		for (int i = 0; i < nFrameCount; i++)
		{
			g_Image->SelectActiveFrame(g_pDimensionIDs, i);
			auto frameTextureTuple = ImageToTexture(g_Image, pDevice);
			gifTexturesTuple.push_back(frameTextureTuple);

			//PropertyTagFrameDelay是GDI+中预定义的一个GIG属性ID值，表示标签帧数据的延迟时间
			UINT TotalBuffer = g_Image->GetPropertyItemSize(PropertyTagFrameDelay);
			PropertyItem* pItem = (PropertyItem*)malloc(TotalBuffer);
			g_Image->GetPropertyItem(PropertyTagFrameDelay, TotalBuffer, pItem);

			frameDelay = ((UINT*)pItem[0].value)[i] * 10; // 单位是毫秒

			frameDelays.push_back(frameDelay / 1000.0f); // 转换为秒
		}

		//load png
		D3DXCreateTextureFromFileA(pDevice, "testp.png", &g_png_texture);
	}


	if (GetAsyncKeyState(VK_INSERT) & 1) ShowMenu = !ShowMenu;
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	//ImGui::GetIO().MouseDrawCursor = true;

	//rend gif
	static int currentFrame = 0;
	float deltaTime = ImGui::GetIO().DeltaTime; // 获取每帧的时间间隔
	float frameDuration = frameDelays[currentFrame]; // 假设每帧的播放时间相同
	static float elapsedTime = 0.0f;
	elapsedTime += deltaTime;

	if (elapsedTime >= frameDuration)
	{
		elapsedTime = 0.0f;
		currentFrame = (currentFrame + 1) % nFrameCount;
	}

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_NoMenuBar;
	//window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_AlwaysAutoResize;
	window_flags |= ImGuiWindowFlags_NoNav;
	window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	window_flags |= ImGuiWindowFlags_UnsavedDocument;

	ImGui::SetNextWindowPos(ImVec2(100, 100));

	//if (no_close)           p_open = NULL; // Don't pass our bool* to Begin
	ImGui::Begin("testgif", NULL, window_flags);
	ImGui::Image((void*)std::get<0>(gifTexturesTuple[currentFrame]), ImVec2((float)std::get<1>(gifTexturesTuple[currentFrame]), std::get<2>(gifTexturesTuple[currentFrame])));
	ImGui::End();

	//rend png

	D3DSURFACE_DESC desc;
	g_png_texture->GetLevelDesc(0, &desc);
	ImVec2 size = ImVec2((float)desc.Width / 3, (float)desc.Height / 3);

	ImGuiWindowFlags window_flags2 = 0;
	window_flags2 |= ImGuiWindowFlags_NoTitleBar;
	window_flags2 |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_NoMenuBar;
	//window_flags |= ImGuiWindowFlags_NoMove;
	window_flags2 |= ImGuiWindowFlags_NoResize;
	window_flags2 |= ImGuiWindowFlags_NoCollapse;
	window_flags2 |= ImGuiWindowFlags_AlwaysAutoResize;
	window_flags2 |= ImGuiWindowFlags_NoNav;
	window_flags2 |= ImGuiWindowFlags_NoBackground;
	window_flags2 |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	window_flags2 |= ImGuiWindowFlags_UnsavedDocument;
	//if (no_close)           p_open = NULL; // Don't pass our bool* to Begin

	ImGui::SetNextWindowPos(ImVec2(200, 100));

	ImGui::Begin("testpng", NULL, window_flags2);
	ImGui::Image((void*)g_png_texture, size);
	ImGui::End();

	//rend pop
	static float alpha = 0.0f;
	ImVec2 popupSize(100, 100);
	ImGui::SetNextWindowSize(popupSize);

	ImGuiWindowFlags window_flags3 = 0;

	window_flags3 |= ImGuiWindowFlags_AlwaysAutoResize;	
	window_flags3 |= ImGuiWindowFlags_UnsavedDocument;
	window_flags3 |= ImGuiWindowFlags_NoSavedSettings;
	//if (no_close)           p_open = NULL; // Don't pass our bool* to Begin
	// 
	// 打开弹窗的按钮
	ImGui::Begin("testpop", NULL, window_flags3);
	if (ImGui::Button("Open Popup"))
	{
		ImGui::OpenPopup("Example Popup");
		alpha = 0.0f; // 重置透明度
	}

	ImGui::SetNextWindowSize(ImVec2(350, 350));
	// 弹窗的实现
	if (ImGui::BeginPopupModal("Example Popup", nullptr))
	{
		// 逐渐增加弹窗的透明度，以达到淡入效果
		if (alpha < 1.0f)
		{
			alpha += 0.01f;
		}

		// 设置弹窗的透明度
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);

		ImColor colorTxt(0.0f, 0.0f, 0.0f, 1.0f);

		ImGui::Text("Hello from the Popup!", colorTxt);

		ImGui::PopStyleVar();

		ImVec2 windowSize = ImGui::GetWindowSize();

		//ImGui::SetWindowPos(ImVec2(100, 200), ImGuiCond_Always);

		ImDrawList* drawList = ImGui::GetWindowDrawList();

		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
		ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right

		ImColor color(1.0f, 1.0f, 1.0f, alpha); // 设置图片的透明度

		drawList->AddImage(g_png_texture, pos, ImVec2(pos.x + windowSize.x, pos.y + windowSize.y), uv_min, uv_max, color);

		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::End();

#if 0
	if (ShowMenu == true) {
		ImGui::ShowDemoWindow();
	}
#else

	ImGui::SetNextWindowPos(ImVec2(200, 200));
	ImGui::ShowDemoWindow();
#endif

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	return oEndScene(pDevice);
}

HRESULT APIENTRY MJPresent(IDirect3DDevice9* pDevice, const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion) {
	return oPresent(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

HRESULT APIENTRY MJDrawIndexedPrimitive(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
	return oDrawIndexedPrimitive(pDevice, Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT APIENTRY MJDrawPrimitive(IDirect3DDevice9* pDevice, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) {
	return oDrawPrimitive(pDevice, PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT APIENTRY MJSetTexture(LPDIRECT3DDEVICE9 pDevice, DWORD Sampler, IDirect3DBaseTexture9* pTexture) {
	return oSetTexture(pDevice, Sampler, pTexture);
}

HRESULT APIENTRY MJReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters) {
	return oReset(pDevice, pPresentationParameters);;
}

HRESULT APIENTRY MJSetStreamSource(LPDIRECT3DDEVICE9 pDevice, UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT sStride) {
	return oSetStreamSource(pDevice, StreamNumber, pStreamData, OffsetInBytes, sStride);
}

HRESULT APIENTRY MJSetVertexDeclaration(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexDeclaration9* pdecl) {
	return oSetVertexDeclaration(pDevice, pdecl);
}

HRESULT APIENTRY MJSetVertexShaderConstantF(LPDIRECT3DDEVICE9 pDevice, UINT StartRegister, const float* pConstantData, UINT Vector4fCount) {
	return oSetVertexShaderConstantF(pDevice, StartRegister, pConstantData, Vector4fCount);
}

HRESULT APIENTRY MJSetVertexShader(LPDIRECT3DDEVICE9 pDevice, IDirect3DVertexShader9* veShader) {
	return oSetVertexShader(pDevice, veShader);
}

HRESULT APIENTRY MJSetPixelShader(LPDIRECT3DDEVICE9 pDevice, IDirect3DPixelShader9* piShader) {
	return oSetPixelShader(pDevice, piShader);
}

DWORD WINAPI MainThread(LPVOID lpParameter) {
	bool WindowFocus = false;
	while (WindowFocus == false) {
		DWORD ForegroundWindowProcessID;
		GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);
		if (GetCurrentProcessId() == ForegroundWindowProcessID) {

			Process::ID = GetCurrentProcessId();
			Process::Handle = GetCurrentProcess();
			Process::Hwnd = GetForegroundWindow();

			RECT TempRect;
			GetWindowRect(Process::Hwnd, &TempRect);
			Process::WindowWidth = TempRect.right - TempRect.left;
			Process::WindowHeight = TempRect.bottom - TempRect.top;

			char TempTitle[MAX_PATH];
			GetWindowText(Process::Hwnd, TempTitle, sizeof(TempTitle));
			Process::Title = TempTitle;

			char TempClassName[MAX_PATH];
			GetClassName(Process::Hwnd, TempClassName, sizeof(TempClassName));
			Process::ClassName = TempClassName;

			char TempPath[MAX_PATH];
			GetModuleFileNameEx(Process::Handle, NULL, TempPath, sizeof(TempPath));
			Process::Path = TempPath;

			WindowFocus = true;
		}
	}
	bool InitHook = false;
	while (InitHook == false) {
		if (DirectX9::Init() == true) {
			CreateHook(42, (void**)&oEndScene, MJEndScene);
			CreateHook(17, (void**)&oPresent, MJPresent);
			CreateHook(82, (void**)&oDrawIndexedPrimitive, MJDrawIndexedPrimitive);
			CreateHook(81, (void**)&oDrawPrimitive, MJDrawPrimitive);
			CreateHook(65, (void**)&oSetTexture, MJSetTexture);
			CreateHook(16, (void**)&oReset, MJReset);
			CreateHook(100, (void**)&oSetStreamSource, MJSetStreamSource);
			CreateHook(87, (void**)&oSetVertexDeclaration, MJSetVertexDeclaration);
			CreateHook(94, (void**)&oSetVertexShaderConstantF, MJSetVertexShaderConstantF);
			CreateHook(92, (void**)&oSetVertexShader, MJSetVertexShader);
			CreateHook(107, (void**)&oSetPixelShader, MJSetPixelShader);
			InitHook = true;
		}
	}
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		if (ChecktDirectXVersion(DirectXVersion.D3D9) == true) {
			Process::Module = hModule;
			CreateThread(0, 0, MainThread, 0, 0, 0);
		}
		break;
	case DLL_PROCESS_DETACH:
		//FreeLibraryAndExitThread(hModule, TRUE);

		if (nullptr != g_png_texture)
		{
			g_png_texture->Release();
			g_png_texture = nullptr;
		}

		for (auto iter = gifTexturesTuple.begin(); iter != gifTexturesTuple.end(); iter++)
		{
			auto texturePtr = std::get<0>(*iter);

			if (nullptr != texturePtr)
			{
				texturePtr->Release();
				texturePtr = nullptr;
			}
		}
		if (nullptr != g_pDimensionIDs)
		{
			delete g_pDimensionIDs;
		}

		if (nullptr != g_Image)
		{
			delete g_Image;
		}

		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		DisableAll();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	default:
		break;
	}
	return TRUE;
}

std::tuple<IDirect3DTexture9*, UINT, UINT> ImageToTexture(Image* image, IDirect3DDevice9* device)
{
	if (!device || !image)
	{
		return std::make_tuple(nullptr, 0, 0);
	}

	// 获取图像的宽度和高度
	UINT width = image->GetWidth();
	UINT height = image->GetHeight();

	// 创建一个 Bitmap 对象，以便我们可以访问像素数据
	Gdiplus::Bitmap bitmap(width, height, PixelFormat32bppARGB);
	Gdiplus::Graphics graphics(&bitmap);
	graphics.DrawImage(image, 0, 0, width, height);

	// 锁定位图以访问像素数据
	Gdiplus::BitmapData bitmapData;
	Gdiplus::Rect rect(0, 0, width, height);
	bitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);

	// 创建 Direct3D 纹理
	IDirect3DTexture9* texture = nullptr;
	HRESULT hr = D3DXCreateTexture(device, width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture);

	if (FAILED(hr))
	{
		MessageBox(NULL, "Failed to create texture from GDI+ image.", "Error", MB_ICONERROR | MB_OK);
		return std::make_tuple(nullptr, 0, 0);
	}

	D3DLOCKED_RECT lockedRect;
	if (SUCCEEDED(texture->LockRect(0, &lockedRect, NULL, 0)))
	{
		BYTE* dest = static_cast<BYTE*>(lockedRect.pBits);
		BYTE* src = static_cast<BYTE*>(bitmapData.Scan0);
		for (UINT y = 0; y < height; ++y)
		{
			memcpy(dest, src, width * 4);
			dest += lockedRect.Pitch;
			src += bitmapData.Stride;
		}
		texture->UnlockRect(0);
	}

	// 解锁位图
	bitmap.UnlockBits(&bitmapData);

	return std::make_tuple(texture, width, height);
}
