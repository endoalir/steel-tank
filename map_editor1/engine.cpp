#include "stdafx.h"
#include "map_editor.h"

engine::engine(){
	prev_time = 0;
	splashtimer = 0;
	insplash = false;
	showfps = true;
	lbutton = false;
	mbutton = false;
	rbutton = false;
	c_x = 0;
	c_y = 0;
	client_width = 0;
	client_height = 0;

	player_ingame = false;

	player_start_sequence = false;
	player_start_sequence_timer = 0;

	last_fire_time = 0;

	camera_x = 0, camera_y = 0;
	map_width = 15000; map_height = 15000;

	player_select_screen = 0;
	number_of_players = 0;

}

engine::~engine(){};

void engine::load(void){
	js.InitDirectInput();

	irr::SIrrlichtCreationParameters param;
	param.DriverType = irr::video::EDT_DIRECT3D9;
	param.WindowId = reinterpret_cast<void*>(app.app_windows["main"]);
	device = irr::createDeviceEx(param);
	if(!device){ return; }
	driver = device->getVideoDriver();
	if(!driver){ return; }
	terrain = driver->getTexture("../stock/terrain1.jpg");
	
	gd.load_map_from_coordinates(md,0,0,0,0);

	driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
	driver->getMaterial2D().AntiAliasing = irr::video::EAAM_FULL_BASIC;
	videodata = irr::video::SExposedVideoData(app.app_windows["mainirr"]);
	smgr = device->getSceneManager();
	font = device->getGUIEnvironment()->getBuiltInFont();

	gd.load_sprite_classes();

	camera this_cam;
	this_cam.driver = driver;
	this_cam.init_camera();

	this_cam.sx = 200;
	this_cam.w = app.client_w - 186;
	this_cam.h = app.client_h+54;

	cam.push_back(this_cam);
	repos_cams();

	sprite_selected = -1;

}

#define LOAD_REGION 500

