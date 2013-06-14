#include "stdafx.h"
#include "steel_tank.h"
#include <ctime>


using std::wstring;

window_application app;

TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];

window_application::window_application(){
	cam_mid = IDM_CAM_STARTS;
}
window_application::~window_application(){}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow){
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	app.hInst = hInstance;

	HACCEL hAccelTable;

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_STEEL_TANK, szWindowClass, MAX_LOADSTRING);

	app.register_class();

	if(!app.InitInstance(nCmdShow)){
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_STEEL_TANK));
	return app.main_loop();
}

ATOM window_application::register_class(){
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= LoadIcon(hInst, MAKEINTRESOURCE(IDI_STEEL_TANK));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_STEEL_TANK);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

int desired_framerate = 150;
int set_sleeprate = 0;

int window_application::main_loop(){
	MSG msg;

	::clock_t start_clock = clock();
	::clock_t ct = clock();
	long counter = 0;
	long last_counter = 0;

	for(;;){
		++counter;

		if((clock() - ct) / CLOCKS_PER_SEC > 2){
			ct = clock();
			last_counter = counter;
			counter = 0;

			if(set_sleeprate == 0){
				if(last_counter < (desired_framerate)){
					set_sleeprate = -1;
				}else{
					double desired_time_spent_per_frame = ceil(2000.0 / (double)desired_framerate);
					double actual_time_spent_per_frame = floor(2000.0 / (double)last_counter);

					set_sleeprate = (int)ceil((desired_time_spent_per_frame - actual_time_spent_per_frame));
#ifdef _DEBUG
					set_sleeprate /= 2;
#endif

				}
			}

		}

		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if(msg.message == WM_QUIT) break;
		}

		if(set_sleeprate > 0)
			Sleep(set_sleeprate);

		eng.runner(last_counter);
	}

	return (int) msg.wParam;
}

BOOL window_application::InitInstance(int nCmdShow){
	boost::shared_ptr<window> this_window(new window());
	this_window->cams.this_window = this_window;
	app_windows.push_back(this_window);

	if(!this_window->make(nCmdShow))
		return false;

	eng.load();
	return true;
}

void window_application::renumber_windows(void){
	int w_num = 0;
	for(auto witr = app_windows.begin(); witr != app_windows.end(); witr++)
		(*witr)->window_number = w_num++;
}

void window_application::reset_cameras(void){
	app.menu_movewindow_camera.clear();
	app.menu_movewindow_fromwindow.clear();
	app.menu_movewindow_towindow.clear();
	app.menu_newwindow_camera.clear();
	app.menu_newwindow_window.clear();
	app.cam_mid = IDM_CAM_STARTS;

	app.renumber_windows();
	for(auto appw = app.app_windows.begin(); appw != app.app_windows.end(); appw++)
		(*appw)->cams.renumber_cameras();
	for(auto appw = app.app_windows.begin(); appw != app.app_windows.end(); appw++)
		(*appw)->populate_camera_menu();
	for(auto appw = app.app_windows.begin(); appw != app.app_windows.end(); appw++)
		(*appw)->cams.reposition();
}

window::window(){
	this_window = 0;
	control = 0;
	client_w = 0, client_h = 0;
	window_number = app.app_windows.size();
}

window::~window(){
	CloseHandle(control);
	CloseHandle(this_window);

	if(smgr){
		smgr->clear();
		smgr->drop();
	}

	if(driver){
		driver->removeAllTextures();
		driver->drop();
	}
	if(device){
		device->closeDevice();
		device->drop();
	}

	cams.clear();
}

