#pragma once
#include "stdafx.h"
#include "joystick.h"
#include "camera_set.h"

using std::map;
using std::vector;

#define TYPE_VANILLA  0
#define TYPE_FLAME    1
#define TYPE_ROCK     2
#define TYPE_SHOCK    3
#define TYPE_HYDRO    4
#define TYPE_STEEL    5

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
};

class game_sequence;

class engine {
public:
	engine();
	~engine();

	sound snd;

	void animagic(void);

	joystick js;

	int number_of_players;

	map<int,bool> destroyed_enemies;

	bool check_collision(int x, int y, int x1, int y1, map_object *);

	// cheats
	bool invincible_mode;
	bool super_player;
	bool clipless;
	bool superfast;
	bool epicplayer;

	bool polled_js;

	map<int, shared_ptr<map_object> > shared_objects;

private:

	irr::gui::IGUIFont *font;

	bool showfps;

	int prev_time;

	int c_response_interval;
	int c_response_ticker;

	unsigned int current_sequence;
	vector<boost::shared_ptr<game_sequence> > sequences;


public:
	void repos_cams(void);
	void center_cams(void);

	int wf_step;

	void runner(int i);
	void load(void);

	bool lbutton;
	bool mbutton;
	bool rbutton;

	int c_x;
	int c_y;

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

	void draw2DImage(irr::video::IVideoDriver *driver, irr::video::ITexture* texture , irr::core::rect<irr::s32> sourceRect, irr::core::position2d<irr::s32> position, irr::core::position2d<irr::s32> rotationPoint, irr::f32 rotation, irr::core::vector2df scale, bool useAlphaChannel, irr::video::SColor color, irr::core::rect<irr::s32> clipRect);
	bool projectile_step(boost::shared_ptr<map_object>, bullet &thebullet);
};
