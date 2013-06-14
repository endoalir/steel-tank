#include "stdafx.h"
#include "map_editor.h"

input_control::input_control(){
	rb_org_camx=-1;
	rb_org_camy=-1;
	rb_org_mx=-1;
	rb_org_my=-1;
}
input_control::~input_control(){}

void input_control::handle_inputs(){
	if(app.eng.gd.players.size() > 0){

	if(keys_down[VK_UP]){
		app.eng.gd.players[0]->speed = 5;
		app.eng.gd.players[0]->move_forward();
		if(app.eng.gd.players[0]->active_layer_frames[0] == 0){
			app.eng.gd.players[0]->active_layer_frames[0] = 1;
		}else{
			app.eng.gd.players[0]->active_layer_frames[0] = 0;
		}
	}
	if(keys_down[VK_DOWN]){
		app.eng.gd.players[0]->speed = -5;
		app.eng.gd.players[0]->move_forward();
		if(app.eng.gd.players[0]->active_layer_frames[0] == 0){
			app.eng.gd.players[0]->active_layer_frames[0] = 1;
		}else{
			app.eng.gd.players[0]->active_layer_frames[0] = 0;
		}

	}
	if(keys_down[VK_LEFT]){
		app.eng.gd.players[0]->sticky_rotate_object_by(-3);
	}
	if(keys_down[VK_RIGHT]){
		app.eng.gd.players[0]->sticky_rotate_object_by(3);
	}

	// get the center position of the sprite
	//int p_x =  (app.eng.gd.players[0]->x + ((app.eng.gd.players[0]->x1 - app.eng.gd.players[0]->x)/2)) - app.eng.camera_x;
	//int p_y = app.client_h + ( (app.eng.camera_y - app.eng.gd.players[0]->y) - ((app.eng.gd.players[0]->y1 - app.eng.gd.players[0]->y)/2));

	// rotate layer 1 (barrel) to angle relative to mouse position
	//app.eng.gd.players[0]->rotate_layer(1, get_relative_angle(p_x, p_y, mouse_xpos, mouse_ypos));

	if(app.eng.js.number_of_joysticks > 0){
		int joy_x = app.eng.js.joystates[0].lX;
		int joy_y = app.eng.js.joystates[0].lY;
		int joy_z = app.eng.js.joystates[0].lZ;

		int joy_rx = app.eng.js.joystates[0].lRx;
		int joy_ry = app.eng.js.joystates[0].lRy;

		int a_angle = get_relative_angle(32767, 32767, joy_x, joy_y);
		int b_angle = get_relative_angle(32767, 32767, joy_rx, joy_ry);

		bool x_is_centered = joy_x > 26000 && joy_x < 38000;
		//bool x_is_centered = joy_x > 10000 && joy_x < 60000;
		bool y_is_centered = joy_y > 26000 && joy_y < 38000;
		bool z_is_centered = joy_z > 26000 && joy_z < 38000;

		bool rx_is_centered = joy_rx > 26000 && joy_rx < 38000;
		bool ry_is_centered = joy_ry > 26000 && joy_ry < 38000;

		bool z_up = joy_z >= 38000;
		bool z_down = joy_z <= 26000;

		bool is_moving = false;

		static bool auto_pilot = false;
		static bool a_was_pressed = false;
		if(app.eng.js.joystates[0].rgbButtons[0]){
			if(!a_was_pressed){
				a_was_pressed = true;
				auto_pilot = !auto_pilot;
			}
		}else{
			a_was_pressed = false;
		}

		if(!z_is_centered && !auto_pilot){
			is_moving = true;

			if(z_up){
				float z_perc = ((float)joy_z - 32767) / 32767;
				app.eng.gd.players[0]->speed = (int)ceil(z_perc * -5);
				app.eng.gd.players[0]->move_forward();
			}else if(z_down){
				float z_perc = 1 - ((float)joy_z / 32767);
				app.eng.gd.players[0]->speed = (int)ceil(z_perc * 5);
				app.eng.gd.players[0]->move_forward();
			}
		}

		if(auto_pilot){
			if(app.eng.gd.players[0]->speed != 0)
				is_moving = true;
			app.eng.gd.players[0]->move_forward();
		}


		if(!x_is_centered || !y_is_centered){
			bool x_left = joy_x < 26000;
			bool x_right = joy_x > 40000;
			bool y_up = joy_y < 26000;
			bool y_down = joy_y > 40000;

			int calculated_speed = 0;

			float x_perc = 0;
			float y_perc = 0;

			if(x_left){
				x_perc = 1 - ((float)joy_x / 32767);
			}
			if(x_right){
				x_perc = ((float)joy_x - 32767) / 32767;
			}
			if(y_up){
				y_perc = 1 - ((float)joy_y / 32767);
			}
			if(y_down){
				y_perc = ((float)joy_y - 32767) / 32767;
			}

			if(y_up){
				//app.eng.gd.players[0]->speed = (int)ceil(5 * y_perc);
				//app.eng.gd.players[0]->move_forward();
				if(app.eng.gd.players[0]->active_layer_frames[0] == 0){
					app.eng.gd.players[0]->active_layer_frames[0] = 1;
				}else{
					app.eng.gd.players[0]->active_layer_frames[0] = 0;
				}
			}

			if(y_down){
				//app.eng.gd.players[0]->speed = (int)floor(-5 * y_perc);
				//app.eng.gd.players[0]->move_forward();
				if(app.eng.gd.players[0]->active_layer_frames[0] == 0){
					app.eng.gd.players[0]->active_layer_frames[0] = 1;
				}else{
					app.eng.gd.players[0]->active_layer_frames[0] = 0;
				}
			}

			if(x_left){
				//app.eng.gd.players[0]->sticky_rotate_object_by(-3);
			}

			if(x_right){
				//app.eng.gd.players[0]->sticky_rotate_object_by(3);
			}

			if(is_moving){
				int change_amt = a_angle - app.eng.gd.players[0]->rotations[0];
				if((change_amt > -5 && change_amt < 5)){change_amt = 0;}
				if(change_amt > 180) { change_amt = change_amt - 360; }
				if(change_amt < -180) { change_amt = 360 + change_amt; }
				if(change_amt > 5){ change_amt = 5; }
				if(change_amt < -5){ change_amt = -5; }
				

				app.eng.gd.players[0]->rotate_object_by(change_amt);
			}
		}else{ // if(!x_is_centered || !y_is_centered){
			if(is_moving){
				bool pov_press = false;
				if(app.eng.js.joystates[0].rgdwPOV[0] < 4200000000){
					pov_press = true;
					a_angle = app.eng.js.joystates[0].rgdwPOV[0] / 100;
				}

				if(pov_press){
					int change_amt = a_angle - app.eng.gd.players[0]->rotations[0];
					if(change_amt > 180) { change_amt = change_amt - 360; }
					if(change_amt < -180) { change_amt = 360 + change_amt; }
					if(change_amt > 5){ change_amt = 5; }
					if(change_amt < -5){ change_amt = -5; }

					app.eng.gd.players[0]->rotate_object_by(change_amt);
				}

			}
		}

		if(app.eng.js.joystates[0].rgbButtons[5]){
			//app.eng.gd.players[0]->speed = 5;
			//app.eng.gd.players[0]->move_forward();
		}

		if(!rx_is_centered || !ry_is_centered){
			int change_amt = b_angle - app.eng.gd.players[0]->rotations[1];
			if((change_amt > -5 && change_amt < 5)){change_amt = 0;}
			if(change_amt > 180) { change_amt = change_amt - 360; }
			if(change_amt < -180) { change_amt = 360 + change_amt; }
			if(change_amt > 5){ change_amt = 5; }
			if(change_amt < -5){ change_amt = -5; }

			app.eng.gd.players[0]->rotate_layer_by(1, change_amt);
		}

		if(app.eng.js.joystates[0].rgbButtons[3]){
			static clock_t last_switch = clock();
			if((clock() - last_switch) >300){
				last_switch = clock();

				vector<boost::shared_ptr<creature> >::iterator first_enemy = app.eng.gd.enemies.begin();
				vector<boost::shared_ptr<creature> >::iterator first_player = app.eng.gd.players.begin();

				boost::shared_ptr<creature> enemy_creature = *first_enemy;
				boost::shared_ptr<creature> player_creature = *first_player;

				app.eng.gd.enemies.erase(first_enemy);
				app.eng.gd.players.erase(first_player);

				app.eng.gd.players.push_back(enemy_creature);
				app.eng.gd.enemies.push_back(player_creature);
			}
		}

		if(app.eng.js.joystates[0].rgbButtons[4]){
			app.eng.gd.players[0]->fire_projectile();
		}

	} // if(app.eng.js.number_of_joysticks > 0){

	} // if(app.eng.gd.players.size() > 0){

	if(0||app.eng.rbutton){
		if(-1 == rb_org_camx){
			rb_org_camx = app.eng.camera_x;
			rb_org_camy = app.eng.camera_y;
			rb_org_mx = mouse_xpos;
			rb_org_my = mouse_ypos;
		}else{
			app.eng.camera_x = rb_org_camx - (mouse_xpos - rb_org_mx);
			if(app.eng.camera_x <=0 ){ app.eng.camera_x = 0; }
			app.eng.camera_y = rb_org_camy + (mouse_ypos - rb_org_my);
			if(app.eng.camera_y <=0 ){ app.eng.camera_y = 0; }
		}
	}else{
		rb_org_camx=-1, rb_org_camy=-1;
		rb_org_mx=-1, rb_org_my=-1;
	}

	if(app.eng.lbutton){
		//app.eng.player_fire();
	}
}

int input_control::get_relative_angle(int x, int y, int x1, int y1){
	int out = 0;

	float h_distance = 0;
	float v_distance = 0;
	double rad_to_deg = 57.2957795;
		
	if(x1 > x && y1 < y){ // quadrant 2
		h_distance = (float)(x1 - x);
		v_distance = (float)(y - y1);
		out =  (int)(atan(h_distance / v_distance)* rad_to_deg);
	}else if(x1 > x && y1 >= y){ // quadrant 3
		h_distance = (float)(x1 - x);
		v_distance = (float)(y1 - y);
		out = (int)((atan(v_distance / h_distance)* rad_to_deg) + 90);
	}else if(x1 <= x && y1 >= y){ // quadrant 4
		h_distance = (float)(x - x1);
		v_distance = (float)(y1 - y);
		out = (int)((atan(h_distance / v_distance)* rad_to_deg) + 180);
	}else{ // quadrant 1
		h_distance = (float)(x - x1);
		v_distance = (float)(y - y1);
		out = (int)((atan(v_distance / h_distance)* rad_to_deg) + 270);
	}

	return out;
}
