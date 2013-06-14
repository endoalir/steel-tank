#pragma once

using std::map;

class input_control {
public:
	input_control();
	~input_control();

	int mouse_xpos;
	int mouse_ypos;

	map<int,bool> keys_down;
	
	void handle_inputs(void);

	int get_relative_angle(int x, int y, int x1, int y1);
	double get_distance(int x, int y, int x1, int y1);

	int rb_org_camx;
	int rb_org_camy;
	int rb_org_mx;
	int rb_org_my;
};

