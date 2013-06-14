#pragma once

#include "stdafx.h"
#include "input_control.h"
#include "resource.h"
#include "game_data.h"
#include "creature.h"

#include "camera.h"
#include "engine.h"
#include "window_messages.h"
#include "eventreceiver.h"


#define MAX_LOADSTRING 100

using std::vector;
using std::map;
using std::string;

class window_application {
public:
	window_application();
	~window_application();

	HINSTANCE hInst;
	BOOL InitInstance(int nCmdShow);
	map<string, HWND> app_windows;
	map<string, string> window_titles;
	map<string, HWND> window_controls;
	ATOM register_class();
	int main_loop(void);

	int client_w;
	int client_h;

	engine eng;
};

extern window_application app;
