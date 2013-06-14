#include "StdAfx.h"
#include "steel_tank.h"

creature::creature() {
	speed = 0;
	health = 100;
	explosion_step = -1;
	explosion_ticker = 0;
	creature_type = TYPE_VANILLA;

	automaton_step = 0;
	automaton_dir = 1;
	automaton_startstep = (int)floor(((double)rand() / (double)RAND_MAX) * 100);
	automaton_newdir = -1;
	automaton_resetdir = true;
	last_fire_time = 0;
}
creature::~creature(){}

void creature::move_forward(void){
	if(explosion_step > -1){return;}

	int player_rotation = rotations[0];
	int player_inertiay = speed;

	int new_x = x;
	int new_y = y;
	int new_x1 = x1;
	int new_y1 = y1;

	int sz_x = x1-x;
	int sz_y = y-y1;

	if(player_rotation < 90){
		int top_angle = 90 - player_rotation;
		float rel_top_perc = (float)top_angle / 90;
		float rel_bottom_perc = 1-rel_top_perc;

		int y_move = (int)ceil((float)player_inertiay * (float)rel_top_perc);
		int x_move = (int)ceil((float)player_inertiay * (float)rel_bottom_perc);

		new_y += y_move;
		new_x += x_move;

	}else if(player_rotation == 90){
		new_x += player_inertiay;
	}else if(player_rotation < 180){
		int top_angle = player_rotation - 90;
		float rel_top_perc = (float)top_angle / 90;
		float rel_bottom_perc = 1-rel_top_perc;

		int y_move = (int)ceil((float)player_inertiay * (float)rel_top_perc);
		int x_move = (int)ceil((float)player_inertiay * (float)rel_bottom_perc);

		new_y -= y_move;
		new_x += x_move;
	}else if(player_rotation == 180){
		new_y -= player_inertiay;
	}else if(player_rotation < 270){
		int top_angle = 90- (player_rotation - 180);
		float rel_top_perc = (float)top_angle / 90;
		float rel_bottom_perc = 1-rel_top_perc;

		int y_move = (int)ceil((float)player_inertiay * (float)rel_top_perc);
		int x_move = (int)ceil((float)player_inertiay * (float)rel_bottom_perc);

		new_y -= y_move;
		new_x += -1*x_move;
	}else if(player_rotation == 270){
		new_x -= player_inertiay;
	}else{ // > 270
		int top_angle = 90- (player_rotation - 270);
		float rel_top_perc = (float)top_angle / 90;
		float rel_bottom_perc = 1-rel_top_perc;

		int y_move = (int)ceil((float)player_inertiay * (float)rel_bottom_perc);
		int x_move = (int)ceil((float)player_inertiay * (float)rel_top_perc);

		new_y += y_move;
		new_x += -1*x_move;
	}

	new_x1 = new_x + sz_x;
	new_y1 = new_y - sz_y;

	bool would_be_collided = app.eng.check_collision(new_x, new_y, new_x1, new_y1, this);
	if(would_be_collided && !(is_player() && app.eng.clipless))
		return;


	int map_width = app.eng.map_width;
	int map_height = app.eng.map_height;

	if(new_x < 0 || new_y < 0 || new_x1 < 0 || new_y1 < 0 ||
		new_x > map_width || new_x1 > map_width || new_y > map_height || new_y1 > map_height){
			return;
	}

	x = new_x, x1=new_x1;
	y = new_y, y1=new_y1;
}

void creature::hit_by_projectile(bullet hitby){
	if(app.eng.invincible_mode){
		if(is_player())
			return;

		to_remove = true;
		return;
	}

	if(app.eng.super_player){
		if(is_player())
			hitby.power /= 4;
		else
			hitby.power *= 4;
	}

	if(app.eng.epicplayer){
		if(is_player())
			hitby.power *= 2;
		else
			hitby.power /= 2;
	}

	bool super_hit = false;

	if(explosion_step < 0){
		if(creature_type == TYPE_FLAME && hitby.type == TYPE_HYDRO){
			super_hit = true;
			hitby.power *= 2;
		}else if(creature_type == TYPE_SHOCK && hitby.type == TYPE_FLAME){
			super_hit = true;
			hitby.power *= 2;
		}else if(creature_type == TYPE_ROCK && hitby.type == TYPE_SHOCK){
			super_hit = true;
			hitby.power *= 2;
		}else if(creature_type == TYPE_HYDRO && hitby.type == TYPE_ROCK){
			super_hit = true;
			hitby.power *= 2;
		}

		health -= hitby.power;
	}

	if(health <= 0 && explosion_step < 0){
		app.eng.snd.playfile("../stock/explode.ogg");
		explosion_step = 0;
	}else{

		if(!super_hit){
			app.eng.snd.playfile("../stock/smallhit.ogg");
		}else{
			app.eng.snd.playfile("../stock/bighit.ogg");
		}
	}
}

bool creature::is_player(void){
	vector<boost::shared_ptr<creature> >::iterator itr;
	for(itr = app.eng.gd.players.begin(); itr != app.eng.gd.players.end(); itr++){
		if(this == itr->get())
			return true;
	}
	return false;
}

void creature::step_explosion(void){
	if(explosion_step < 0 ){return;}

	explosion_ticker++;

	if(explosion_ticker > 10){ explosion_step++; explosion_ticker = 0; }

	if(explosion_step < app.app_windows[0]->number_of_explosion_steps){
		//overlay_graphic = app.eng.explosion_steps[explosion_step];
	}else{
		overlay_graphic = 0;
		to_remove = true;
	}
}

