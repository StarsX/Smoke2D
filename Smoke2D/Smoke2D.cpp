//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "Smoke2D.h"

using namespace std;
using namespace Concurrency;
using namespace DirectX;
using namespace DX;
using namespace XSDX;

using upCDXUTTextHelper = unique_ptr<CDXUTTextHelper>;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CDXUTDialogResourceManager		g_DialogResourceManager;	// manager for shared resources of dialogs
//CModelViewerCamera					g_Camera;					// A model viewing camera
//CD3DSettingsDlg						g_D3DSettingsDlg;			// Device settings dialog
//CDXUTDialog							g_HUD;						// manages the 3D   
CDXUTDialog						g_SampleUI;					// dialog for sample specific controls
bool							g_bShowHelp = false;		// If true, it renders the UI control text
bool							g_bShowFPS = false;			// If true, it shows the FPS
bool							g_bViscous = false;
bool							g_bLoadingComplete = false;

upCDXUTTextHelper				g_pTxtHelper;

upFluid2D						g_pFluid;

spShader						g_pShader;
spState							g_pState;

CPDXBuffer						g_pCBEnvironment;
//CPDXBuffer					g_pCBImmutable;

XMFLOAT2						g_vViewport;
XMFLOAT2						g_vMouse;
XMFLOAT4						g_vForce;

#define DELTA_TIME				0.5f

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_TOGGLEWARP          4

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext);
void CALLBACK OnMouse(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
	bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
	int xPos, int yPos, void *pUserContext);
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);

bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext);
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext);
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext);
void CALLBACK OnD3D11DestroyDevice(void* pUserContext);
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext);

void InitApp();
void RenderText();


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	AllocConsole();
	FILE *stream;
	freopen_s(&stream, "CONOUT$", "w+t", stdout);
	freopen_s(&stream, "CONIN$", "r+t", stdin);
#endif

	// DXUT will create and use the best device
	// that is available on the system depending on which D3D callbacks are set below

	//g_CBEnvironment.m_vLightPt = XMFLOAT4(10.0f, 45.0f, -75.0f, 0.0f);

	// Set DXUT callbacks
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackMouse(OnMouse, true);
	DXUTSetCallbackKeyboard(OnKeyboard);
	DXUTSetCallbackFrameMove(OnFrameMove);

	DXUTSetCallbackD3D11DeviceAcceptable(IsD3D11DeviceAcceptable);
	DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
	DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
	DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);
	DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
	DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);

	InitApp();
	DXUTInit(true, true, nullptr); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings(false, true); // Show the cursor and clip it when in full screen
	DXUTCreateWindow(L"Smoke 2D");
	DXUTCreateDevice(D3D_FEATURE_LEVEL_10_1, true, 1280, 960);
	DXUTMainLoop(); // Enter into the DXUT render loop

#if defined(DEBUG) | defined(_DEBUG)
	FreeConsole();
#endif

	return DXUTGetExitCode();
}

//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	g_SampleUI.Init(&g_DialogResourceManager);
	g_SampleUI.SetCallback(OnGUIEvent);
}

//--------------------------------------------------------------------------------------
// Called right before creating a D3D device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
	pDeviceSettings->d3d11.SyncInterval = 0;

	return true;
}

//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	// Update the camera's position based on user input 
	//g_Camera.FrameMove(fElapsedTime);
	//if (g_bLoadingComplete) g_pCharacter->FrameMove(fTime);
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void RenderText()
{
	UINT nBackBufferHeight = DXUTGetDXGIBackBufferSurfaceDesc()->Height;

	g_pTxtHelper->Begin();
	g_pTxtHelper->SetInsertionPos(2, 0);
	g_pTxtHelper->SetForegroundColor(Colors::Yellow);
	g_pTxtHelper->DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
	g_pTxtHelper->DrawTextLine(DXUTGetDeviceStats());

	// Draw help
	if (g_bShowHelp)
	{
		g_pTxtHelper->SetInsertionPos(2, nBackBufferHeight - 20 * 4);
		g_pTxtHelper->SetForegroundColor(Colors::Orange);
		g_pTxtHelper->DrawTextLine(L"Controls:");

		g_pTxtHelper->SetInsertionPos(20, nBackBufferHeight - 20 * 3);
		g_pTxtHelper->DrawTextLine(L"Rotate camera: Left mouse button\n"
			L"Zoom camera: Mouse wheel scroll\n");

		g_pTxtHelper->SetInsertionPos(550, nBackBufferHeight - 20 * 3);
		g_pTxtHelper->DrawTextLine(L"Hide help: F1\n"
			L"Quit: ESC\n");
	}
	else
	{
		g_pTxtHelper->SetForegroundColor(Colors::White);
		g_pTxtHelper->DrawTextLine(L"Press F1 for help");
	}

	g_pTxtHelper->End();
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext)
{
	// Pass messages to dialog resource manager calls so GUI state is updated correctly
	*pbNoFurtherProcessing = g_DialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing) return 0;

	// Give the dialogs a chance to handle the message first
	*pbNoFurtherProcessing = g_SampleUI.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing) return 0;

	// Pass all remaining windows messages to camera so it can respond to user input
	//g_Camera.HandleMessages(hWnd, uMsg, wParam, lParam);

	return 0;
}

