#include "StdAfx.h"
#include "steel_tank.h"

camera::camera(void){
	x=0,y=0,sx=0,sy=0,w=0,h=0;
	camera_initialized = false;
	owns_device = false;
	init_center = false;
	cam_num = 0;
}

camera::~camera(void){
	md.map_objects.clear();
	this_window.reset();
}

void camera::init_camera(void){
	camera_initialized = true;
}

void camera::render(void){
	//x = app.eng.camera_x;
	//y = app.eng.camera_y;

	int tefy = y % 100; // terrain effective x
	int tefx = 100 - (x % 100);

	// Terrain
	driver->draw2DImage(this_window->terrain, // texture
		irr::core::position2d<irr::s32>(sx+tefx, sy+tefy), // dest rect
		irr::core::rect<irr::s32>(0,0,w-tefx, h-tefy), // source rect
		0, // clip rect
		irr::video::SColor(255,255,255,255),  //colors
		true);

	// Terrain left edge
	driver->draw2DImage(this_window->terrain, // texture
		irr::core::position2d<irr::s32>(sx, sy+tefy), // dest rect
		irr::core::rect<irr::s32>(100-tefx,0,100,h-tefy), // source rect
		0, // clip rect
		irr::video::SColor(255,255,255,255),  //colors
		true); // use alpha

	// terrain top edge
	driver->draw2DImage(this_window->terrain, // texture
		irr::core::position2d<irr::s32>(sx+tefx, sy), // dest rect
		irr::core::rect<irr::s32>(0,100-tefy,w-tefx,100), // source rect
		0, // clip rect
		irr::video::SColor(255,255,255,255),  //colors
		true); // use alpha

	// terrain top corner
	driver->draw2DImage(this_window->terrain, // texture
		irr::core::position2d<irr::s32>(sx, sy), // dest rect
		irr::core::rect<irr::s32>(100-tefx,100-tefy,100,100), // source rect
		0, // clip rect
		irr::video::SColor(255,255,255,255),  //colors
		true); // use alpha


	vector<map<int, shared_ptr<map_object> >* > object_sets;
	object_sets.push_back(&md.map_objects);
	object_sets.push_back(&app.eng.shared_objects);

	vector<map<int, shared_ptr<map_object> >* >::iterator os_itr;

	for(os_itr = object_sets.begin(); os_itr != object_sets.end(); os_itr++){
		// Draw all the map objects
		map<int, shared_ptr<map_object> > *this_object_set = *os_itr;

		map<int, boost::shared_ptr<map_object> >::iterator itr;
		for(itr = this_object_set->begin(); itr != this_object_set->end();){
			boost::shared_ptr<map_object> this_object = itr->second;

			if(this_object->object_class == "start_flame" ||
				this_object->object_class == "start_hydro" ||
				this_object->object_class == "start_shock" ||
				this_object->object_class == "start_rock"){
					itr++;
					continue;
			}

			bool dontdraw = false;
			if(this_object->explosion_step > -1){
				//dontdraw = true;
			}

			int rotation_n = 0;
			if(this_object->to_remove){
				app.eng.destroyed_enemies[this_object->object_id] = true;
				//app.eng.gd.loaded_objects.erase(this_object->object_id);
				itr = this_object_set->erase(itr);
				continue;
			}else{
				map<string, sprite_class>::iterator this_sprite_class = this_window->sprite_classes.find(this_object->sprite_class);
				if(this_sprite_class != this_window->sprite_classes.end()){
					int layer_num = -1;
					for(vector<sprite_layer>::iterator slitr = this_sprite_class->second.layers.begin(); slitr != this_sprite_class->second.layers.end(); slitr++){
						layer_num++;
						int active_frame = (this_object)->active_layer_frames[layer_num];
						irr::video::ITexture *sprite_texture = slitr->images[active_frame];

						int object_width = this_object->x1-this_object->x;
						int object_height = this_object->y-this_object->y1;

						int rel_x = this_object->x-x;
						int rel_y = y-this_object->y;

						int prtc_x = sx+rel_x; // Position Relative to Camera
						int prtc_y = sy+h+rel_y;

						int object_center_x = prtc_x + object_width/2;
						int object_center_y = prtc_y + object_height/2;

						// only draw if within the bounds of the camera
						if((prtc_x+object_width) > sx && prtc_x < sx+w &&
							(prtc_y+object_height) > sy && prtc_y < sy+h){

								if(!dontdraw){
							app.eng.draw2DImage(driver, 
								sprite_texture,  
								irr::core::rect<irr::s32>(0,0, object_width, object_height),  // source rect
								irr::core::position2d<irr::s32>(prtc_x,prtc_y),  // position
								irr::core::position2d<irr::s32>(object_center_x, object_center_y),  // rotation point
								(irr::f32)this_object->rotations[rotation_n++], 
								irr::core::position2df(1,1), 
								true, 
								irr::video::SColor(255,255,255,255),
								irr::core::rect<irr::s32>(sx,sy,sx+w,sy+h)
								);
								}

							irr::video::ITexture *exp = 0;

							if(this_object->explosion_step > 0)
								exp = this_window->explosion_steps[this_object->explosion_step];

							if(exp){
								app.eng.draw2DImage(driver, 
									exp,  
									irr::core::rect<irr::s32>(0,0, object_width, object_height),  // source rect
									irr::core::position2d<irr::s32>(prtc_x,prtc_y),  // position
									irr::core::position2d<irr::s32>(object_center_x, object_center_y),  // rotation point
									(irr::f32)0, 
									irr::core::position2df(1,1), 
									true, 
									irr::video::SColor(255,255,255,255),
									irr::core::rect<irr::s32>(sx,sy,sx+w,sy+h)
									);
							}
						}
					}
				}
			}

			this_object->step_explosion(); // z8rYotiiFP8

			itr++;
		}
	}

	// draw projectiles
	for(map<int, boost::shared_ptr<map_object> >::iterator itr = md.map_objects.begin(); itr != md.map_objects.end();){
		boost::shared_ptr<map_object> this_object = itr->second;
		if(!this_object->is_enemy){itr++;continue;}


		boost::shared_ptr<creature> this_creature = boost::shared_dynamic_cast<creature>(this_object);

		if(this_creature->to_remove){
			app.eng.destroyed_enemies[this_creature->object_id] = true;

			itr = md.map_objects.erase(itr);
			continue;
		}

		for(vector<::bullet>::iterator bulletitr = this_creature->bullets.begin(); bulletitr != this_creature->bullets.end();){
			
			irr::video::ITexture *img;
			switch(bulletitr->type){
			case TYPE_FLAME:
				img = this_window->flame_bullet;
				break;
			case TYPE_HYDRO:
				img = this_window->hydro_bullet;
				break;
			case TYPE_ROCK:
				img = this_window->rock_bullet;
				break;
			case TYPE_SHOCK:
				img = this_window->shock_bullet;
				break;
			default:
				img = this_window->bulletimg;
			}

			if(true){
				app.eng.draw2DImage(driver, 
					img,  
					irr::core::rect<irr::s32>(0,0,15,15), 
					irr::core::position2d<irr::s32>(sx+bulletitr->x-x,sy+h+(y-bulletitr->y)), 
					irr::core::position2d<irr::s32>(sx+bulletitr->x+x,sy+h+(y-bulletitr->y)), 
					0, 
					irr::core::position2df(1,1), 
					true, 
					irr::video::SColor(255,255,255,255),
					irr::core::rect<irr::s32>(sx,sy,sx+w,sy+h)
					);
			}else{
				bulletitr = this_creature->bullets.erase(bulletitr);
				continue;
			}
			bulletitr++;
		}

		itr++;
	}

	for(vector<boost::shared_ptr<creature> >::iterator player_itr = app.eng.gd.players.begin(); player_itr != app.eng.gd.players.end();){
		if((*player_itr)->to_remove){
			//player_itr = app.eng.gd.players.erase(player_itr);
			player_itr++;
			continue;
		}

		for(vector<::bullet>::iterator bulletitr = (*player_itr)->bullets.begin(); bulletitr != (*player_itr)->bullets.end();){
			irr::video::ITexture *img;
			switch(bulletitr->type){
			case TYPE_FLAME:
				img = this_window->flame_bullet;
				break;
			case TYPE_HYDRO:
				img = this_window->hydro_bullet;
				break;
			case TYPE_ROCK:
				img = this_window->rock_bullet;
				break;
			case TYPE_SHOCK:
				img = this_window->shock_bullet;
				break;
			default:
				img = this_window->bulletimg;
			}

			if(1){
				app.eng.draw2DImage(driver, 
					img,  
					irr::core::rect<irr::s32>(0,0,15,15), 
					irr::core::position2d<irr::s32>(sx+bulletitr->x-x,sy+h+(y-bulletitr->y)), 
					irr::core::position2d<irr::s32>(sx+bulletitr->x+x,sy+h+(y-bulletitr->y)), 
					0, 
					irr::core::position2df(1,1), 
					true, 
					irr::video::SColor(255,255,255,255),
					irr::core::rect<irr::s32>(sx,sy,sx+w,sy+h)
					);
			}else{
				bulletitr = (*player_itr)->bullets.erase(bulletitr);
				continue;
			}
			bulletitr++;
		}

		player_itr++;
	}

	// draw health bar
	boost::shared_ptr<creature> creature_m;
	creature_m = boost::shared_dynamic_cast<creature>(centered_object);
	if(creature_m){
		double current_health = (double)creature_m->health / 100.0;
		int bar_width = (int)(floor(current_health * w));
		driver->draw2DRectangle(irr::video::SColor(255,255,0,0), irr::core::rect<irr::s32>(sx,sy,sx+bar_width,sy+10));
	}
}

