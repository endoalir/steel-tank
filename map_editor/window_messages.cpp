#include "stdafx.h"
#include "map_editor.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	int xPos = 0;
	int yPos = 0;

	int cwidth = 0;
	int cheight = 0;
	int relx = 0;
	int rely = 0;
	float proportionx;
	float proportiony;		
	int speed = 5;

	switch (message){
	case WM_RBUTTONDOWN:
		app.eng.rbutton = true;
	break;
	case WM_LBUTTONDOWN:
		app.eng.lbutton = true;
	case WM_MOUSEMOVE:
		xPos = LOWORD(lParam); 
		yPos = HIWORD(lParam);

		proportionx = (float)((float)xPos / (float)app.client_w);
		proportiony = (float)((float)yPos / (float)app.client_h);

		relx = (int)(proportionx * app.eng.client_width);
		rely = (int)(proportiony * app.eng.client_height);

		app.eng.c_x = relx;
		app.eng.c_y = rely;
		app.eng.inp.mouse_xpos = relx;
		app.eng.inp.mouse_ypos = rely;
	break;

	case WM_KEYDOWN:
		app.eng.inp.keys_down[wParam] = true;
	break;

	case WM_KEYUP:
		app.eng.inp.keys_down[wParam] = false;
	break;

	case WM_LBUTTONUP:
		app.eng.lbutton = false;
	break;

	case WM_RBUTTONUP:
		app.eng.rbutton = false;
	break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		switch (wmId){
		case IDM_SAVE:
			app.eng.gd.write_map(app.eng.md);
			break;
		case IDM_ABOUT:
			DialogBox(app.hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		app.client_w = LOWORD(lParam);
		app.client_h = HIWORD(lParam);

		app.eng.client_width = app.client_w;
		app.eng.client_height = app.client_h;

		if(app.eng.cam.size() > 0){
			app.eng.cam[0].w = app.client_w - 186;
			app.eng.cam[0].h = app.client_h;
		}

		app.eng.driver->OnResize(irr::core::dimension2d<irr::u32>(app.client_w, app.client_h));

		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	UNREFERENCED_PARAMETER(lParam);
	switch (message){
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