void engine::runner(int i){
	device->getTimer()->tick();
	unsigned int timerv = (unsigned int)clock();

	static bool lbutton_rb = false;
	static int rb_x = 0;
	static int rb_y = 0;
	static int rb_mx = 0;
	static int rb_my = 0;

	static bool lbutton_clicking = false;

	driver->beginScene(true, true, 0, videodata);


	if(c_x < 200){
		if(lbutton){
			if(!lbutton_clicking)
				lbutton_clicking = true;
		}else{
			if(lbutton_clicking){
				int whichone = -1;
				int whichfound = -1;
				vector<irr::core::rect<irr::s32> >::iterator itr;
				for(itr = sprite_buttons.begin(); itr!=sprite_buttons.end(); itr++){
					whichone++;
					if(itr->isPointInside(irr::core::vector2d<irr::s32>(c_x, c_y))){
						whichfound = whichone;
						break;
					}
				}

				sprite_selected = whichfound;
			}

			lbutton_clicking = false;
		}
	}

	if(sprite_selected > -1){
		driver->draw2DRectangle(irr::video::SColor(255,255,255,255), sprite_buttons[sprite_selected]);
	}

	for(vector<camera>::iterator camitr = cam.begin(); camitr != cam.end(); camitr++){
		camitr->render();
	}

	int sticky_x = 80;
	int sticky_y = 80;

	if(c_x > 200){
		if(lbutton){
			if(!lbutton_clicking)
				lbutton_clicking = true;
		}else{
			int place_x_at =  (c_x) - (c_x % sticky_x) - (cam[0].x % sticky_x) - (sticky_x /2);
			int place_y_at = c_y - (c_y % sticky_y) + (cam[0].y % sticky_y);

			int rel_x = c_x + cam[0].x;
			int rel_y = cam[0].y - c_y;

			int p_rel_x = place_x_at + cam[0].x;
			int p_rel_y = cam[0].y - place_y_at;

			int prtc_x = rel_x - cam[0].sx; // Position Relative to Camera
			int prtc_y = cam[0].sy+cam[0].h+rel_y;

			int p_prtc_x = p_rel_x - cam[0].sx; 
			int p_prtc_y = cam[0].sy+cam[0].h+p_rel_y;

			if(lbutton_clicking){
				if(sprite_selected == -1){
					for(vector<boost::shared_ptr<map_object> >::iterator itr = app.eng.gd.map_objects.begin(); itr != app.eng.gd.map_objects.end();itr++){
						irr::core::rect<irr::s32> mo_rect((*itr)->x, (*itr)->y1, (*itr)->x1, (*itr)->y);
						if(mo_rect.isPointInside(irr::core::vector2d<irr::s32>(prtc_x, prtc_y))){
							(*itr)->to_remove = true;
						}
					}
				}else{

					string found_sprite_class = "";
					map<string, sprite_class>::iterator sc_itr;
					int cnt = -1;
					for(sc_itr = gd.sprite_classes.begin(); sc_itr != gd.sprite_classes.end(); sc_itr++){
						if(++cnt == sprite_selected){
							found_sprite_class = sc_itr->first;
							break;
						}
					}


					boost::shared_ptr<map_object> new_object(new map_object());
					new_object->sprite_class = found_sprite_class;
					new_object->object_name = "";
					new_object->x = p_prtc_x;
					new_object->y = p_prtc_y;
					new_object->x1 = new_object->x+80;
					new_object->y1 = new_object->y-80;

					for(int i=0; i<50; i++){
						new_object->active_layer_frames.push_back(0);
						new_object->rotations.push_back(0);
					}

					md.map_objects.push_back(new_object);
				}

			}
			lbutton_clicking = false;
		}

		if(rbutton){
			if(!lbutton_rb){
				lbutton_rb = true;
				rb_x = cam[0].x;
				rb_y = cam[0].y;
				rb_mx = c_x;
				rb_my = c_y;
			}else{
				cam[0].x = rb_x - (c_x - rb_mx);
				cam[0].y = (c_y - rb_my) + rb_y;

				int cam_width = cam[0].w;
				int cam_height =cam[0].h;

				int avg_px = 0;
				int avg_py = 0;

				int total_px = 0;
				int total_py = 0;
				int total_players = 0;

				int x = cam[0].x - (cam_width/2);
				int y = cam[0].y - (cam_height/2);

				if(x < 0){x = 0;}
				if(y < 0){y = 0;}

				int center_x = x;
				int center_y = y;

				if(cam_width > 0 && cam_height > 0){
					if((center_x - md.c_x) > cam[0].w/2 || (center_y - md.c_y) > cam[0].h/2 || (md.c_x - center_x) > cam[0].w/2 || (md.c_y - center_y) > cam[0].h/2){
			#ifdef _DEBUG
						static int map_load_count = 0;
						string obj_count = boost::lexical_cast<string>((int)md.map_objects.size());
						string out_str = "Map load " + boost::lexical_cast<string>(map_load_count++) + " map objects: " + obj_count + "\n";
						::OutputDebugStringA(out_str.data());
			#endif

						//app.eng.gd.cull_map(md, md.loaded_x, md.loaded_y, md.loaded_x1, md.loaded_y1);

						int l_x = x - (cam[0].w+LOAD_REGION);
						int l_y = y - (cam[0].h+LOAD_REGION);
						int l_x1 = x + ((2*cam[0].w)+LOAD_REGION);
						int l_y1 = y + ((2*cam[0].h)+LOAD_REGION);

						md.c_x = center_x;
						md.c_y = center_y;
						app.eng.gd.load_map_from_coordinates(md, l_x, l_y, l_x1, l_y1);
						app.eng.gd.cull_map(md, l_x, l_y, l_x1, l_y1);
					}
				}

			}
		}else{
			lbutton_rb = false;
		}

		if(sprite_selected > -1){
			map<string, sprite_class>::iterator sc_itr;
			int cnt = -1;
			for(sc_itr = gd.sprite_classes.begin(); sc_itr != gd.sprite_classes.end(); sc_itr++){
				if(++cnt == sprite_selected){
					int x_at =  (c_x) - (c_x % sticky_x) - (cam[0].x % sticky_x) - (sticky_x /2);
					int y_at = c_y - (c_y % sticky_y) + (cam[0].y % sticky_y);
					int width = 80;
					int height = 80;

					if(sc_itr->second.layers[0].images[0]){
						driver->draw2DImage(sc_itr->second.layers[0].images[0],
							irr::core::vector2d<irr::s32>(x_at, y_at),
							irr::core::rect<irr::s32>(0,0,width,height),
							0,irr::video::SColor(255,255,255,255),true);
					}
				}
			}
		}
	}

	if(cam[0].x < 0){
		irr::core::vector2d<irr::s32> a_point;
		a_point.X = 0 - cam[0].x + cam[0].sx;
		a_point.Y = 0;
		irr::core::vector2d<irr::s32> b_point;
		b_point.X = 0 - cam[0].x + cam[0].sx;
		b_point.Y = cam[0].h;

		driver->draw2DLine(a_point, b_point);
	}

	if(cam[0].y < 0){
		irr::core::vector2d<irr::s32> a_point;
		a_point.X = cam[0].sx;
		a_point.Y = cam[0].y + cam[0].h;
		irr::core::vector2d<irr::s32> b_point;
		b_point.X = cam[0].w + cam[0].sx;
		b_point.Y = cam[0].y + cam[0].h;

		driver->draw2DLine(a_point, b_point);
	}

	if(cam[0].y + cam[0].h > app.eng.map_height){
		irr::core::vector2d<irr::s32> a_point;
		a_point.X = cam[0].sx;
		a_point.Y = cam[0].y + cam[0].h - app.eng.map_height;
		irr::core::vector2d<irr::s32> b_point;
		b_point.X = cam[0].w + cam[0].sx;
		b_point.Y = cam[0].y + cam[0].h - app.eng.map_height;

		driver->draw2DLine(a_point, b_point);
	}

	if(cam[0].x + cam[0].w > app.eng.map_width){
		irr::core::vector2d<irr::s32> a_point;
		a_point.X = app.eng.map_width - cam[0].x + cam[0].w + cam[0].sx;
		a_point.Y = 0;
		irr::core::vector2d<irr::s32> b_point;
		b_point.X = app.eng.map_width - cam[0].x + cam[0].w + cam[0].sx;
		b_point.Y = cam[0].h;

		driver->draw2DLine(a_point, b_point);
	}

	map<string, sprite_class>::iterator sc_itr;

	int cnt = -1;
	int y_at = 10;
	int x_at = 10;
	for(sc_itr = gd.sprite_classes.begin(); sc_itr != gd.sprite_classes.end(); sc_itr++){
		cnt++;

		int width = sc_itr->second.w;
		int height = sc_itr->second.h;

		if(width == 0)
			width = 80;
		if(height == 0){
			height = 80;
		}

		irr::core::rect<irr::s32> btn_rect(x_at, y_at, x_at + width, y_at + height);
		sprite_buttons.push_back(btn_rect);

		if(sc_itr->second.layers[0].images[0]){
			driver->draw2DImage(sc_itr->second.layers[0].images[0],
				irr::core::vector2d<irr::s32>(x_at, y_at),
				irr::core::rect<irr::s32>(0,0,width,height),
				0,irr::video::SColor(255,255,255,255),true);
		}

		if(cnt % 2 == 0){
			x_at = width + 30;
		}else{
			x_at = 10;
			y_at += height + 20;
		}
	}

	smgr->drawAll();
	driver->endScene();
}

