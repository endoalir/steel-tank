#include "StdAfx.h"
#include "steel_tank.h"
#include "camera_set.h"

using boost::shared_ptr;

camera_set::camera_set(){
	num_cols = 2;
	host_width = 0, host_height = 0;
	border_space = 10;
	camera_count = 0;
}
camera_set::~camera_set(void){
	cameras.clear();
}

unsigned int camera_set::make_camera(){
	unsigned int camnum = cameras.size();

	shared_ptr<camera> this_cam(new camera());
	this_cam->driver = this_window->driver;
	this_cam->init_camera();
	this_cam->w = 0;
	this_cam->h = 0;
	this_cam->cam_num = camnum;
	this_cam->this_window = this_window;

	cameras.push_back(this_cam);

	reposition();

	this_window->populate_camera_menu();

	return camnum;
}

void camera_set::renumber_cameras(void){
	int c_num = 0;
	camera_count = 0;
	for(auto cami = cameras.begin(); cami != cameras.end(); cami++){
		(*cami)->cam_num = c_num++;
	}
	camera_count = c_num;
}

void camera_set::move_camera(int which_camera, int to_window){
	unsigned int uiwc = which_camera;
	if(uiwc > cameras.size()-1 || cameras.size() == 0){
		return;
	}
	unsigned int uitw = to_window;
	if(uitw > app.app_windows.size()-1 || app.app_windows.size() == 0){
		return;
	}

	shared_ptr<camera> referenced_camera = cameras[which_camera];

	vector<shared_ptr<camera> >::iterator i;
	for(i = cameras.begin(); i!= cameras.end();){
		if(*i == referenced_camera){
			cameras.erase(i);
			break;
		}
		i++;
	}

	referenced_camera->this_window = app.app_windows[to_window];
	referenced_camera->driver = app.app_windows[to_window]->driver;

	app.app_windows[to_window]->cams.cameras.push_back(referenced_camera);

}

void camera_set::remove_camera(void){}
void camera_set::reposition(void){
	unsigned int total_cameras = cameras.size();
	if(total_cameras <= 0){return;}

	int cami = 0;
	int which_col = 0, which_row = 0;
	int total_rows = (total_cameras / num_cols) + (total_cameras % num_cols);

	
	host_width = this_window->client_w;
	host_height = this_window->client_h;

	int cam_widths = (host_width / num_cols);
	int cam_heights = (host_height / total_rows);

	vector<shared_ptr<camera> >::iterator itr;
	shared_ptr<camera> last_camera;
	for(itr = cameras.begin(); itr != cameras.end(); itr++){
		shared_ptr<camera> this_cam = *itr;
		last_camera = this_cam;

		which_col = cami % num_cols;

		this_cam->sx = which_col * cam_widths;
		this_cam->sy = which_row * cam_heights;
		this_cam->w = cam_widths - border_space;
		this_cam->h = cam_heights - border_space;

		if(which_row+1 == total_rows)
			this_cam->h += border_space;
		if(which_col+1 == num_cols)
			this_cam->w += border_space;

		if(which_col == num_cols-1)
			which_row++;
		cami++;
	}
	
	if(cami > 0){

		int total_slots = total_rows * num_cols;
		int remaining_slots = total_slots - cami;
		if(remaining_slots > 0){
			shared_ptr<camera> this_cam = last_camera;

			this_cam->w = cam_widths * (remaining_slots+1) + (border_space);
		}
	}

	center();
}

void camera_set::render(void){
	vector<shared_ptr<camera> >::iterator itr;
	for(itr = cameras.begin(); itr != cameras.end(); itr++){
		shared_ptr<camera> this_cam = *itr;
		this_cam->render();
	}
}

void camera_set::center(void){
	vector<shared_ptr<camera> >::iterator itr;
	for(itr = cameras.begin(); itr != cameras.end(); itr++){
		shared_ptr<camera> this_cam = *itr;
		if(this_cam->centered_object.get())
			this_cam->center_on_object(this_cam->centered_object); 
	}
}

void camera_set::clear(void){
	cameras.clear();
}

void camera_set::set_centered_object(unsigned int cam_num, shared_ptr<map_object> this_object){
	if(cam_num < cameras.size()){
		cameras[cam_num]->centered_object = this_object;
	}
}
