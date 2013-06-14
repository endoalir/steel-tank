#pragma once

class window;

class camera{
public:
	friend class camera_set;
	friend class window;

	~camera(void);

private:
	camera(void);

	irr::video::IVideoDriver *driver;

	int x,y; // position of camera on map
	int sx,sy,w,h; // position of camera in window

	boost::shared_ptr<window> this_window;

	bool camera_initialized;
	bool owns_device;

	boost::shared_ptr<map_object> centered_object;

	void init_camera(void);
	void render(void);
	void center_on_object(boost::shared_ptr<map_object> the_object);

	map_data md;
	bool init_center;
	unsigned int cam_num;
};
