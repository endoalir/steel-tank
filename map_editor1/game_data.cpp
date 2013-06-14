#include "StdAfx.h"
#include "map_editor.h"

sprite_layer::sprite_layer() {}
sprite_layer::~sprite_layer() {}
sprite_class::sprite_class() {}
sprite_class::~sprite_class() {}

map_object::map_object() {
	object_name="(none)";
	x=0,y=0,x1=0,y1=0;
	actual_rotation = 0;
	to_remove = false;
	is_solid = true;
	overlay_graphic = 0;
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
	return false;
}

void map_object::step_explosion(void){}
void map_object::hit_by_projectile(bullet hitby){}

void map_object::object_action(void) {}

map_data::map_data() {
	c_x = 0, c_y = 0;
	loaded_x = 0, loaded_y = 0;
	loaded_x1 = 0, loaded_y1 = 0;
}
map_data::~map_data() {}

void map_data::insert_object(map_object &mo){
	int errc;
	if(0 == app.eng.gd.gamedb)
		errc = sqlite3_open(GAME_DB, &app.eng.gd.gamedb);
	if(0 == app.eng.gd.gamedb || errc != SQLITE_OK){return;}

	string query_text = "insert into map_objects ()";

}


game_data::game_data() {
	gamedb = 0;
}

game_data::~game_data() {
	if(0 != gamedb){
		sqlite3_close(gamedb);
	}
}

void game_data::load_sprite_classes(void){
	int errc = 0;
	if(0 == gamedb)
		errc = sqlite3_open(GAME_DB, &gamedb);
	if(0 == gamedb || errc != SQLITE_OK){return;}

	sqlite3_stmt *preped_st;
	string query_text = "select * from sprite_classes;";
	if(SQLITE_OK != (errc = sqlite3_prepare_v2(gamedb, query_text.data(), query_text.length()+1, &preped_st, NULL))){
		const char *errmsg = sqlite3_errmsg(gamedb);
		return;
	}

	while(SQLITE_ROW == sqlite3_step(preped_st)){
		string classname = (char*)sqlite3_column_text(preped_st, 0);
		if(sprite_classes.find(classname) == sprite_classes.end()){
			sprite_class sc;
			sc.class_name = classname;
			sprite_classes[classname] = sc;
		}

		sprite_classes[classname].w = sqlite3_column_int(preped_st, 3);
		sprite_classes[classname].h = sqlite3_column_int(preped_st, 4);

		int layer_num = sqlite3_column_int(preped_st, 1);
		if(0 == layer_num){layer_num++;}

		int number_of_layers = sprite_classes[classname].layers.size();
		
		if(layer_num > number_of_layers){
			int how_many_more_layers = layer_num - number_of_layers;

			for(int n=0; n< how_many_more_layers; n++){
				sprite_classes[classname].layers.push_back(sprite_layer());
			}
		}
		
		sprite_classes[classname].layers[layer_num-1].fspecs.push_back(string((char*)sqlite3_column_text(preped_st, 2)));
	}

	for(map<string, sprite_class>::iterator scitr = sprite_classes.begin(); scitr != sprite_classes.end(); scitr++){
		for(vector<sprite_layer>::iterator slitr = scitr->second.layers.begin(); slitr != scitr->second.layers.end(); slitr++){
			for(vector<string>::iterator fpitr = slitr->fspecs.begin(); fpitr != slitr->fspecs.end(); fpitr++){
				irr::video::ITexture* this_texture = app.eng.driver->getTexture(fpitr->data());
				slitr->images.push_back(this_texture);
			}
		}
	}

	sqlite3_finalize(preped_st);
}

