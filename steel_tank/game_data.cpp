#include "StdAfx.h"
#include "steel_tank.h"

sprite_layer::sprite_layer() {}
sprite_layer::~sprite_layer() {}
sprite_class::sprite_class() {}
sprite_class::~sprite_class() {}

game_data::game_data() {
	gamedb = 0;
}

game_data::~game_data() {
	if(0 != gamedb){
		sqlite3_close(gamedb);
	}
}

void game_data::load_sprite_classes(map<string, sprite_class> &sprite_classes, irr::video::IVideoDriver *driver){
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
				irr::video::ITexture* this_texture = driver->getTexture(fpitr->data());
				slitr->images.push_back(this_texture);
			}
		}
	}

	sqlite3_finalize(preped_st);
	//sqlite3_close(gamedb); gamedb = 0;
}

void game_data::find_start_positions(void){
	int errc = 0;
	if(0 == gamedb)
		errc = sqlite3_open(GAME_DB, &gamedb);
	if(0 == gamedb || errc != SQLITE_OK){return;}

	sqlite3_stmt *preped_st;

	string query_text = "select OID,* from map_objects where spriteclass like 'start%';";
	
	if(SQLITE_OK != (errc = sqlite3_prepare_v2(gamedb, query_text.data(), query_text.length()+1, &preped_st, NULL))){
		const char *errmsg = sqlite3_errmsg(gamedb);
		return;
	}
	
	while(SQLITE_ROW == sqlite3_step(preped_st)){
		int oid = sqlite3_column_int(preped_st, 0);
		string object_class = string((char*)sqlite3_column_text(preped_st, 2));
		int x = sqlite3_column_int(preped_st, 4);
		int y = sqlite3_column_int(preped_st, 5);
		int x1 = sqlite3_column_int(preped_st, 6) + x;
		int y1 = y - sqlite3_column_int(preped_st, 7);

		int which_type = -1;

		if("start_flame" == object_class){
			which_type = TYPE_FLAME;
		}else if("start_hydro" == object_class){
			which_type = TYPE_HYDRO;
		}else if("start_shock" == object_class){
			which_type = TYPE_SHOCK;
		}else if("start_rock" == object_class){
			which_type = TYPE_ROCK;
		}

		if(which_type > -1){
			unsigned int wtype = which_type+1;
			while(wtype > start_data.size()){
				s_start_data s;
				s.oid = 0, s.x = 0, s.y = 0;
				start_data.push_back(s);
			}

			start_data[which_type].oid = oid;
			start_data[which_type].x = x;
			start_data[which_type].y = y;
		}
	}
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

		if(app.eng.destroyed_enemies[oid]){
			continue;
		}

		if(md.map_objects.find(oid) != md.map_objects.end()){
			continue;
		}

		map<int,boost::shared_ptr<map_object> >::iterator found_object_itr;
		found_object_itr = loaded_objects.find(oid);
		if(found_object_itr != loaded_objects.end()){
			boost::shared_ptr<map_object> found_object = found_object_itr->second;
			found_object->use_count++;
			md.map_objects[oid] = found_object;
			continue;
		}

		boost::shared_ptr<map_object> an_object;

		string object_class = string((char*)sqlite3_column_text(preped_st, 2));
		bool is_creature = false;



		if("flame_tank" == object_class ||
			"shock_tank" == object_class ||
			"hydro_tank" == object_class ||
			"rock_tank" == object_class){
				an_object.reset(new creature);
				is_creature = true;

		}else{
			an_object.reset(new map_object);
		}

		if("start_flame" == object_class ||
			"start_hydro" == object_class ||
			"start_shock" == object_class ||
			"start_rock" == object_class){
				an_object->is_solid = false;
		}


		an_object->use_count = 1;

		an_object->object_name = string((char*)sqlite3_column_text(preped_st, 1));
		an_object->object_class = object_class;
		an_object->sprite_class = string((char*)sqlite3_column_text(preped_st, 3));
		an_object->x = sqlite3_column_int(preped_st, 4);
		an_object->y = sqlite3_column_int(preped_st, 5);
		an_object->x1 = sqlite3_column_int(preped_st, 6) + an_object->x;
		an_object->y1 = an_object->y - sqlite3_column_int(preped_st, 7);
		an_object->object_id = oid;

		// hacky hacky
		for(int i=0; i<20; i++){
			an_object->rotations.push_back(0);
			an_object->active_layer_frames.push_back(0);
		}
		
		if(is_creature){
			boost::shared_ptr<creature> creature_object;
			creature_object = boost::shared_dynamic_cast<creature>(an_object);

			if("flame_tank" == object_class){
				creature_object->creature_type = TYPE_FLAME;
			}else if("shock_tank" == object_class){
				creature_object->creature_type = TYPE_SHOCK;
			}else if("rock_tank" == object_class){
				creature_object->creature_type = TYPE_ROCK;
			}else if("hydro_tank" == object_class){
				creature_object->creature_type = TYPE_HYDRO;
			}


			creature_object->is_enemy = true;

			md.map_objects[oid] = creature_object;
			loaded_objects[oid] = creature_object;
		}else{
			md.map_objects[oid] = an_object;
			loaded_objects[oid] = an_object;
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


			itr->second->use_count--;
			int mo_id = itr->second->object_id;
			if(itr->second->use_count <= 0){
				map<int, boost::shared_ptr<map_object> >::iterator lo_itr;
				lo_itr = loaded_objects.find(mo_id);
				if(lo_itr != loaded_objects.end()){
					loaded_objects.erase(lo_itr);
				}
			}

			md.map_objects.erase(itr++);
			continue;
		}
		itr++;
	}

}

void game_data::make_players(void){
	int h_oid = 0;
	for(int i=0; i<app.eng.number_of_players; i++){
		boost::shared_ptr<creature> hero(new creature);

		hero->object_id  = start_data[type_selections[i]].oid;
		hero->x  = start_data[type_selections[i]].x;
		hero->y  = start_data[type_selections[i]].y;
		hero->creature_type = type_selections[i];

		switch(type_selections[i]){
		case TYPE_FLAME:
			hero->sprite_class = "flame_tank";
			break;
		case TYPE_HYDRO:
			hero->sprite_class = "hydro_tank";
			break;
		case TYPE_SHOCK:
			hero->sprite_class = "shock_tank";
			break;
		case TYPE_ROCK:
			hero->sprite_class = "rock_tank";
			break;
		}

		hero->x1 = hero->x + 80;
		hero->y1 = hero->y - 80;

		hero->object_name = "player";
		hero->rotations.push_back(0);
		hero->rotations.push_back(90);
		hero->active_layer_frames.push_back(0);
		hero->active_layer_frames.push_back(0);
		hero->active_layer_frames.push_back(0);

		players.push_back(hero);

		app.eng.gd.loaded_objects[hero->object_id] = hero;
		app.eng.shared_objects[hero->object_id] = hero;
	}
}
