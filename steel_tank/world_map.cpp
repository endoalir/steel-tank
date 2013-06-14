#include "stdafx.h"
#include "steel_tank.h"


map_object::map_object() {
	object_name="(none)";
	x=0,y=0,x1=0,y1=0;
	actual_rotation = 0;
	to_remove = false;
	is_solid = true;
	overlay_graphic = 0;
	object_id = 0;
	is_enemy = false;
	use_count = 0;
	last_botomatic_step = 0;
	last_projectile_step = 0;
	explosion_step = -1;
}
map_object::~map_object() {}

bool map_object::rotate_layer(int layer, int rot){
	while(rot < 0){rot += 360;}
	while(rot >= 360){rot -= 360;}

	rotations[layer] = rot;
	return true;
}

bool map_object::rotate_layer_by(int layer, int rot){
	while(rot < 0){rot += 360;}
	while(rot >= 360){rot -= 360;}

	int new_rot = rotations[layer] + rot;

	while(new_rot < 0){new_rot += 360;}
	while(new_rot > 360){new_rot -= 360;}

	rotate_layer(layer, new_rot);
	return true;
}


bool map_object::rotate_object(int rot){
	while(rot < 0){rot += 360;}
	while(rot >= 360){rot -= 360;}

	bool first = true;
	int first_angle = 0;

	for(vector<int>::iterator itr = rotations.begin(); itr!=rotations.end(); itr++){
		// The first layer rotates to the angle specified
		// Every layer on top moves to a new angle relative to the change in the first angle

		if(first){
			first_angle = *itr;
			*itr = rot;
		}else{
			int rel = *itr - first_angle;
			int new_angle = rot + rel;
			while(new_angle < 0){new_angle += 360;}
			while(new_angle >= 360){new_angle -= 360;}
			*itr = new_angle;
		}

		first = false;
	}
	return true;
}

bool map_object::rotate_object_sticky_cardinal(int rot){
	bool intercardinals = false;
	vector<int> sticky_angles;
	sticky_angles.push_back(0);
	if(intercardinals)
		sticky_angles.push_back(45);
	sticky_angles.push_back(90);
	if(intercardinals)
		sticky_angles.push_back(135);
	sticky_angles.push_back(180);
	if(intercardinals)
		sticky_angles.push_back(225);
	sticky_angles.push_back(270);

	int newrot = rot;

	for(vector<int>::iterator itr = sticky_angles.begin(); itr!=sticky_angles.end(); itr++){
		if(rot >= (*itr)-15 && rot <= (*itr)+15){
			newrot = *itr;
		}
	}

	rotate_object(newrot);

	return true;
}

bool map_object::rotate_object_by(int rot){
	if(actual_rotation == 0){actual_rotation = rotations[0];}
	actual_rotation = actual_rotation+rot;

	while(actual_rotation < 0){actual_rotation += 360;}
	while(actual_rotation > 360){actual_rotation -= 360;}

	rotate_object(actual_rotation);
	return true;
}

bool map_object::sticky_rotate_object_by(int rot){
	if(actual_rotation == 0){actual_rotation = rotations[0];}
	actual_rotation = actual_rotation+rot;

	while(actual_rotation < 0){actual_rotation += 360;}
	while(actual_rotation > 360){actual_rotation -= 360;}

	rotate_object_sticky_cardinal(actual_rotation);
	return true;
}

bool map_object::is_within(int check_x, int check_y){
	if(check_x > x && check_x < x1 && check_y < y && check_y > y1)
		return true;
	check_x++; check_y++;
	if(check_x > x && check_x < x1 && check_y < y && check_y > y1)
		return true;
	return false;
}

void map_object::step_explosion(void){}
void map_object::hit_by_projectile(bullet hitby){}

void map_object::object_action(void) {}

map_data::map_data() {
	c_x = 0xFFFFFFFF;
	c_y = 0xFFFFFFFF;
	loaded_x = 0;
	loaded_x1 = 0;
	loaded_y = 0;
	loaded_y1 = 0;
}
map_data::~map_data() {}

