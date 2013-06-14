#pragma once
#include "stdafx.h"
#include "joystick.h"

using std::map;
using std::vector;

#define TYPE_VANILLA 0
#define TYPE_FLAME 1
#define TYPE_ROCK 2
#define TYPE_SHOCK 3
#define TYPE_HYDRO 4
#define TYPE_STEEL 5

class bullet {
public:
	bullet();
	~bullet();

	int x;
	int y;
	int inertia_x;
	int inertia_y;
	int counter;
	int fade_after;
	int power;

	int type;

	bool to_destroy;

	irr::video::ITexture *img;
};

class engine {
public:
	engine();
	~engine();

	int number_of_explosion_steps;
	vector<irr::video::ITexture*> explosion_steps;

	irr::video::ITexture *bulletimg;

	joystick js;

	irr::video::ITexture *flame_bullet;
	irr::video::ITexture *rock_bullet;
	irr::video::ITexture *shock_bullet;
	irr::video::ITexture *hydro_bullet;

	int number_of_players;

	vector<irr::core::rect<irr::s32> > sprite_buttons;

	int sprite_selected;

private:

	irr::IrrlichtDevice *device;
	irr::scene::ISceneManager *smgr;
	irr::video::SExposedVideoData videodata;
	irr::gui::IGUIFont *font;

	bool intitle;
	irr::video::ITexture *titlescreen;
	irr::video::ITexture *splashscreen;
	irr::video::ITexture *playerstart;
	irr::video::ITexture *player_select_screen;

	
	bool player_ingame;

	clock_t splashtimer;
	bool insplash;

	bool showfps;

	int prev_time;

	int c_response_interval;
	int c_response_ticker;

public:
	void repos_cams(void);

	irr::video::ITexture *terrain; // temporarily

	irr::video::IVideoDriver *driver;

	void runner(int i);
	void load(void);

	bool lbutton;
	bool mbutton;
	bool rbutton;

	int c_x;
	int c_y;

	int client_width;
	int client_height;

	int offs_left;
	int offs_top;

	bool player_start_sequence;
	clock_t player_start_sequence_timer;

	int camera_x;
	int camera_y;
	int map_width;
	int map_height;

	void player_fire(void);
	int player_rotation;
	time_t last_fire_time;

	game_data gd;
	map_data md;

	input_control inp;

	vector<camera> cam;

	void draw2DImage(irr::video::IVideoDriver *driver, irr::video::ITexture* texture , irr::core::rect<irr::s32> sourceRect, irr::core::position2d<irr::s32> position, irr::core::position2d<irr::s32> rotationPoint, irr::f32 rotation, irr::core::vector2df scale, bool useAlphaChannel, irr::video::SColor color, irr::core::rect<irr::s32> clipRect);
	bool projectile_step(boost::shared_ptr<creature> thecreature, bullet &thebullet);
};