//--------------------------------------------------------------------------------------
// Handle mouse events
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
	bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
	int xPos, int yPos, void *pUserContext)
{
	static const float fForceScl = SIM_SCALE * 0.3f;
	static bool bStart = true;
	XMFLOAT2 vMouse(static_cast<float>(xPos), static_cast<float>(yPos));
	if (bLeftButtonDown) {
		if (bStart) {
			g_vMouse = vMouse;
			bStart = false;
		}
		else {
			g_vForce.x = fForceScl * (vMouse.x - g_vMouse.x);
			g_vForce.y = fForceScl * (vMouse.y - g_vMouse.y);
			g_vForce.z = vMouse.x * SIM_SCALE + g_vForce.x;
			g_vForce.w = vMouse.y * SIM_SCALE + g_vForce.y;
			g_vMouse = vMouse;
		}
	}
	else {
		g_vForce.x = 0;
		g_vForce.y = 0;
		bStart = true;
	}
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
	if (bKeyDown) {
		switch (nChar) {
			case VK_F1:
				g_bShowHelp = !g_bShowHelp; break;
			case VK_F2:
				g_bShowFPS = !g_bShowFPS; break;
			case 'V':
				g_bViscous = !g_bViscous; break;
			case 'J':
				g_vForce = XMFLOAT4(0, -2.0f, 0.5f * g_vViewport.x * SIM_SCALE, 0.9f * g_vViewport.y * SIM_SCALE);
				break;
		}
	}
	else {
		switch (nChar) {
			case 'J':
				g_vForce.x = 0;
				g_vForce.y = 0;
				break;
		}
	}
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
	return true;
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	HRESULT hr;

	ID3D11DeviceContext *pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	V_RETURN(g_DialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));
	g_pTxtHelper = make_unique<CDXUTTextHelper>(pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15);

	// Load shaders asynchronously.
	g_pShader = make_shared<Shader>(pd3dDevice);
	g_pState = make_shared<State>(pd3dDevice);

	g_pFluid = make_unique<Fluid2D>(pd3dDevice, g_pShader, g_pState);

	auto loadVSTask = g_pShader->CreateVertexShader(L"QuadVS.cso", 0);
	auto loadPSTask = g_pShader->CreatePixelShader(L"Smoke2DPS.cso", 0);
	auto loadCSTask = g_pShader->CreateComputeShader(L"Advect2DCS.cso", 0);
	loadCSTask = loadCSTask && g_pShader->CreateComputeShader(L"Diffuse2DCS.cso", 1);
	loadCSTask = loadCSTask && g_pShader->CreateComputeShader(L"Impulse2DCS.cso", 2);
	loadCSTask = loadCSTask && g_pShader->CreateComputeShader(L"Divergence2DCS.cso", 3);
	loadCSTask = loadCSTask && g_pShader->CreateComputeShader(L"Pressure2DCS.cso", 4);
	loadCSTask = loadCSTask && g_pShader->CreateComputeShader(L"Project2DCS.cso", 5);
	loadCSTask = loadCSTask && g_pShader->CreateComputeShader(L"AdvectDye2DCS.cso", 6);
	loadCSTask = loadCSTask && g_pShader->CreateComputeShader(L"ImpulseDye2DCS.cso", 7);
	loadCSTask = loadCSTask && g_pShader->CreateComputeShader(L"Boundary2DCS.cso", 8);
	
	auto createShaderTask = loadVSTask && loadPSTask && loadCSTask;

	auto createConstTask = create_task([pd3dDevice]() {
		// Setup constant buffers
		CD3D11_BUFFER_DESC desc(sizeof(XMFLOAT4), D3D11_BIND_CONSTANT_BUFFER);
		ThrowIfFailed(pd3dDevice->CreateBuffer(&desc, nullptr, &g_pCBEnvironment));
		/*CD3D11_BUFFER_DESC desc(sizeof(CBEnvironment), D3D11_BIND_CONSTANT_BUFFER);
		ThrowIfFailed(pd3dDevice->CreateBuffer(&desc, nullptr, &g_pCBEnvironment));

		desc.ByteWidth = sizeof(CBImmutable);
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		CBImmutable im;
		im.m_vDirectional = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		im.m_vAmbient = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
		im.m_vSpecular = XMFLOAT4(0.8f, 0.8f, 0.8f, 0.15f);
		D3D11_SUBRESOURCE_DATA dsd;
		dsd.pSysMem = &im;
		dsd.SysMemPitch = 0;
		dsd.SysMemSlicePitch = 0;
		ThrowIfFailed(pd3dDevice->CreateBuffer(&desc, &dsd, &g_pCBImmutable));*/
	});

	// Once the cube is loaded, the object is ready to be rendered.
	(createShaderTask && createConstTask).then([]() {
		g_pShader->ReleaseShaderBuffers();

		// View
		// Setup the camera's view parameters
		/*XMFLOAT4 vLookAtPt(0.0f, 8.0f, 0.0f, 25.0f);
		auto vEyePt = XMVectorSet(vLookAtPt.x, vLookAtPt.y, vLookAtPt.z - vLookAtPt.w, 0.0f);
		g_Camera.SetViewParams(vEyePt, XMLoadFloat4(&vLookAtPt));*/

		g_bLoadingComplete = true;
	});

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	HRESULT hr;

	V_RETURN(g_DialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

	// Setup the camera's projection parameters
	auto fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	//g_Camera.SetProjParams(XM_PIDIV4, fAspectRatio, 1.0f, 1000.0f);
	//g_Camera.SetWindow(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);
	//g_Camera.SetButtonMasks(MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON);

	// Initialize window size dependent resources
	// Viewport clipping
	auto iVpNum = 1u;
	D3D11_VIEWPORT viewport;
	DXUTGetD3D11DeviceContext()->RSGetViewports(&iVpNum, &viewport);

	const auto iWidth = pBackBufferSurfaceDesc->Width;
	const auto iHeight = pBackBufferSurfaceDesc->Height;
	g_vViewport = XMFLOAT2(viewport.Width, viewport.Height);
	if (g_pFluid.get())
		g_pFluid->Init(iWidth * SIM_SCALE, iHeight * SIM_SCALE);

	// Set window size dependent constants

	//g_HUD.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
	//g_HUD.SetSize(170, 170);
	g_SampleUI.SetLocation(pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300);
	g_SampleUI.SetSize(170, 300);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext)
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!g_bLoadingComplete) return;

	// Set render targets to the screen.
	ID3D11RenderTargetView *pRTVs[] = { DXUTGetD3D11RenderTargetView() };
	ID3D11DepthStencilView *pDSV = DXUTGetD3D11DepthStencilView();
	pd3dImmediateContext->ClearRenderTargetView(pRTVs[0], DirectX::Colors::CornflowerBlue);
	pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, pDSV);

	// Prepare the constant buffer to send it to the graphics device.
	// Get the projection & view matrix from the camera class
	auto mWorld = XMMatrixIdentity();
	//auto mProj = g_Camera.GetProjMatrix();
	//auto mView = g_Camera.GetViewMatrix();
	//auto mViewProj = XMMatrixMultiply(mView, mProj);

	//XMStoreFloat4(&g_CBEnvironment.m_vEyePt, mViewInv.r[3]);

	// Set PS constants
	pd3dImmediateContext->UpdateSubresource(g_pCBEnvironment.Get(), 0, nullptr, &g_vViewport, 0, 0);
	pd3dImmediateContext->PSSetConstantBuffers(0, 1, g_pCBEnvironment.GetAddressOf());
	/*ID3D11Buffer *pCBs[] = { g_pCBEnvironment.Get(), g_pCBImmutable.Get() };
	pd3dImmediateContext->UpdateSubresource(g_pCBEnvironment.Get(), 0, nullptr, &g_CBEnvironment, 0, 0);
	pd3dImmediateContext->PSSetConstantBuffers(0, 2, pCBs);*/

	// Render
	g_pFluid->Simulate(DELTA_TIME, g_vForce, 20, g_bViscous ? 10 : 0);
	g_pFluid->Render();

	DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"HUD / Stats");
	if (g_bShowFPS) {
		g_SampleUI.OnRender(fElapsedTime);
		RenderText();
	}
	DXUT_EndPerfEvent();
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
	g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
	g_DialogResourceManager.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();

	g_bLoadingComplete = false;
	g_pCBEnvironment.Reset();
	g_pTxtHelper.reset();
	g_pFluid.reset();
	g_pState.reset();
	g_pShader.reset();
}
