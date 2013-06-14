#include "StdAfx.h"
#include "steel_tank.h"
#include "game_sequence.h"

game_sequence::game_sequence(int seq_num){
	sequence_over = false;
	sequence_start = clock();
	sequence_number = seq_num;
}
game_sequence::~game_sequence(void){}
void game_sequence::load(void){}

void game_sequence::reset(void){
	sequence_over = false;
	sequence_start = clock();
}

splash_sequence::splash_sequence(int seq_num) : game_sequence(seq_num){
	splashtimer = 0;
	splashscreen = 0;
}

void splash_sequence::load(void){
	splashtimer = sequence_start + CLOCKS_PER_SEC * 2;
	splashscreen = app.app_windows[0]->driver->getTexture("../stock/splash.jpg");
}

void splash_sequence::reset(void){
	game_sequence::reset();
	splashtimer = sequence_start + CLOCKS_PER_SEC * 2;
}

void splash_sequence::render(void){
	app.app_windows[0]->driver->draw2DImage(splashscreen, irr::core::position2d<irr::s32>(0,0), irr::core::rect<irr::s32>(0,0,app.app_windows[0]->client_w,app.app_windows[0]->client_h), 0, irr::video::SColor(255,255,255,255), true);
	if(clock() > splashtimer){
		sequence_over = true;
	}
}

title_sequence::title_sequence(int seq_num) : game_sequence(seq_num){
	titlescreen = 0;
}

void title_sequence::load(void){
	titlescreen = app.app_windows[0]->driver->getTexture("../stock/title.jpg");
}

void title_sequence::render(void){
	app.app_windows[0]->driver->draw2DImage(titlescreen, irr::core::vector2d<irr::s32>(0,0));
	if(app.eng.js.number_of_joysticks > 0 && app.eng.polled_js){
		if(app.eng.js.joystates[0].rgbButtons[7]){
			sequence_over = true;
		}
	}
}

playerselect_sequence::playerselect_sequence(int seq_num) : game_sequence(seq_num){
	player_select_screen = 0;
}

void playerselect_sequence::load(void){
	player_select_screen = app.app_windows[0]->driver->getTexture("../stock/player_select.jpg");
}

void playerselect_sequence::reset(void){
	game_sequence::reset();
	app.eng.number_of_players = 0;
}

void playerselect_sequence::render(void){
	app.app_windows[0]->driver->draw2DImage(player_select_screen, irr::core::vector2d<irr::s32>(0,0));

	if((clock() - sequence_start) > 200)
		if(app.eng.js.number_of_joysticks > 0 && app.eng.polled_js)
			for(int n = 0; n < 4; n++)
				if(app.eng.js.joystates[0].rgbButtons[n])
					app.eng.number_of_players = n+1;

	if(app.eng.number_of_players > 0){ sequence_over = true; }
}

void play_start_sequence::load(void){
	playerstart = app.app_windows[0]->driver->getTexture("../stock/playerstart.jpg");
	splashtimer = sequence_start + CLOCKS_PER_SEC * 3;
}

play_start_sequence::play_start_sequence(int seq_num) : game_sequence(seq_num){
	init_cam = false;
	playerstart = 0;

	splashtimer = 0;
}

void play_start_sequence::reset(void){
	game_sequence::reset();
	splashtimer = sequence_start + CLOCKS_PER_SEC * 3;
	init_cam = false;

	vector<shared_ptr<window> >::iterator itr;
	for(itr = app.app_windows.begin(); itr != app.app_windows.end(); itr++){
		shared_ptr<window> this_window(*itr);
		this_window->cams.clear();
	}

	app.eng.gd.loaded_objects.clear();
	app.eng.gd.players.clear();
	app.eng.destroyed_enemies.clear();
}

void play_start_sequence::render(void){
	if(!init_cam){
		init_cam = true;

		app.eng.gd.make_players();

		app.app_windows[0]->cams.make_camera();
		app.app_windows[0]->cams.set_centered_object(0, app.eng.gd.players[0]);
		
		for(int i=1; i<app.eng.number_of_players; i++){
			/*boost::shared_ptr<window> this_window(new window());
			this_window->cams.this_window = this_window;
			app.app_windows.push_back(this_window);
			this_window->make(SW_SHOW);*/

			app.app_windows[0]->cams.make_camera();			
			app.app_windows[0]->cams.set_centered_object(i, app.eng.gd.players[i]);

		}

		app.eng.repos_cams();
		app.reset_cameras();
	}

	app.app_windows[0]->driver->draw2DImage(playerstart, irr::core::position2d<irr::s32>(0,0), irr::core::rect<irr::s32>(0,0,app.app_windows[0]->client_w,app.app_windows[0]->client_h), 0, irr::video::SColor(255,255,255,255), true);
	if(clock() > splashtimer){
		sequence_over = true;
	}
}

playertypeselect_sequence::playertypeselect_sequence(int seq_num) : game_sequence(seq_num){
	pick_one = 0;
}

void playertypeselect_sequence::load(void){
	pick_one = app.app_windows[0]->driver->getTexture("../stock/pick_one.png");
	reset();
}
void playertypeselect_sequence::reset(void){
	game_sequence::reset();

	current_selection.clear();
	confirmed_selection.clear();
	taken_selections.clear();
	last_move.clear();

	for(int i=0; i<app.eng.number_of_players; i++){
		current_selection[i] = -1;
		confirmed_selection[i] = false;
		taken_selections[i] = false;
		last_move[i] = 0;
	}

}

