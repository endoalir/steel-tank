#pragma once

#include "stdafx.h"

using std::vector;
using std::map;
using std::string;

#include "sound.h"
#include "input_control.h"
#include "resource.h"
#include "world_map.h"
#include "game_data.h"
#include "creature.h"

#include "camera.h"
#include "camera_set.h"
#include "engine.h"
#include "window_messages.h"
#include "eventreceiver.h"

#include "game_sequence.h"


#define MAX_LOADSTRING 100

#define LOAD_REGION 500

class window {
public:
	window();
	~window();

	HWND this_window;
	HWND control;

	int client_w;
	int client_h;

	int window_number;

	irr::video::IVideoDriver *driver;
	irr::IrrlichtDevice *device;
	irr::scene::ISceneManager *smgr;
	irr::video::SExposedVideoData videodata;

	int number_of_explosion_steps;

	map<string, sprite_class> sprite_classes;
	irr::video::ITexture *bulletimg;
	irr::video::ITexture *flame_bullet;
	irr::video::ITexture *rock_bullet;
	irr::video::ITexture *shock_bullet;
	irr::video::ITexture *hydro_bullet;
	vector<irr::video::ITexture*> explosion_steps;
	irr::video::ITexture *terrain;


	camera_set cams;

	bool make(int cmdshow);

	void close(void);

	void populate_camera_menu(void);

};


class window_application {
public:
	window_application();
	~window_application();

	HINSTANCE hInst;
	BOOL InitInstance(int nCmdShow);
	vector<boost::shared_ptr<window> > app_windows;

	void renumber_windows(void);
	void reset_cameras(void);

	ATOM register_class();
	int main_loop(void);

	engine eng;

	map<int, int> menu_movewindow_camera;
	map<int, int> menu_movewindow_fromwindow;
	map<int, int> menu_movewindow_towindow;

	map<int, int> menu_newwindow_camera;
	map<int, int> menu_newwindow_window;

	int cam_mid;

};

extern window_application app;
