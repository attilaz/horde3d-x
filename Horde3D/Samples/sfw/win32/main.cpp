#include "../sfw.h"
#include <windows.h>

static const int winWidth = 1024;
static const int winHeight = 576;
static const char* kWindowClassName = "Horde3DSample";

static HWND createWindow();
static void destroyWindow(HWND hWnd);
static void SetupPixelFormat(HDC hDC);

#ifdef HORDE3D_GL
class Device
{
public:
	Device(HWND hWnd);
	~Device();
	void SwapBuffers();
	void* GetNativeHandle() { return NULL; }
protected:
	HWND _hWnd;
	HGLRC _hRC; 
};

#elif HORDE3D_GLES2
#elif HORDE3D_D3D11
#include "DXGI.h" 
#include <d3d11.h>
class Device
{
public:
	Device(HWND hWnd);
	~Device();
	virtual void SwapBuffers();
	virtual void* GetNativeHandle() { return _d3dDevice; }
protected:
	ID3D11Device*            _d3dDevice;
	IDXGISwapChain*          _swapChain;
};

#endif

static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static sfwKey::Code VirtualKeyCodeToSfwKeyCode(WPARAM VirtualKey, LPARAM Flags);

int APIENTRY WinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    // Main message loop:
    MSG msg;
    LARGE_INTEGER nFreq;
    LARGE_INTEGER nLast;
    LARGE_INTEGER nNow;

    QueryPerformanceFrequency(&nFreq);
    QueryPerformanceCounter(&nLast);

	HWND hWnd = createWindow();
	if (hWnd==0) return -1;
	HDC hDC = GetDC(hWnd);

	Device* device = new Device(hWnd);

    ShowWindow(hWnd, SW_SHOW);

	sfwMain(winWidth, winHeight, device->GetNativeHandle());

	bool quit = false;

    while (!quit)
    {
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
			if (WM_QUIT == msg.message)
				quit = true;

			// Deal with windows message.
			TranslateMessage(&msg);
			DispatchMessage(&msg);
        }

            // Get current time tick.
		QueryPerformanceCounter(&nNow);
		float elapsed = (float) ((double)(nNow.QuadPart - nLast.QuadPart) / (double)nFreq.QuadPart);
		nLast = nNow;
		sfwUpdate( elapsed );

		device->SwapBuffers();
	}

	sfwUpdate( 1.0f/60.0f );

	sfwEvent e;
	e.type = sfwEventType::Quit;
	sfwPushEvent(&e);

	delete device;
	destroyWindow(hWnd);

    return (int) msg.wParam; 
}

static HWND createWindow()
{
	WNDCLASS  wc;        // Windows Class Structure

        // Redraw On Size, And Own DC For Window.
    wc.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc    = _WindowProc;                    // WndProc Handles Messages
    wc.cbClsExtra     = 0;                              // No Extra Window Data
    wc.cbWndExtra     = 0;                                // No Extra Window Data
    wc.hInstance      = GetModuleHandle( NULL );          // Set The Instance
	wc.hIcon		  = 0;
    wc.hCursor        = LoadCursor( NULL, IDC_ARROW );    // Load The Arrow Pointer
    wc.hbrBackground  = NULL;                           // No Background Required For GL
	wc.lpszMenuName = NULL;
    wc.lpszClassName  = kWindowClassName;               // Set The Class Name

    if (! RegisterClass(&wc) && 1410 != GetLastError())
		return 0;

        // center window position
    // create window
    HWND hWnd = CreateWindowEx(
        WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,    // Extended Style For The Window
        kWindowClassName,                                    // Class Name
        "Horde3D Sample",                                    // Window Title
        WS_CAPTION | WS_POPUPWINDOW | WS_MINIMIZEBOX,        // Defined Window Style
        0, 0,                                                // Window Position
        winWidth,                                               // Window Width
        winHeight,                                               // Window Height
        NULL,                                                // No Parent Window
        NULL,                                                // No Menu
        GetModuleHandle( NULL ),                             // Instance
        NULL );

    if (! hWnd)
		return 0;

		//resize and center window
    RECT rcWindow, rcClient, rcDesktop;
    GetWindowRect(hWnd, &rcWindow);
    GetWindowRect(GetDesktopWindow(), &rcDesktop); 
    GetClientRect(hWnd, &rcClient);

    // calculate new window width and height
    POINT ptDiff;
    ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
    ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
    rcClient.right = rcClient.left + winWidth;
    rcClient.bottom = rcClient.top + winHeight;

    AdjustWindowRectEx(&rcClient, GetWindowLong(hWnd, GWL_STYLE), FALSE, GetWindowLong(hWnd, GWL_EXSTYLE));

    int offsetX = (rcDesktop.right - rcDesktop.left - (winWidth + ptDiff.x)) / 2;
    offsetX = (offsetX > 0) ? offsetX : rcDesktop.left;
    int offsetY = (rcDesktop.bottom - rcDesktop.top - (winHeight + ptDiff.y)) / 2;
    offsetY = (offsetY > 0) ? offsetY : rcDesktop.top; 

    // change width and height
    SetWindowPos(hWnd, 0, offsetX, offsetY, winWidth + ptDiff.x, winHeight + ptDiff.y,
                 SWP_NOCOPYBITS | SWP_NOOWNERZORDER | SWP_NOZORDER);

	return hWnd;
}

