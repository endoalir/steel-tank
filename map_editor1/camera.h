#pragma once
class camera
{
public:
	camera(void);
	~camera(void);

	irr::video::IVideoDriver *driver;

	int x,y; // position of camera on map
	int sx,sy,w,h; // position of camera in window


	bool camera_initialized;
	bool owns_device;

	void init_camera(void);
	void render(void);

	void center_on_object(boost::shared_ptr<map_object> the_object);
};