void camera::center_on_object(boost::shared_ptr<map_object> the_object){
	int cam_width = w;
	int cam_height =h;

	int avg_px = 0;
	int avg_py = 0;

	int total_px = 0;
	int total_py = 0;
	int total_players = 0;

	x = the_object->x - (cam_width/2);
	y = the_object->y - (cam_height/2);

	if(x < 0){x = 0;}
	if(y < 0){y = 0;}

	int center_x = x + w/2;
	int center_y = y + h/2;

	if(cam_width > 0 && cam_height > 0 && init_center){
		if((center_x - md.c_x) > w/2 || (center_y - md.c_y) > h/2 || (md.c_x - center_x) > w/2 || (md.c_y - center_y) > h/2){
#ifdef _DEBUG
			static int map_load_count = 0;
			string obj_count = boost::lexical_cast<string>((int)md.map_objects.size());
			string out_str = "Map load " + boost::lexical_cast<string>(map_load_count++) + " map objects: " + obj_count + "\n";
			::OutputDebugStringA(out_str.data());
#endif

			//app.eng.gd.cull_map(md, md.loaded_x, md.loaded_y, md.loaded_x1, md.loaded_y1);

			int l_x = x - (w+LOAD_REGION);
			int l_y = y - (h+LOAD_REGION);
			int l_x1 = x + ((2*w)+LOAD_REGION);
			int l_y1 = y + ((2*h)+LOAD_REGION);

			md.c_x = the_object->x;
			md.c_y = the_object->y;
			app.eng.gd.load_map_from_coordinates(md, l_x, l_y, l_x1, l_y1);
			app.eng.gd.cull_map(md, l_x, l_y, l_x1, l_y1);
		}
	}

	init_center = true;
	//if((x + cam_width) > map_width){camera_x = (map_width - cam_width);}
	//if((y + cam_height) > map_height){camera_y = (map_height - cam_height);}
}