void destroyWindow(HWND hWnd)
{
    if (hWnd)
    {
        DestroyWindow(hWnd);
        hWnd = NULL;
    }
    UnregisterClass(kWindowClassName, GetModuleHandle(NULL));
}

static void SetupPixelFormat(HDC hDC)
{
    int pixelFormat;

    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),  // size
        1,                          // version
        PFD_SUPPORT_OPENGL |        // OpenGL window
        PFD_DRAW_TO_WINDOW |        // render to window
        PFD_DOUBLEBUFFER,           // support double-buffering
        PFD_TYPE_RGBA,              // color type
        32,                         // preferred color depth
        0, 0, 0, 0, 0, 0,           // color bits (ignored)
        0,                          // no alpha buffer
        0,                          // alpha bits (ignored)
        0,                          // no accumulation buffer
        0, 0, 0, 0,                 // accum bits (ignored)
        24,                         // depth buffer
        8,                          // no stencil buffer
        0,                          // no auxiliary buffers
        PFD_MAIN_PLANE,             // main layer
        0,                          // reserved
        0, 0, 0,                    // no layer, visible, damage masks
    };

    pixelFormat = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, pixelFormat, &pfd);
}

#ifdef HORDE3D_GL

Device::Device(HWND hWnd): _hWnd(hWnd)
{
    HDC hDC = GetDC(hWnd);
    SetupPixelFormat(hDC);
    _hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, _hRC);
}

Device::~Device()
{
    HDC hDC = GetDC(_hWnd);
    if (hDC != NULL && _hRC != NULL)
    {
        // deselect rendering context and delete it
        wglMakeCurrent(hDC, NULL);
        wglDeleteContext(_hRC);
    }
}

void Device::SwapBuffers()
{
    HDC hDC = GetDC(_hWnd);
	::SwapBuffers(hDC); 
}

#elif HORDE3D_D3D11
Device::Device(HWND hWnd)
{
	HRESULT hr;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
//		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
	};
	UINT numDriverTypes = ARRAYSIZE( driverTypes );

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = winWidth;
	sd.BufferDesc.Height = winHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	ID3D11DeviceContext*     immediateContext;
	ID3D11RenderTargetView*  renderTargetView;
	ID3D11Texture2D*         depthStencil;
	ID3D11DepthStencilView*  depthStencilView;

	D3D_FEATURE_LEVEL        featureLevel;

	for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
	{
		hr = D3D11CreateDeviceAndSwapChain( NULL, driverTypes[driverTypeIndex], NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &_swapChain, &_d3dDevice, &featureLevel, &immediateContext );
		if( SUCCEEDED( hr ) )
			break;
	}
	if ( FAILED( hr ) ) return;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = _swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
	if ( FAILED( hr ) )  exit(-1);

	hr = _d3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &renderTargetView );
	pBackBuffer->Release();
	if ( FAILED( hr ) ) return;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory( &descDepth, sizeof(descDepth) );
	descDepth.Width = winWidth;
	descDepth.Height = winHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = _d3dDevice->CreateTexture2D( &descDepth, NULL, &depthStencil );
	if ( FAILED( hr ) ) return;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory( &descDSV, sizeof(descDSV) );
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = _d3dDevice->CreateDepthStencilView( depthStencil, &descDSV, &depthStencilView );
	if ( FAILED( hr ) ) return;

	immediateContext->OMSetRenderTargets( 1, &renderTargetView, depthStencilView );

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)winWidth;
	vp.Height = (FLOAT)winHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	immediateContext->RSSetViewports( 1, &vp ); 
}

Device::~Device()
{
	if (_d3dDevice!=NULL)
		_d3dDevice->Release();
}

void Device::SwapBuffers()
{
	if (_swapChain)
		_swapChain->Present(0,0);
}
#endif

