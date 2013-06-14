#pragma once

class bullet;

class creature : public map_object {
public:
	creature();
	~creature();

	int creature_type;

	int speed;
	int health;
	int explosion_ticker;

	void move_forward(void);
	void step_explosion(void);
	void hit_by_projectile(bullet hitby);

	void fire_projectile();
	clock_t last_fire_time;
	vector<bullet> bullets;

	irr::video::ITexture *img;

	int automaton_step;
	int automaton_dir;
	int automaton_startstep;
	int automaton_newdir;
	bool automaton_resetdir;

	void botomatic(void);

	bool is_player(void);
};
