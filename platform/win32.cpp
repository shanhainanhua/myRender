﻿#include "./win32.h"

#include <cassert>
#include <cstdio>

window_t* window = NULL;

static LRESULT CALLBACK msg_callback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		window->is_close = 1;
		break;
	case WM_KEYDOWN:
		window->keys[wParam & 511] = 1;
		break;
	case WM_KEYUP:
		window->keys[wParam & 511] = 0;
		break;
	case WM_LBUTTONDOWN:
		window->mouse_info.orbit_pos = get_mouse_pos();
		window->buttons[0] = 1; break;
	case WM_LBUTTONUP:
		window->buttons[0] = 0;
		break;
	case WM_RBUTTONDOWN:
		window->mouse_info.fv_pos = get_mouse_pos();
		window->buttons[1] = 1;
		break;
	case WM_RBUTTONUP:
		window->buttons[1] = 0;
		break;
	case WM_MOUSEWHEEL:
		window->mouse_info.wheel_delta = GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
		break;

	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}


/*
	UINT        style;
	WNDPROC     lpfnWndProc;
	int         cbClsExtra;
	int         cbWndExtra;
	HINSTANCE   hInstance;
	HICON       hIcon;
	HCURSOR     hCursor;
	HBRUSH      hbrBackground;
	LPCSTR      lpszMenuName;
	LPCSTR      lpszClassName;
*/
static void register_window_class()
{
	ATOM atom;
	//初始化结构体
	WNDCLASS wc;
	wc.style = CS_BYTEALIGNCLIENT;							//窗口风格
	wc.lpfnWndProc = (WNDPROC)msg_callback;					//回调函数
	wc.cbClsExtra = 0;										//紧跟在窗口类尾部的一块额外空间，不用则设为0
	wc.cbWndExtra = 0;										//紧跟在窗口实例尾部的一块额外空间，不用则设为0
	wc.hInstance = GetModuleHandle(NULL);					//当前实列句柄
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);				//任务栏图标
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);				//光标样式
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);	//背景样式
	wc.lpszMenuName = NULL;									//菜单
	wc.lpszClassName = L"MyRender";					//窗口类的名字

	atom = RegisterClass(&wc); //注册窗口类
	assert(atom != 0);
}

/*
		DWORD      biSize;
		LONG       biWidth;
		LONG       biHeight;
		WORD       biPlanes;
		WORD       biBitCount;
		DWORD      biCompression;
		DWORD      biSizeImage;
		LONG       biXPelsPerMeter;
		LONG       biYPelsPerMeter;
		DWORD      biClrUsed;
		DWORD      biClrImportant;
*/
static void init_bm_header(BITMAPINFOHEADER& bi, int width, int height)
{
	memset(&bi, 0, sizeof(BITMAPINFOHEADER));
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height;   //从上到下
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = width * height * 4;
}

int window_init(int width, int height, const char* title)
{
	window = (window_t*)malloc(sizeof(window_t));
	memset(window, 0, sizeof(window_t));
	window->is_close = 0;

	RECT rect = { 0, 0, width, height }; //一个矩形范围，左上右下
	int wx, wy, sx, sy;
	LPVOID ptr; //void *
	HDC hDC;    
	BITMAPINFOHEADER bi;

	//注册窗口类
	register_window_class();

	//创建窗口

	window->h_window = CreateWindowA("MyRender", title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);
	assert(window->h_window != NULL);

	//初始化位图头格式
	init_bm_header(bi, width, height);

	//获得兼容性DC
	hDC = GetDC(window->h_window);
	window->mem_dc = CreateCompatibleDC(hDC);
	ReleaseDC(window->h_window, hDC);

	//创建位图
	window->bm_dib = CreateDIBSection(window->mem_dc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, &ptr, 0, 0); //创建设备无关句柄
	assert(window->bm_dib != NULL);

	window->bm_old = (HBITMAP)SelectObject(window->mem_dc, window->bm_dib);//把新创建的位图句柄写入mem_dc
	window->window_fb = (unsigned char*)ptr;

	window->width = width;
	window->height = height;


	AdjustWindowRect(&rect, GetWindowLong(window->h_window, GWL_STYLE), 0);//调整窗口大小
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2; // GetSystemMetrics(SM_CXSCREEN) 获取屏幕分辨率
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2; // 算出中心位置
	if (sy < 0) sy = 0;

	SetWindowPos(window->h_window, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(window->h_window);
	ShowWindow(window->h_window, SW_NORMAL);

	//消息分派
	msg_dispatch();

	//初始化keys window_fp 为0
	memset(window->window_fb, 0, width * height * 4);
	memset(window->keys, 0, sizeof(char) * 512);
	return 0;
}

int window_destroy()
{
	if (window->mem_dc)
	{
		if (window->bm_old)
		{
			SelectObject(window->mem_dc, window->bm_old); // 写入原来的bitmap才能释放DC
			window->bm_old = NULL;
		}
		DeleteDC(window->mem_dc);
		window->mem_dc = NULL;
	}
	if (window->bm_dib)
	{
		DeleteObject(window->bm_dib);
		window->bm_dib = NULL;
	}
	if (window->h_window)
	{
		CloseWindow(window->h_window);
		window->h_window = NULL;
	}

	free(window);
	return 0;
}

void msg_dispatch()
{
	MSG msg;
	while (1)
	{
		
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break; 
		if (!GetMessage(&msg, NULL, 0, 0)) break;

		TranslateMessage(&msg);	
		DispatchMessage(&msg); 
	}
}

static void window_display()
{
	LOGFONT logfont; 
	ZeroMemory(&logfont, sizeof(LOGFONT));
	logfont.lfCharSet = ANSI_CHARSET;
	logfont.lfHeight = 20; 
	HFONT hFont = CreateFontIndirect(&logfont);

	HDC hDC = GetDC(window->h_window);

	SelectObject(window->mem_dc, hFont);
	SetTextColor(window->mem_dc, RGB(190, 190, 190));
	SetBkColor(window->mem_dc, RGB(80, 80, 80));
	TextOut(window->mem_dc, 20, 20,
		L"control:hold left buttion to rotate, right button to pan",
		strlen("Control:hold left buttion to rotate, right button to pan"));


	BitBlt(hDC, 0, 0, window->width, window->height, window->mem_dc, 0, 0, SRCCOPY);
	ReleaseDC(window->h_window, hDC);

}

void window_draw(unsigned char* framebuffer)
{
	int i, j;
	for (int i = 0; i < window->height; i++)
	{
		for (int j = 0; j < window->width; j++)
		{
			//RGBA四个颜色通道，每个通道都是一个字节，需要将索引乘以4才能正确地指向帧缓冲区中地下一个像素
			int index = (i * window->width + j) * 4;
			window->window_fb[index] = framebuffer[index + 2];
			window->window_fb[index + 1] = framebuffer[index + 1];
			window->window_fb[index + 2] = framebuffer[index];
		}
	}
	window_display();
}

vec2 get_mouse_pos()
{
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(window->h_window, &point); //屏幕空间转到窗口空间
	return vec2((float)point.x, (float)point.y);
}

/* misc platform functions */
static double get_native_time(void) {
	static double period = -1;
	LARGE_INTEGER counter;
	if (period < 0) {
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		period = 1 / (double)frequency.QuadPart;
	}
	QueryPerformanceCounter(&counter);
	return counter.QuadPart * period;
}

float platform_get_time(void) {
	static double initial = -1;
	if (initial < 0) {
		initial = get_native_time();
	}
	return (float)(get_native_time() - initial);
}