void creature::fire_projectile(){
	if(explosion_step > -1){return;}

	if(last_fire_time > 0){
		clock_t now_clock = clock();
		clock_t elapsed_time = now_clock - last_fire_time;
		if( elapsed_time < 800){
			return;
		}
	}

	app.eng.snd.playfile("../stock/fire.ogg");
	
	int speed = 8;
	bullet newbullet;

	newbullet.type = creature_type;

	int angle = rotations[1];
	int relx = x;
	int rely = y;

	if(angle < 90){
		int top_angle = 90 - angle;
		float rel_top_perc = (float)top_angle / 90;
		float rel_bottom_perc = 1-rel_top_perc;

		int y_move = (int)(ceil((float)speed * (float)rel_top_perc));
		int x_move = (int)(ceil((float)speed * (float)rel_bottom_perc));

		newbullet.inertia_y += y_move;
		newbullet.inertia_x += x_move;

	}else if(angle == 90){
		newbullet.inertia_x = speed;
	}else if(angle < 180){
		int top_angle = angle - 90;
		float rel_top_perc = (float)top_angle / 90;
		float rel_bottom_perc = 1-rel_top_perc;

		int y_move = (int)(ceil((float)speed * (float)rel_top_perc));
		int x_move = (int)(ceil((float)speed * (float)rel_bottom_perc));

		newbullet.inertia_y += -1*y_move;
		newbullet.inertia_x += x_move;
	}else if(angle == 180){
		newbullet.inertia_y = -speed;
	}else if(angle < 270){
		int top_angle = 90- (angle - 180);
		float rel_top_perc = (float)top_angle / 90;
		float rel_bottom_perc = 1-rel_top_perc;

		int y_move = (int)(ceil((float)speed * (float)rel_top_perc));
		int x_move = (int)(ceil((float)speed * (float)rel_bottom_perc));

		newbullet.inertia_y += -1*y_move;
		newbullet.inertia_x += -1*x_move;
	}else if(angle == 270){
		newbullet.inertia_x = -speed;
	}else{ // > 270
		int top_angle = 90- (angle - 270);
		float rel_top_perc = (float)top_angle / 90;
		float rel_bottom_perc = 1-rel_top_perc;

		int y_move = (int)(ceil((float)speed * (float)rel_bottom_perc));
		int x_move = (int)(ceil((float)speed * (float)rel_top_perc));

		newbullet.inertia_y += y_move;
		newbullet.inertia_x += -1*x_move;
	}

	newbullet.x = relx + 35;
	newbullet.y = rely - 35;

	last_fire_time = clock();

	bullets.push_back(newbullet);
}

void creature::botomatic(){
	automaton_step += automaton_dir;
	if(automaton_step >= 100){
		automaton_dir = -1;
	}
	if(automaton_step <= 0){
		automaton_dir = 1;
	}

	if(automaton_startstep > -1){
		if(automaton_step < automaton_startstep)
			return;

		automaton_startstep = -1;
		automaton_step = 0;
		automaton_dir = 1;
	}

	double closest_distance = -1;
	double angle_to_player = -1;

	boost::shared_ptr<creature> closest_player;
	double the_dist = 0;

	vector<boost::shared_ptr<creature> >::iterator playeritr;
	for(playeritr = app.eng.gd.players.begin(); playeritr != app.eng.gd.players.end(); playeritr++){
		boost::shared_ptr<creature> this_player;
		this_player = boost::shared_ptr<creature>(*playeritr);

		if(this_player->to_remove){
			continue;
		}

		the_dist = app.eng.inp.get_distance(this_player->x, this_player->y, x, y);
		if(closest_distance <= 0){
			closest_player = this_player; 
			closest_distance = the_dist;
		}

		if(the_dist > -1){
			if(the_dist < closest_distance){
				closest_distance = the_dist;
				closest_player = this_player;
			}
		}
	}

	if(closest_distance > -1){
		angle_to_player = 360 - app.eng.inp.get_relative_angle(closest_player->x, closest_player->y, x, y);
	}

	if(automaton_dir == 1){
		if(closest_distance > -1 && closest_distance < 25){
			double change_amt = rotations[1] - angle_to_player;
			if((change_amt > -5 && change_amt < 5)){change_amt = 0;}
			if(change_amt > 180) { change_amt = change_amt - 360; }
			if(change_amt < -180) { change_amt = 360 + change_amt; }
			bool close_enough = true;
			if(change_amt > 5){ change_amt = -4; close_enough = false; }
			if(change_amt < -5){ change_amt = 4; close_enough = false;}

			if(!close_enough){
				rotate_layer_by(1,(int)change_amt);
			}else{
				fire_projectile();
			}
		}else{
			if(automaton_resetdir){
				automaton_resetdir = false;
				automaton_newdir = (int)(((double)rand() / (double)RAND_MAX)*360);
			}

			if(rotations[0] != automaton_newdir){
				int change_amt = automaton_newdir - rotations[0];
				if((change_amt > -5 && change_amt < 5)){change_amt = 0;}
				if(change_amt > 180) { change_amt = change_amt - 360; }
				if(change_amt < -180) { change_amt = 360 + change_amt; }
				if(change_amt > 5){ change_amt = 5; }
				if(change_amt < -5){ change_amt = -5; }

				rotate_object_by(change_amt);
			}else{
				speed = 2;

// mass movement takes too long for debug mode, so this until I get an rtree in place
#ifndef _DEBUG
				move_forward();
#endif
			}
		}
	}else{
		automaton_resetdir = true;
		speed = 2;
#ifndef _DEBUG
			move_forward();
#endif
	}

}