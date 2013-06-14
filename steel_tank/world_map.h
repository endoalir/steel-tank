#pragma once

using std::string;
using std::vector;

class bullet;

class map_object {
public:
	map_object();
	virtual ~map_object();

	string object_name;
	string object_class;

	vector<string> sprites;

	vector<int> rotations;
	int actual_rotation;

	string sprite_class;

	int explosion_step;

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

	int object_id;

	bool is_enemy;

	int use_count;

	int last_botomatic_step;
	int last_projectile_step;

};


class map_data {
public:
	map_data();
	~map_data();

	int c_x;
	int c_y;

	int loaded_x;
	int loaded_y;
	int loaded_x1;
	int loaded_y1;

	map<int,boost::shared_ptr<map_object> > map_objects;
};