void window::populate_camera_menu(void){
	HMENU menubar = GetMenu(this_window);
	if(!menubar){ ::OutputDebugString(_T("No menubar")); return; }

	const unsigned int THE_NUMBER_TWO = 2; // TWO!!!!1!
	// TODO: Make NUMERIC CONSTANT PROVIDER FACTORIEZ

	HMENU cameramenu = GetSubMenu(menubar, THE_NUMBER_TWO);
	// This is how you make a menu at 6am

	while(RemoveMenu(cameramenu, 0, MF_BYPOSITION));

	int &m_id = app.cam_mid;

	map<int,int> &menu_movewindow_camera = app.menu_movewindow_camera;
	map<int,int> &menu_movewindow_fromwindow = app.menu_movewindow_fromwindow;
	map<int,int> &menu_movewindow_towindow = app.menu_movewindow_towindow;

	map<int,int> &menu_newwindow_camera = app.menu_newwindow_camera;
	map<int,int> &menu_newwindow_window = app.menu_newwindow_window;

	for(auto t_cam = cams.cameras.begin(); t_cam != cams.cameras.end(); t_cam++){
		wchar_t cam_menutext[MAX_LOADSTRING];

		HMENU cam_submenu = CreateMenu();

		int submenu_number = 0;

		StringCbPrintf(cam_menutext, sizeof(cam_menutext),  _T("To New Window"));
		MENUITEMINFO mif_tnw;
		mif_tnw.cbSize = sizeof(MENUITEMINFO);
		mif_tnw.fMask = MIIM_STATE|MIIM_ID|MIIM_TYPE;
		mif_tnw.fType = MFT_STRING;
		mif_tnw.fState = MFS_ENABLED;
		mif_tnw.wID = m_id;
		mif_tnw.hSubMenu = NULL;
		mif_tnw.hbmpChecked = NULL;
		mif_tnw.hbmpUnchecked = NULL;
		mif_tnw.dwItemData = 0;
		mif_tnw.dwTypeData = cam_menutext;
		mif_tnw.cch = wcslen(cam_menutext);
		mif_tnw.hbmpItem = NULL;

		if(!InsertMenuItem(cam_submenu, submenu_number++, true, &mif_tnw))
			break;

		menu_newwindow_camera[m_id] = (*t_cam)->cam_num;
		menu_newwindow_window[m_id] = (*t_cam)->this_window->window_number;
		m_id++;

		int to_window_number = 0;
		for(auto t_win = app.app_windows.begin(); t_win != app.app_windows.end(); t_win++){
			shared_ptr<window> this_window(*t_win);
			StringCbPrintf(cam_menutext, sizeof(cam_menutext),  _T("To Window %u"), to_window_number);

			MENUITEMINFO mif;
			mif.cbSize = sizeof(MENUITEMINFO);
			mif.fMask = MIIM_STATE|MIIM_ID|MIIM_TYPE;
			mif.fType = MFT_STRING;
			mif.fState = MFS_ENABLED;
			mif.wID = m_id;
			mif.hSubMenu = NULL;
			mif.hbmpChecked = NULL;
			mif.hbmpUnchecked = NULL;
			mif.dwItemData = 0;
			mif.dwTypeData = cam_menutext;
			mif.cch = wcslen(cam_menutext);
			mif.hbmpItem = NULL;
			if(!InsertMenuItem(cam_submenu, submenu_number++, true, &mif))
				break;

			menu_movewindow_camera[m_id] = (*t_cam)->cam_num;
			menu_movewindow_fromwindow[m_id] = (*t_cam)->this_window->window_number;
			menu_movewindow_towindow[m_id] = to_window_number;

			to_window_number++;
			m_id++;
		}

		StringCbPrintf(cam_menutext, sizeof(cam_menutext),  _T("Camera %u"), (*t_cam)->cam_num+1);

		MENUITEMINFO mif;
		mif.cbSize = sizeof(MENUITEMINFO);
		mif.fMask = MIIM_STATE|MIIM_ID|MIIM_TYPE|MIIM_SUBMENU;
		mif.fType = MFT_STRING;
		mif.fState = MFS_ENABLED;
		mif.wID = m_id++;
		mif.hSubMenu = cam_submenu;
		mif.hbmpChecked = NULL;
		mif.hbmpUnchecked = NULL;
		mif.dwItemData = 0;
		mif.dwTypeData = cam_menutext;
		mif.cch = wcslen(cam_menutext);
		mif.hbmpItem = NULL;

		if(!InsertMenuItem(cameramenu, (*t_cam)->cam_num+1, true, &mif))
			break;
	}

	DrawMenuBar(this_window);
}

bool window::make(int cmdshow){
	this_window = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 800, 600, NULL, NULL, NULL, NULL);
	if(!this_window){return false;}


	ShowWindow(this_window, cmdshow);
	UpdateWindow(this_window);
	WINDOWINFO wi;
	GetWindowInfo(this_window, &wi);
	
	//client_w = wi.rcClient.right;
	//client_h = wi.rcClient.bottom;

	client_w = wi.rcClient.right-wi.rcClient.left;
	client_h = wi.rcClient.bottom-wi.rcClient.top;

	// Otherwise it doesn't fill the window. I don't understand it, but oh well
	client_w += 14;
	client_h += 54;

	control = CreateWindow(_T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 0, 0, client_w, client_h, this_window, NULL, NULL, NULL);

	irr::SIrrlichtCreationParameters param;
	param.DriverType = irr::video::EDT_DIRECT3D9;
	param.WindowId = reinterpret_cast<void*>(this_window);
	device = irr::createDeviceEx(param);
	if(!device){ return false; }
	driver = device->getVideoDriver();
	if(!driver){ return false; }

	videodata = irr::video::SExposedVideoData(control);
	smgr = device->getSceneManager();

	terrain = driver->getTexture("../stock/terrain1.jpg");
	bulletimg = driver->getTexture("../stock/bullet.png");
	flame_bullet = driver->getTexture("../stock/flame_bullet.png");
	rock_bullet = driver->getTexture("../stock/rock_bullet.png");
	shock_bullet = driver->getTexture("../stock/shock_bullet.png");
	hydro_bullet = driver->getTexture("../stock/hydro_bullet.png");


	number_of_explosion_steps = 11;
	for(int step=1; step<=number_of_explosion_steps; step++){
		explosion_steps.push_back(driver->getTexture(string("../stock/explosion/step" + boost::lexical_cast<string>(step) + ".png").data()));
	}

	app.eng.gd.load_sprite_classes(sprite_classes, driver);

	return true;
}

void window::close(void){
	SendMessage(this_window, WM_CLOSE, NULL, NULL);
}
