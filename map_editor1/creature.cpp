#include "StdAfx.h"
#include "map_editor.h"

creature::creature() {
	speed = 0;
	health = 100;
	explosion_step = -1;
	explosion_ticker = 0;
	creature_type = TYPE_VANILLA;
}
creature::~creature(){}

void creature::move_forward(void){
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

	bool would_be_collided = false;

	for(vector<boost::shared_ptr<map_object> >::iterator mo_itr = app.eng.gd.map_objects.begin(); mo_itr != app.eng.gd.map_objects.end(); mo_itr++){
		if((*mo_itr).get() != this && (*mo_itr)->is_solid){
			if((*mo_itr)->is_within(new_x, new_y) || (*mo_itr)->is_within(new_x, new_y1)
				|| (*mo_itr)->is_within(new_x1, new_y) || (*mo_itr)->is_within(new_x1, new_y1)){
				would_be_collided = true;
				break;
			}
		}
	}

	if(would_be_collided)
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
	if(explosion_step < 0){
		if(creature_type == TYPE_FLAME && hitby.type == TYPE_HYDRO){
			hitby.power *= 2;
		}else if(creature_type == TYPE_SHOCK && hitby.type == TYPE_FLAME){
			hitby.power *= 2;
		}else if(creature_type == TYPE_ROCK && hitby.type == TYPE_SHOCK){
			hitby.power *= 2;
		}else if(creature_type == TYPE_HYDRO && hitby.type == TYPE_ROCK){
			hitby.power *= 2;
		}

		health -= hitby.power;
	}
	if(health <= 0 && explosion_step < 0){
		explosion_step = 0;
	}
}

void creature::step_explosion(void){
	if(explosion_step < 0 ){return;}

	explosion_ticker++;

	if(explosion_ticker > 10){ explosion_step++; explosion_ticker = 0; }

	if(explosion_step < app.eng.number_of_explosion_steps){
		overlay_graphic = app.eng.explosion_steps[explosion_step];
	}else{
		overlay_graphic = 0;
		to_remove = true;
	}
}

void creature::fire_projectile(){
	if(last_fire_time > 0){
		if( (clock() - last_fire_time) < 800){
			return;
		}
	}
	
	int speed = 8;
	bullet newbullet;

	newbullet.type = creature_type;
	switch(creature_type){
	case TYPE_FLAME:
		newbullet.img = app.eng.flame_bullet;
		break;
	case TYPE_ROCK:
		newbullet.img = app.eng.rock_bullet;
		break;
	case TYPE_SHOCK:
		newbullet.img = app.eng.shock_bullet;
		break;
	case TYPE_HYDRO:
		newbullet.img = app.eng.hydro_bullet;
		break;
	default:
		newbullet.img = app.eng.bulletimg;
	}

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
