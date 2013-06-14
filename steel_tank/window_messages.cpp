#include "stdafx.h"
#include "steel_tank.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	shared_ptr<window> this_window;

	/*if(WM_CREATE == message || WM_GETMINMAXINFO == message || WM_NCCREATE == message){
		return DefWindowProc(hWnd, message, wParam, lParam);
	}*/

	bool found_win = false;
	if(WM_DESTROY != message){
		vector<shared_ptr<window> >::iterator winitr;
		for(winitr = app.app_windows.begin(); winitr != app.app_windows.end(); winitr++){
			shared_ptr<window> which_win(*winitr);
			if(which_win->this_window == hWnd){
				this_window = which_win;
				found_win = true;
				break;
			}
			/*if(which_win->control == hWnd){
				this_window = which_win;
				found_win = true;
				break;
			}*/
		}
	}

	int cli_w = 0, cli_h = 0;

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
	bool handled = false;

	shared_ptr<window> next_window;

	switch (message){
	case WM_RBUTTONDOWN:
		app.eng.rbutton = true;
	break;
	case WM_LBUTTONDOWN:
		app.eng.lbutton = true;
	case WM_MOUSEMOVE:
		if(found_win){
			xPos = LOWORD(lParam); 
			yPos = HIWORD(lParam);

			proportionx = (float)((float)xPos / (float)this_window->client_w);
			proportiony = (float)((float)yPos / (float)this_window->client_h);

			relx = (int)(proportionx * this_window->client_w);
			rely = (int)(proportiony * this_window->client_h);

			app.eng.c_x = relx;
			app.eng.c_y = rely;
			app.eng.inp.mouse_xpos = relx;
			app.eng.inp.mouse_ypos = rely;
		}
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

		handled = false;

		switch (wmId){
		case IDM_ABOUT:
			DialogBox(app.hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			handled = true;
			break;

		case IDM_CHEATS_INVINCIBLE:
			app.eng.invincible_mode = !app.eng.invincible_mode;
			handled = true;
			break;

		case IDM_CHEATS_SUPER_PLAYER:
			app.eng.super_player = !app.eng.super_player;
			handled = true;
			break;

		case IDM_CHEATS_CLIPLESS:
			app.eng.clipless = !app.eng.clipless;
			handled = true;
			break;

		case IDM_CHEATS_SUPERFAST:
			app.eng.superfast = !app.eng.superfast;
			handled = true;
			break;

		case IDM_CHEATS_EPIC_PLAYER:
			app.eng.epicplayer = !app.eng.epicplayer;
			handled = true;
			break;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			handled = true;
			break;
		}

		if(!handled){
			map<int,int>::iterator menu_newwindow_camera_itr = app.menu_newwindow_camera.find(wmId);
			if(menu_newwindow_camera_itr != app.menu_newwindow_camera.end()){
				int cam_to_move = menu_newwindow_camera_itr->second;
				unsigned int moving_from = app.menu_newwindow_window[wmId];
				
				boost::shared_ptr<window> new_window(new window());
				new_window->cams.this_window = this_window;
				app.app_windows.push_back(new_window);
				new_window->make(SW_SHOW);


				if(moving_from < app.app_windows.size())
					app.app_windows[moving_from]->cams.move_camera(cam_to_move, new_window->window_number);
			}

			map<int,int>::iterator menu_movewindow_camera_itr = app.menu_movewindow_camera.find(wmId);
			if(menu_movewindow_camera_itr != app.menu_movewindow_camera.end()){
				int cam_to_move = menu_movewindow_camera_itr->second;
				unsigned int moving_from = app.menu_movewindow_fromwindow[wmId];
				int moving_to = app.menu_movewindow_towindow[wmId];

				if(moving_from < app.app_windows.size())
					app.app_windows[moving_from]->cams.move_camera(cam_to_move, moving_to);
			}

			app.reset_cameras();
		}


		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		EndPaint(hWnd, &ps);
		break;
	case WM_CLOSE:
		if(1 == app.app_windows.size()){
			PostQuitMessage(0);
			break;
		}

		if(this_window){

			for(auto winitr = app.app_windows.begin(); winitr != app.app_windows.end(); winitr++){
				next_window = *winitr;
				if(next_window != this_window)
					break;
			}

			for(unsigned int i = 0; i < this_window->cams.camera_count; i++){
				this_window->cams.move_camera(0, next_window->window_number);
			}

			for(auto winitr = app.app_windows.begin(); winitr != app.app_windows.end();){
				if(*winitr == this_window){
					app.app_windows.erase(winitr);
					break;
				}
				winitr++;
			}

			app.reset_cameras();
		}

		return DefWindowProc(hWnd, message, wParam, lParam);

		break;
	case WM_SIZE:
		if(found_win){
			cli_w = LOWORD(lParam);
			cli_h = HIWORD(lParam);

			this_window->client_w = cli_w;
			this_window->client_h = cli_h;

			app.eng.repos_cams();

			SetWindowPos(this_window->control, 0, 0, 0, cli_w, cli_h, 0);
			this_window->driver->OnResize(irr::core::dimension2d<irr::u32>(cli_w, cli_h));
		}
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
