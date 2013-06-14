#pragma once

using std::vector;
using boost::shared_ptr;

class window;

class camera_set{
public:
	friend class window; // window can access cameras

	camera_set();
	~camera_set(void);

	unsigned int make_camera(void);
	void remove_camera(void);
	void reposition(void);
	void render(void);
	void center(void);
	void clear(void);

	unsigned int camera_count;

	void renumber_cameras(void);

	void set_centered_object(unsigned int, shared_ptr<map_object>);

	int host_width, host_height;
	int border_space;

	boost::shared_ptr<window> this_window;

	void move_camera(int which_camera, int to_window);

private:
	vector<shared_ptr<camera> > cameras;
	int num_cols;

};
