#include <Windows.h>

#include "game.h"
#include "sound.h"

static BITMAPINFO* bmi;
static game_t game;
static uint32 input;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			RECT rc;

			GetClientRect(hwnd, &rc);

			uint32 w = rc.right - rc.left;
			uint32 h = rc.bottom - rc.top;

			StretchDIBits(hdc, 0, 0, w, h, 0, 0, GPU_SCR_W, GPU_SCR_H, game_get_framebuffer(&game), bmi, DIB_RGB_COLORS, SRCCOPY);
			EndPaint(hwnd, &ps);

			break;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			switch (wParam)
			{
				case VK_UP:
					input |= INPUT_UP;
					break;
				case VK_DOWN:
					input |= INPUT_DOWN;
					break;
				case VK_LEFT:
					input |= INPUT_LEFT;
					break;
				case VK_RIGHT:
					input |= INPUT_RIGHT;
					break;
				case 'Z':
					input |= INPUT_CONFIRM;
					break;
				case 'X':
					input |= INPUT_CANCEL;
					break;
			}

			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			switch (wParam)
			{
				case VK_UP:
					input &= ~INPUT_UP;
					break;
				case VK_DOWN:
					input &= ~INPUT_DOWN;
					break;
				case VK_LEFT:
					input &= ~INPUT_LEFT;
					break;
				case VK_RIGHT:
					input &= ~INPUT_RIGHT;
					break;
				case 'Z':
					input &= ~INPUT_CONFIRM;
					break;
				case 'X':
					input &= ~INPUT_CANCEL;
					break;
			}

			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);

			break;
		}
		default:
		{
			return DefWindowProcA(hwnd, msg, wParam, lParam);
		}
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	const char* path = "";

	if (lpCmdLine && *lpCmdLine)
	{
		path = lpCmdLine;
	}

	if (!game_init(&game, path))
	{
		MessageBoxA(NULL, "Game files missing", NULL, MB_OK);

		return EXIT_FAILURE;
	}

	WNDCLASSA wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = "TigerTiger";

	RegisterClassA(&wc);

	RECT rect = { 0, 0, GPU_SCR_W * 2, GPU_SCR_H * 2 };

	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX), FALSE, 0);

	uint32 window_w = rect.right - rect.left;
	uint32 window_h = rect.bottom - rect.top;

	RECT screen;

	SystemParametersInfoA(SPI_GETWORKAREA, 0, &screen, 0);

	uint32 screen_w = screen.right - screen.left;
	uint32 screen_h = screen.bottom - screen.top;
	uint32 window_x = screen.left + (screen_w - window_w) / 2;
	uint32 window_y = screen.top + (screen_h - window_h) / 2;

	HWND hwnd = CreateWindowExA(0, "TigerTiger", "Tiger! Tiger!", WS_OVERLAPPEDWINDOW | WS_VISIBLE, window_x, window_y, window_w, window_h, 0, 0, hInstance, 0);

	bmi = calloc(1, sizeof(BITMAPINFO) + 3 * sizeof(DWORD));
	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi->bmiHeader.biWidth = GPU_SCR_W;
	bmi->bmiHeader.biHeight = -GPU_SCR_H;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biBitCount = 32;
	bmi->bmiHeader.biCompression = BI_BITFIELDS;
	bmi->bmiColors[0].rgbBlue = UINT8_MAX;
	bmi->bmiColors[1].rgbGreen = UINT8_MAX;
	bmi->bmiColors[2].rgbRed = UINT8_MAX;

	LARGE_INTEGER freq;
	LARGE_INTEGER time;

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&time);

	double freq_rcp = 1.0 / (double)freq.QuadPart;
	double frame_time = 1.0 / 30.0;
	double elapsed = 0.0;

	MSG msg;
	uint8 running = 1;

	while (running)
	{
		while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				running = 0;

				break;
			}

			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		LARGE_INTEGER now;

		QueryPerformanceCounter(&now);

		elapsed += (now.QuadPart - time.QuadPart) * freq_rcp;
		time = now;

		if (elapsed >= frame_time)
		{
			elapsed = MIN(elapsed - frame_time, frame_time);

			if (!game_update(&game, input))
			{
				break;
			}

			InvalidateRect(hwnd, NULL, FALSE);
		}

		Sleep(0);
	}

	free(bmi);

	return EXIT_SUCCESS;
}
