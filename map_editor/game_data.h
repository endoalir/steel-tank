#pragma once

#define GAME_DB "../steel_tank/gamedata.db"

using std::map;
using std::vector;
using std::string;

class creature;
class bullet;

class sprite_layer {
public:
	sprite_layer();
	~sprite_layer();

	vector<string> fspecs;
	vector<irr::video::ITexture*> images;
};

class sprite_class {
public:
	sprite_class();
	~sprite_class();

	string class_name;
	int w;
	int h;

	vector<sprite_layer> layers;
};

class map_object {
public:
	map_object();
	~map_object();

	string object_name;

	vector<string> sprites;

	vector<int> rotations;
	int actual_rotation;

	string sprite_class;

	int x,y,x1,y1;

	virtual void object_action(void);

	bool rotate_layer(int,int);
	bool rotate_layer_by(int,int);
	bool rotate_object(int);
	bool rotate_object_sticky_cardinal(int);
	bool rotate_object_by(int);
	bool sticky_rotate_object_by(int);

	bool to_remove;
	bool is_solid;

	bool is_within(int, int);

	vector<int> active_layer_frames;

	virtual void step_explosion(void);
	virtual void hit_by_projectile(bullet hitby);

	irr::video::ITexture *overlay_graphic;

};


class map_data {
public:
	map_data();
	~map_data();

	vector<map_object> loaded_objects;
};

class game_data {
public:
	game_data();
	~game_data();

	vector<boost::shared_ptr<map_object> > map_objects;

	void load_map_from_coordinates(map_data&,int,int,int,int);
	void write_map(map_data&);

	void load_sprite_classes(void);

	vector<boost::shared_ptr<creature> > players;
	vector<boost::shared_ptr<creature> > enemies;

	sqlite3 *gamedb;

	map<string, sprite_class> sprite_classes;
};

