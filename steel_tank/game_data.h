#pragma once

#define GAME_DB "gamedata.db"

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

struct s_start_data {
	int oid;
	int x;
	int y;
};

class game_data {
public:
	map<int, int> type_selections;

	game_data();
	~game_data();

	void load_map_from_coordinates(map_data&,int,int,int,int);
	void cull_map(map_data&,int,int,int,int);

	void load_sprite_classes(map<string, sprite_class> &sprite_classes, irr::video::IVideoDriver *driver);
	vector<boost::shared_ptr<creature> > players;
	sqlite3 *gamedb;

	void find_start_positions(void);
	void make_players(void);
	map<int,boost::shared_ptr<map_object> > loaded_objects;

	vector<s_start_data> start_data;
};