void playertypeselect_sequence::move_selection(int p, int dir){
	if(clock() - last_move[p] < 300){return;}
	last_move[p] = clock();

	for(int t=0; t<4; t++){
		1==dir ? current_selection[p]++ : current_selection[p]--;
		if(current_selection[p] > 3)
			current_selection[p] = 0;
		if(current_selection[p] < 0)
			current_selection[p] = 3;

		if(!taken_selections[current_selection[p]])
			break;
	}
}

void playertypeselect_sequence::render(void){
	app.app_windows[0]->driver->draw2DImage(pick_one, irr::core::vector2d<irr::s32>(0,0),
		irr::core::rect<irr::s32>(0,0,400,100), 0, irr::video::SColor(255, 255,255,255), false);

	if(app.eng.js.number_of_joysticks > 0 && app.eng.polled_js){
		for(int i=0; i < app.eng.number_of_players; i++){
			if(i+1 > app.eng.js.number_of_joysticks){
				continue;
			}
			if(!confirmed_selection[i]){
				if(app.eng.js.joystates[i].rgdwPOV[0] == 0){ // UP
					move_selection(i, 0);
				}
				if(app.eng.js.joystates[i].rgdwPOV[0] == 180 * 100){ // DOWN
					move_selection(i, 1);
				}
			}

			if(app.eng.js.joystates[i].rgbButtons[0]){
				if(current_selection[i] > -1){
					confirmed_selection[i] = true;
					taken_selections[current_selection[i]] = true;
					for(int j=0; j<app.eng.number_of_players; j++){
						if(i!=j && current_selection[j] == current_selection[i]){
							move_selection(j,1);
						}
					}
				}
			}
			if(app.eng.js.joystates[i].rgbButtons[1]){
				if(current_selection[i] > -1){
					confirmed_selection[i] = false;
					taken_selections[current_selection[i]] = false;
				}
			}
		}
	}

	bool done_selecting = true;
	for(int i=0; i < app.eng.number_of_players; i++){
		if(!confirmed_selection[i]){
			done_selecting = false;
			break;
		}
	}

	if(done_selecting){
		for(int i=0; i < app.eng.number_of_players; i++){
			app.eng.gd.type_selections[i] = current_selection[i]+1;
			sequence_over = true;
		}
		return;
	}

	for(int i=0; i < app.eng.number_of_players; i++){
		for(int j=0; j < 4; j++){
			string sprite_type = "";
			switch(j){
			case 0:
				sprite_type = "flame_tank";
				break;
			case 1:
				sprite_type = "rock_tank";
				break;
			case 2:
				sprite_type = "shock_tank";
				break;
			case 3:
				sprite_type = "hydro_tank";
				break;
			}

			int xpos = (i * 120) + 30;
			int ypos = (j * 100) + 150;

			if(j == current_selection[i]){
				if(!confirmed_selection[i]){
					app.app_windows[0]->driver->draw2DRectangle(
						irr::core::rect<irr::s32>(xpos - 10, ypos - 10, xpos + 90, ypos + 90), 
						irr::video::SColor(255, 255,255,255),
						irr::video::SColor(255, 255,255,255),
						irr::video::SColor(255, 255,255,255),
						irr::video::SColor(255, 255,255,255),
						0);
				}else{
					app.app_windows[0]->driver->draw2DRectangle(
						irr::core::rect<irr::s32>(xpos - 10, ypos - 10, xpos + 90, ypos + 90), 
						irr::video::SColor(255, 255,0,0),
						irr::video::SColor(255, 255,0,0),
						irr::video::SColor(255, 255,0,0),
						irr::video::SColor(255, 255,0,0),
						0);
				}
			}

			map<string, sprite_class> &sprite_classes = app.app_windows[0]->sprite_classes;

			map<string, sprite_class>::iterator this_sprite_class = sprite_classes.find(sprite_type);
			if(this_sprite_class != sprite_classes.end()){
				int layer_num = -1;
				for(vector<sprite_layer>::iterator slitr = this_sprite_class->second.layers.begin(); slitr != this_sprite_class->second.layers.end(); slitr++){
					layer_num++;
					int active_frame = 0;
					irr::video::ITexture *sprite_texture = slitr->images[active_frame];

					int object_width = 80;
					int object_height = 80;

					int object_center_x = xpos + object_width/2;
					int object_center_y = ypos + object_height/2;

					app.eng.draw2DImage(app.app_windows[0]->driver, 
						sprite_texture,  
						irr::core::rect<irr::s32>(0,0, object_width, object_height),  // source rect
						irr::core::position2d<irr::s32>(xpos,ypos),  // position
						irr::core::position2d<irr::s32>(object_center_x, object_center_y),  // rotation point
						0, 
						irr::core::position2df(1,1), 
						true, 
						irr::video::SColor(255,255,255,255),
						irr::core::rect<irr::s32>(0,0,app.app_windows[0]->client_w,app.app_windows[0]->client_h)
						);
				}
			}

		}
	}
}

main_sequence::main_sequence(int seq_num) : game_sequence(seq_num){
	c_response_ticker = 0;
}

void main_sequence::render(void){
	c_response_ticker = 0;
		
	app.eng.inp.handle_inputs();
	app.eng.animagic();
	app.eng.center_cams();
	vector<shared_ptr<window> >::iterator itr;
	for(itr = app.app_windows.begin(); itr != app.app_windows.end(); itr++){
		shared_ptr<window> this_window(*itr);
		this_window->cams.render();
	}
}
