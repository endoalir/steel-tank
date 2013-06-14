#include "stdafx.h"
#include "map_editor.h"
#include <ctime>

window_application app;

TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];

window_application::window_application(){}
window_application::~window_application(){}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow){
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	HACCEL hAccelTable;

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MAP_EDITOR, szWindowClass, MAX_LOADSTRING);

	app.register_class();

	if(!app.InitInstance(nCmdShow)){
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MAP_EDITOR));
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
	wcex.hIcon			= LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAP_EDITOR));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MAP_EDITOR);
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
	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 800, 600, NULL, NULL, hInst, NULL);
	if (!hWnd){ return false; }
	app_windows["main"] = hWnd;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	WINDOWINFO wi;
	GetWindowInfo(hWnd, &wi);
	
	client_w = wi.rcClient.right-wi.rcClient.left;
	client_h = wi.rcClient.bottom-wi.rcClient.top;

	eng.client_width = 800;
	eng.client_height = 600;

	HWND hIrrlichtWindow = CreateWindow(_T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
	window_controls["mainirr"] = hIrrlichtWindow;

	eng.load();

	return true;
}