void engine::repos_cams(void){
	switch(number_of_players){
	case 1:
		cam[0].w = app.client_w;
		cam[0].h = app.client_h;
		break;
	case 2:
		cam[0].w = app.client_w/2 - 5;
		cam[0].h = app.client_h;
		cam[1].sx = app.client_w/2 + 5;
		cam[1].w = app.client_w/2 - 5;
		cam[1].h = app.client_h;
		break;
	case 3:
		for(int i=0; i<3; i++){
			cam[i].w = app.client_w/2 - 5;
			cam[i].h = app.client_h/2 - 5;
		}
		cam[1].sx = app.client_w/2 + 5;
		cam[2].sy = app.client_h/2 + 5;
		break;
	case 4:
		for(int i=0; i<4; i++){
			cam[i].w = app.client_w/2 - 5;
			cam[i].h = app.client_h/2 - 5;
		}
		cam[1].sx = app.client_w/2 + 5;
		cam[2].sy = app.client_h/2 + 5;
		cam[3].sx = app.client_w/2 + 5;
		cam[3].sy = app.client_h/2 + 5;
		break;
	}
}

void engine::player_fire(void){
	gd.players[0]->fire_projectile();
}

// this function was written by Lonesome Ducky // modified to add clipping
// taken from http://irrlicht.sourceforge.net/forum/viewtopic.php?f=9&t=32388&start=15
void engine::draw2DImage(irr::video::IVideoDriver *driver, irr::video::ITexture* texture , irr::core::rect<irr::s32> sourceRect, irr::core::position2d<irr::s32> position, irr::core::position2d<irr::s32> rotationPoint, irr::f32 rotation, irr::core::vector2df scale, bool useAlphaChannel, irr::video::SColor color, irr::core::rect<irr::s32> clipRect) {
       
    irr::video::SMaterial material;

    // Store and clear the projection matrix
    irr::core::matrix4 oldProjMat = driver->getTransform(irr::video::ETS_PROJECTION);
    driver->setTransform(irr::video::ETS_PROJECTION,irr::core::matrix4());
       
    // Store and clear the view matrix
    irr::core::matrix4 oldViewMat = driver->getTransform(irr::video::ETS_VIEW);
    driver->setTransform(irr::video::ETS_VIEW,irr::core::matrix4());

    // Find the positions of corners
    irr::core::vector2df corner[8];

	float px = (irr::f32)position.X;
	float py = (irr::f32)position.Y;
	float pw = (irr::f32)sourceRect.getWidth();
	float ph = (irr::f32)sourceRect.getHeight();

    corner[0] = irr::core::vector2df(px,py);
	corner[1] = irr::core::vector2df(px + (pw * scale.X)/2,py);
    corner[2] = irr::core::vector2df(px+pw*scale.X,py);
	corner[3] = irr::core::vector2df(px,py+(ph * scale.Y)/2);
	corner[4] = irr::core::vector2df(px + pw*scale.X,py+(ph*scale.Y)/2);
    corner[5] = irr::core::vector2df(px,py+ph*scale.Y);
	corner[6] = irr::core::vector2df(px + (pw*scale.X)/2, py + ph*scale.Y);
    corner[7] = irr::core::vector2df(px+pw*scale.X,py+ph*scale.Y);

    // Rotate corners
    if (rotation != 0.0f)
        for (int x = 0; x < 8; x++)
            corner[x].rotateBy((irr::f32)rotation,irr::core::vector2df((irr::f32)rotationPoint.X, (irr::f32)rotationPoint.Y));

	irr::core::vector2df source_corners[8];
    source_corners[0] = irr::core::vector2df(0,0);
	source_corners[1] = irr::core::vector2df((float)sourceRect.getWidth()/2,0);
    source_corners[2] = irr::core::vector2df((float)sourceRect.getWidth(),0);
	source_corners[3] = irr::core::vector2df(0,(float)sourceRect.getHeight()/2);
	source_corners[4] = irr::core::vector2df((float)sourceRect.getWidth(),(float)sourceRect.getHeight()/2);
    source_corners[5] = irr::core::vector2df(0,(float)sourceRect.getHeight());
	source_corners[6] = irr::core::vector2df((float)sourceRect.getWidth()/2,(float)sourceRect.getHeight());
    source_corners[7] = irr::core::vector2df((float)sourceRect.getWidth(),(float)sourceRect.getHeight());

	// note: this assumes the rotation point is the actually center of the image
	float source_center_x = (float)sourceRect.getWidth()/2;
	float source_center_y = (float)sourceRect.getHeight()/2;

	// because there isn't a true to form clipping method available, I do this cludgy clipping method
	// first, I make the shape an octogon, then I clip the points of the rotated shape
	// then I find out what the source image ought to look like when clipped from those dimensions

	// 1. Find what the rotation the source rect corners would be
    if (rotation != 0.0f)
        for (int x = 0; x < 8; x++)
            source_corners[x].rotateBy((irr::f32)rotation,irr::core::vector2df(source_center_x, source_center_y));

	// 2. Clip the rotated shape
	for(int x=0; x < 8; x++){
		// these translations aren't correct for rotations that aren't vertical or horizontal. need to translate the points on the correct axis
		if(!clipRect.isPointInside(irr::core::vector2d<irr::s32>((irr::s32)source_corners[x].X + position.X, (irr::s32)source_corners[x].Y + position.Y))){
			if((source_corners[x].X + position.X) < clipRect.UpperLeftCorner.X){
				source_corners[x].X = (float)((float)clipRect.UpperLeftCorner.X - (float)position.X);
			}
			if((source_corners[x].X + position.X) > clipRect.LowerRightCorner.X){
				source_corners[x].X = (float)((float)clipRect.LowerRightCorner.X - (float)position.X);
			}
			if((source_corners[x].Y + position.Y) < clipRect.UpperLeftCorner.Y){
				source_corners[x].Y = (float)((float)clipRect.UpperLeftCorner.Y - (float)position.Y);
			}
			if((source_corners[x].Y + position.Y) > clipRect.LowerRightCorner.Y){
				source_corners[x].Y = (float)((float)clipRect.LowerRightCorner.Y - (float)position.Y);
			}
		}

		if(!clipRect.isPointInside(irr::core::vector2d<irr::s32>((irr::s32)corner[x].X, (irr::s32)corner[x].Y))){
			if(corner[x].X < clipRect.UpperLeftCorner.X){
				corner[x].X = (float)clipRect.UpperLeftCorner.X;				
			}
			if(corner[x].X > clipRect.LowerRightCorner.X){
				corner[x].X = (float)clipRect.LowerRightCorner.X; 
			}
			if(corner[x].Y < clipRect.UpperLeftCorner.Y){
				corner[x].Y = (float)clipRect.UpperLeftCorner.Y;
			}
			if(corner[x].Y > clipRect.LowerRightCorner.Y){
				corner[x].Y = (float)clipRect.LowerRightCorner.Y;
			}
		}
	}

	// 3. Rotate it back
    if (rotation != 0.0f)
        for (int x = 0; x < 8; x++)
            source_corners[x].rotateBy((irr::f32)rotation*-1,irr::core::vector2df(source_center_x, source_center_y));

	// 4. Use the UV Coordinates of the resulting shape
	irr::core::vector2df uvCorner[8];
	for (int x = 0; x < 8; x++)
		uvCorner[x] = source_corners[x];

    for (int x = 0; x < 8; x++) {
        float uvX = uvCorner[x].X/(float)texture->getSize().Width;
        float uvY = uvCorner[x].Y/(float)texture->getSize().Height;
        uvCorner[x] = irr::core::vector2df(uvX,uvY);
    }

	irr::video::S3DVertex vertices[8];

    // each indices is a point of a triangle to render
	// put them in the wrong order, and the triangle won't render right
	// I solved this puzzle one triangle at a time, to come up with this list
    irr::u16 indices[18] = { 0, 1, 3, 0, 2, 3, 3, 2, 4, 3, 4, 7, 3, 7, 6, 3, 6, 5 };

    // Convert pixels to world coordinates
    float screenWidth = (float)driver->getScreenSize().Width;
    float screenHeight = (float)driver->getScreenSize().Height;

    for (int x = 0; x < 8; x++) {
        float screenPosX = ((corner[x].X/screenWidth)-0.5f)*2.0f;
        float screenPosY = ((corner[x].Y/screenHeight)-0.5f)*-2.0f;

        vertices[x].Pos = irr::core::vector3df(screenPosX,screenPosY,1);
        vertices[x].TCoords = uvCorner[x];
        vertices[x].Color = color;
    }

    material.Lighting = false;
    material.ZWriteEnable = false;
    material.ZBuffer = false;
    material.TextureLayer[0].Texture = texture;
    material.MaterialTypeParam = irr::video::pack_texureBlendFunc(irr::video::EBF_SRC_ALPHA, irr::video::EBF_ONE_MINUS_SRC_ALPHA, irr::video::EMFN_MODULATE_1X, irr::video::EAS_TEXTURE | irr::video::EAS_VERTEX_COLOR);

    if (useAlphaChannel)
        material.MaterialType = irr::video::EMT_ONETEXTURE_BLEND;
    else
        material.MaterialType = irr::video::EMT_SOLID;

    driver->setMaterial(material);
	driver->drawIndexedTriangleList(&vertices[0],8,&indices[0],6);

    // Restore projection and view matrices
    driver->setTransform(irr::video::ETS_PROJECTION,oldProjMat);
    driver->setTransform(irr::video::ETS_VIEW,oldViewMat);
}

bullet::bullet(){
	x = 0;
	y = 0;
	inertia_x = 0;
	inertia_y = 0;
	counter = 0;
	fade_after = 80;

	type = TYPE_VANILLA;

	to_destroy = false;

	power = 10;
}

bullet::~bullet(){}

bool engine::projectile_step(boost::shared_ptr<creature> thecreature, bullet &thebullet){
	thebullet.x += thebullet.inertia_x;
	thebullet.y += thebullet.inertia_y;

	if(++(thebullet.counter) > thebullet.fade_after){
		return false;
	}

	for(vector<boost::shared_ptr<map_object> >::iterator mo_itr = gd.map_objects.begin(); mo_itr != gd.map_objects.end(); mo_itr++){
		if(thebullet.x > (*mo_itr)->x && thebullet.x < (*mo_itr)->x1 &&
			thebullet.y > (*mo_itr)->y1 && thebullet.y < (*mo_itr)->y){
					
			if(*mo_itr != thecreature){
				(*mo_itr)->hit_by_projectile(thebullet);

				return false;
			}
		}
	}

	return true;
}