static LRESULT CALLBACK _WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL bProcessed = FALSE;
	static BOOL bMouseCapture = FALSE;

    switch (message)
    {
    case WM_LBUTTONDOWN:
        {
            POINT point = {(short)LOWORD(lParam), (short)HIWORD(lParam)};
			RECT rc;
			GetClientRect(hWnd,&rc);
			if(PtInRect(&rc,point))
			{
				SetCapture(hWnd);
				sfwEvent e;
				e.type = sfwEventType::PointerDown;
				e.pointer.x = (float)point.x;
				e.pointer.y = (float)(winHeight - point.y);
				sfwPushEvent(&e);
			}
        }
        break;

    case WM_MOUSEMOVE:
        if (MK_LBUTTON == wParam)
        {
			if (GetCapture()==hWnd)
			{
				POINT point = {(short)LOWORD(lParam), (short)HIWORD(lParam)};
				sfwEvent e;
				e.type = sfwEventType::PointerMoved;
				e.pointer.x = (float)point.x;
				e.pointer.y = (float)(winHeight - point.y);
				sfwPushEvent(&e);
			}
        }
        break;

    case WM_LBUTTONUP:
		{
            POINT point = {(short)LOWORD(lParam), (short)HIWORD(lParam)};
			sfwEvent e;
			e.type = sfwEventType::PointerUp;
			e.pointer.x = (float)point.x;
			e.pointer.y = (float)(winHeight - point.y);
			sfwPushEvent(&e);
            ReleaseCapture(); 
		}
        break;
    case WM_ACTIVATE:
		{
			sfwEvent e;
			e.type = (wParam==WA_INACTIVE) ? sfwEventType::LostFocus : sfwEventType::GainedFocus;
			sfwPushEvent(&e);
		}
        break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		{
			sfwEvent e;
			e.type = sfwEventType::KeyPressed;
			e.key = VirtualKeyCodeToSfwKeyCode(wParam, lParam);
			sfwPushEvent(&e);
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		{
			sfwEvent e;
			e.type = sfwEventType::KeyReleased;
			e.key = VirtualKeyCodeToSfwKeyCode(wParam, lParam);
			sfwPushEvent(&e);
		}
		break;
    case WM_PAINT:
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

static sfwKey::Code VirtualKeyCodeToSfwKeyCode(WPARAM VirtualKey, LPARAM Flags)
{
    switch (VirtualKey)
    {
        case VK_SHIFT :
        {
            static UINT LShift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
            UINT scancode = (Flags & (0xFF << 16)) >> 16;
            return scancode == LShift ? sfwKey::LShift : sfwKey::RShift;
        }
        case VK_MENU : return (HIWORD(Flags) & KF_EXTENDED) ? sfwKey::RAlt : sfwKey::LAlt;
        case VK_CONTROL : return (HIWORD(Flags) & KF_EXTENDED) ? sfwKey::RControl : sfwKey::LControl;

        // Other keys are reported properly
        case VK_LWIN :       return sfwKey::LSystem;
        case VK_RWIN :       return sfwKey::RSystem;
        case VK_APPS :       return sfwKey::Menu;
        case VK_OEM_5 :      return sfwKey::BackSlash;
        case VK_ESCAPE :     return sfwKey::Escape;
        case VK_RETURN :     return sfwKey::Return;
        case VK_TAB :        return sfwKey::Tab;
        case VK_PRIOR :      return sfwKey::PageUp;
        case VK_NEXT :       return sfwKey::PageDown;
        case VK_END :        return sfwKey::End;
        case VK_HOME :       return sfwKey::Home;
        case VK_INSERT :     return sfwKey::Insert;
        case VK_DELETE :     return sfwKey::Delete;
        case VK_PAUSE :      return sfwKey::Pause;
        case VK_F1 :         return sfwKey::F1;
        case VK_F2 :         return sfwKey::F2;
        case VK_F3 :         return sfwKey::F3;
        case VK_F4 :         return sfwKey::F4;
        case VK_F5 :         return sfwKey::F5;
        case VK_F6 :         return sfwKey::F6;
        case VK_F7 :         return sfwKey::F7;
        case VK_F8 :         return sfwKey::F8;
        case VK_F9 :         return sfwKey::F9;
        case VK_F10 :        return sfwKey::F10;
        case VK_F11 :        return sfwKey::F11;
        case VK_F12 :        return sfwKey::F12;
        case VK_F13 :        return sfwKey::F13;
        case VK_F14 :        return sfwKey::F14;
        case VK_F15 :        return sfwKey::F15;
        case VK_LEFT :       return sfwKey::Left;
        case VK_RIGHT :      return sfwKey::Right;
        case VK_UP :         return sfwKey::Up;
        case VK_DOWN :       return sfwKey::Down;
        case VK_NUMPAD0 :    return sfwKey::Numpad0;
        case VK_NUMPAD1 :    return sfwKey::Numpad1;
        case VK_NUMPAD2 :    return sfwKey::Numpad2;
        case VK_NUMPAD3 :    return sfwKey::Numpad3;
        case VK_NUMPAD4 :    return sfwKey::Numpad4;
        case VK_NUMPAD5 :    return sfwKey::Numpad5;
        case VK_NUMPAD6 :    return sfwKey::Numpad6;
        case VK_NUMPAD7 :    return sfwKey::Numpad7;
        case VK_NUMPAD8 :    return sfwKey::Numpad8;
        case VK_NUMPAD9 :    return sfwKey::Numpad9;

		default:
        {
            // Convert to printable character (ISO-8859-1 or Unicode)
            VirtualKey = MapVirtualKey( (UINT) VirtualKey, 2 ) & 0x0000FFFF;

            VirtualKey = (WPARAM) CharUpperA( (LPSTR) VirtualKey );

            // Valid ISO-8859-1 character?
            if( (VirtualKey >=  32 && VirtualKey <= 126) ||
                (VirtualKey >= 160 && VirtualKey <= 255) )
				return (sfwKey::Code) VirtualKey;
		}
    }

    return sfwKey::Code(0);
}
