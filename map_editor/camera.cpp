#include "StdAfx.h"
#include "map_editor.h"

camera::camera(void){
	x=0,y=0,sx=0,sy=0,w=0,h=0;
	camera_initialized = false;
	owns_device = false;
}

camera::~camera(void){}

void camera::init_camera(void){
	camera_initialized = true;
}

void camera::render(void){
	//x = app.eng.camera_x;
	//y = app.eng.camera_y;

	int tefy = y % 100; // terrain effective x
	int tefx = 100 - (x % 100);

	// Terrain
	driver->draw2DImage(app.eng.terrain, // texture
		irr::core::position2d<irr::s32>(sx+tefx, sy+tefy), // dest rect
		irr::core::rect<irr::s32>(0,0,w-tefx, h-tefy), // source rect
		0, // clip rect
		irr::video::SColor(255,255,255,255),  //colors
		true);

	// Terrain left edge
	driver->draw2DImage(app.eng.terrain, // texture
		irr::core::position2d<irr::s32>(sx, sy+tefy), // dest rect
		irr::core::rect<irr::s32>(100-tefx,0,100,h-tefy), // source rect
		0, // clip rect
		irr::video::SColor(255,255,255,255),  //colors
		true); // use alpha

	// terrain top edge
	driver->draw2DImage(app.eng.terrain, // texture
		irr::core::position2d<irr::s32>(sx+tefx, sy), // dest rect
		irr::core::rect<irr::s32>(0,100-tefy,w-tefx,100), // source rect
		0, // clip rect
		irr::video::SColor(255,255,255,255),  //colors
		true); // use alpha

	// terrain top corner
	driver->draw2DImage(app.eng.terrain, // texture
		irr::core::position2d<irr::s32>(sx, sy), // dest rect
		irr::core::rect<irr::s32>(100-tefx,100-tefy,100,100), // source rect
		0, // clip rect
		irr::video::SColor(255,255,255,255),  //colors
		true); // use alpha

	// Draw all the map objects
	for(vector<boost::shared_ptr<map_object> >::iterator itr = app.eng.gd.map_objects.begin(); itr != app.eng.gd.map_objects.end();){
		int rotation_n = 0;
		if((*itr)->to_remove){
			itr = app.eng.gd.map_objects.erase(itr);
			continue;
		}else{
			map<string, sprite_class>::iterator this_sprite_class = app.eng.gd.sprite_classes.find((*itr)->sprite_class);
			if(this_sprite_class != app.eng.gd.sprite_classes.end()){
				int layer_num = -1;
				for(vector<sprite_layer>::iterator slitr = this_sprite_class->second.layers.begin(); slitr != this_sprite_class->second.layers.end(); slitr++){
					layer_num++;
					int active_frame = (*itr)->active_layer_frames[layer_num];
					irr::video::ITexture *sprite_texture = slitr->images[active_frame];

					int object_width = (*itr)->x1-(*itr)->x;
					int object_height = (*itr)->y-(*itr)->y1;

					int rel_x = (*itr)->x-x;
					int rel_y = y-(*itr)->y;

					int prtc_x = sx+rel_x; // Position Relative to Camera
					int prtc_y = sy+h+rel_y;

					int object_center_x = prtc_x + object_width/2;
					int object_center_y = prtc_y + object_height/2;

					// only draw if within the bounds of the camera
					if((prtc_x+object_width) > sx && prtc_x < sx+w &&
						(prtc_y+object_height) > sy && prtc_y < sy+h){

						app.eng.draw2DImage(driver, 
							sprite_texture,  
							irr::core::rect<irr::s32>(0,0, object_width, object_height),  // source rect
							irr::core::position2d<irr::s32>(prtc_x,prtc_y),  // position
							irr::core::position2d<irr::s32>(object_center_x, object_center_y),  // rotation point
							(irr::f32)(*itr)->rotations[rotation_n++], 
							irr::core::position2df(1,1), 
							true, 
							irr::video::SColor(255,255,255,255),
							irr::core::rect<irr::s32>(sx,sy,sx+w,sy+h)
							);

						if((*itr)->overlay_graphic){
							app.eng.draw2DImage(driver, 
								(*itr)->overlay_graphic,  
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

			(*itr)->step_explosion(); // z8rYotiiFP8
		}
		itr++;
	}

	// draw projectiles
	for(vector<boost::shared_ptr<creature> >::iterator enemy_itr = app.eng.gd.enemies.begin(); enemy_itr != app.eng.gd.enemies.end();){
		if((*enemy_itr)->to_remove){
			enemy_itr = app.eng.gd.enemies.erase(enemy_itr);
			continue;
		}

		for(vector<::bullet>::iterator bulletitr = (*enemy_itr)->bullets.begin(); bulletitr != (*enemy_itr)->bullets.end();){
			if(app.eng.projectile_step(*enemy_itr, *bulletitr)){
				app.eng.draw2DImage(driver, 
					bulletitr->img,  
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
				bulletitr = (*enemy_itr)->bullets.erase(bulletitr);
				continue;
			}
			bulletitr++;
		}

		enemy_itr++;
	}

	for(vector<boost::shared_ptr<creature> >::iterator player_itr = app.eng.gd.players.begin(); player_itr != app.eng.gd.players.end();){
		if((*player_itr)->to_remove){
			player_itr = app.eng.gd.players.erase(player_itr);
			continue;
		}

		for(vector<::bullet>::iterator bulletitr = (*player_itr)->bullets.begin(); bulletitr != (*player_itr)->bullets.end();){
			if(app.eng.projectile_step(*player_itr, *bulletitr)){
				app.eng.draw2DImage(driver, 
					bulletitr->img,  
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
	//if((x + cam_width) > map_width){camera_x = (map_width - cam_width);}
	//if((y + cam_height) > map_height){camera_y = (map_height - cam_height);}
}