void game_data::load_map_from_coordinates(map_data &md, int x, int y, int x1, int y1){
	md.loaded_x = x;
	md.loaded_x1 = x1;
	md.loaded_y = y;
	md.loaded_y1 = y1;

	int errc = 0;
	if(0 == gamedb)
		errc = sqlite3_open(GAME_DB, &gamedb);
	if(0 == gamedb || errc != SQLITE_OK){return;}

	sqlite3_stmt *preped_st;

	string query_text = "select OID,* from map_objects where x>? and y>? and x<? and y<?;";
	
	if(SQLITE_OK != (errc = sqlite3_prepare_v2(gamedb, query_text.data(), query_text.length()+1, &preped_st, NULL))){
		const char *errmsg = sqlite3_errmsg(gamedb);
		return;
	}
	
	sqlite3_bind_int(preped_st, 1, x);
	sqlite3_bind_int(preped_st, 2, y);
	sqlite3_bind_int(preped_st, 3, x1);
	sqlite3_bind_int(preped_st, 4, y1);

	while(SQLITE_ROW == sqlite3_step(preped_st)){
		int oid = sqlite3_column_int(preped_st, 0);

		boost::shared_ptr<map_object> an_object;

		string object_class = string((char*)sqlite3_column_text(preped_st, 2));
		bool is_creature = false;

		an_object->object_name = string((char*)sqlite3_column_text(preped_st, 1));
		//an_object->object_class = object_class;
		an_object->sprite_class = string((char*)sqlite3_column_text(preped_st, 3));
		an_object->x = sqlite3_column_int(preped_st, 4);
		an_object->y = sqlite3_column_int(preped_st, 5);
		an_object->x1 = sqlite3_column_int(preped_st, 6) + an_object->x;
		an_object->y1 = an_object->y - sqlite3_column_int(preped_st, 7);
		//an_object->object_id = oid;

		// hacky hacky
		for(int i=0; i<20; i++){
			an_object->rotations.push_back(0);
			an_object->active_layer_frames.push_back(0);
		}
		
	}

	sqlite3_finalize(preped_st);
}

void game_data::cull_map(map_data &md, int x, int y, int x1, int y1){
	map<int, boost::shared_ptr<map_object> >::iterator itr;

	for(itr = md.map_objects.begin(); itr != md.map_objects.end();){
		boost::shared_ptr<map_object> this_object = itr->second;
		if(this_object->x < x || this_object->y < y || this_object->x1 > x1 || this_object->y1 > y1){

			bool itsaplayer = false;
			for(unsigned int i=0; i<players.size(); i++){
				if(players[i]== this_object)
					itsaplayer = true;
			}

			if(itsaplayer){
				itr++;
				continue;
			}


			md.map_objects.erase(itr++);
			continue;
		}
		itr++;
	}

}

void game_data::write_map(map_data &md){
	int errc = 0;
	if(0 == gamedb)
		errc = sqlite3_open(GAME_DB, &gamedb);
	if(0 == gamedb || errc != SQLITE_OK){return;}

	sqlite3_stmt *preped_st;

	// clear table
	string query_text = "delete from map_objects;";
	if(SQLITE_OK != (errc = sqlite3_prepare_v2(gamedb, query_text.data(), query_text.length()+1, &preped_st, NULL))){
		const char *errmsg = sqlite3_errmsg(gamedb);
		return;
	}
	sqlite3_step(preped_st);
	sqlite3_finalize(preped_st);

	vector<boost::shared_ptr<map_object> >::iterator mo;

	for(mo = app.eng.gd.map_objects.begin(); mo!=app.eng.gd.map_objects.end(); mo++){
		
		query_text = "insert into map_objects (name, objclass, spriteclass, x, y, w, h) values (?,?,?,?,?,?,?)";
		if(SQLITE_OK != (errc = sqlite3_prepare_v2(gamedb, query_text.data(), query_text.length()+1, &preped_st, NULL))){
			const char *errmsg = sqlite3_errmsg(gamedb);
			return;
		}

		sqlite3_bind_text(preped_st, 1, (*mo)->object_name.data(), (*mo)->object_name.length()+1, SQLITE_STATIC);
		sqlite3_bind_text(preped_st, 2, (*mo)->sprite_class.data(), (*mo)->sprite_class.length()+1, SQLITE_STATIC);
		sqlite3_bind_text(preped_st, 3, (*mo)->sprite_class.data(), (*mo)->sprite_class.length()+1, SQLITE_STATIC);
		sqlite3_bind_int(preped_st, 4, (*mo)->x);
		sqlite3_bind_int(preped_st, 5, (*mo)->y);
		sqlite3_bind_int(preped_st, 6, (*mo)->x1 - (*mo)->x);
		sqlite3_bind_int(preped_st, 7, (*mo)->y - (*mo)->y1);
		
		errc = sqlite3_step(preped_st);
	
		sqlite3_reset(preped_st);
		sqlite3_clear_bindings(preped_st);
	}

	sqlite3_finalize(preped_st);

}