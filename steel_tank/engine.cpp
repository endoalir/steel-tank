#include "stdafx.h"
#include "steel_tank.h"

engine::engine() {
	invincible_mode = false;
	super_player = false;
	clipless = false;
	superfast = false;
	epicplayer = false;

	prev_time = 0;
	showfps = true;
	lbutton = false;
	mbutton = false;
	rbutton = false;
	c_x = 0;
	c_y = 0;

	last_fire_time = 0;

	wf_step = 0;

	camera_x = 0, camera_y = 0;
	map_width = 15000; map_height = 15000;

	number_of_players = 0;

	current_sequence = 0;

	polled_js = false;
}

engine::~engine(){};

bool engine::check_collision(int x, int y, int x1, int y1, map_object *collided_with){
	bool would_be_collided = false;

	map<int, boost::shared_ptr<map_object> >::iterator mo_itr;
	for(mo_itr = app.eng.gd.loaded_objects.begin(); mo_itr != app.eng.gd.loaded_objects.end();){
		boost::shared_ptr<map_object> this_object = mo_itr->second;
		if(this_object->to_remove){
			mo_itr = app.eng.gd.loaded_objects.erase(mo_itr);
			continue;
		}

		if(this_object.get() != collided_with && this_object->is_solid){
			if(this_object->is_within(x, y)  || this_object->is_within(x, y1)
				|| this_object->is_within(x1, y) || this_object->is_within(x1, y1)){
				would_be_collided = true;
				break;
			}
		}

		mo_itr++;
	}

	return would_be_collided;
}

void engine::animagic(void){
	map<int, boost::shared_ptr<map_object> >::iterator mo_itr;
	for(mo_itr = app.eng.gd.loaded_objects.begin(); mo_itr != app.eng.gd.loaded_objects.end(); mo_itr++){
		boost::shared_ptr<map_object> this_object( (*mo_itr).second );
		
		try{
			boost::shared_ptr<creature> creature_obj = boost::shared_dynamic_cast<creature>(this_object);
			if(creature_obj){
				for(vector<::bullet>::iterator bulletitr = creature_obj->bullets.begin(); bulletitr != creature_obj->bullets.end();){
					if(!app.eng.projectile_step(creature_obj, *bulletitr)){
						bulletitr = creature_obj->bullets.erase(bulletitr);
						continue;
					}
					bulletitr++;
				}

				if(creature_obj->is_enemy)
					creature_obj->botomatic();
			}
		}catch(...){}

	}
}

void engine::load(void){
	int s = 0;
	sequences.push_back(boost::shared_ptr<game_sequence>(new splash_sequence(s++)));
	sequences.push_back(boost::shared_ptr<game_sequence>(new title_sequence(s++)));
	sequences.push_back(boost::shared_ptr<game_sequence>(new playerselect_sequence(s++)));
	sequences.push_back(boost::shared_ptr<game_sequence>(new playertypeselect_sequence(s++)));
	sequences.push_back(boost::shared_ptr<game_sequence>(new play_start_sequence(s++)));
	sequences.push_back(boost::shared_ptr<game_sequence>(new main_sequence(s++)));

	sequences[0]->load();


	js.InitDirectInput();
	snd.init();

	
	/*vector<boost::shared_ptr<game_sequence> >::iterator gs;
	for(gs = sequences.begin(); gs != sequences.end(); gs++){
		boost::shared_ptr<game_sequence> this_seq = *gs;
		this_seq->load();
	}*/

	irr::video::IVideoDriver *driver = app.app_windows[0]->driver;
	irr::IrrlichtDevice *device = app.app_windows[0]->device;

	app.eng.gd.find_start_positions();

	driver->getMaterial2D().TextureLayer[0].BilinearFilter=true;
	driver->getMaterial2D().AntiAliasing = irr::video::EAAM_FULL_BASIC;

	font = device->getGUIEnvironment()->getBuiltInFont();

}

void engine::runner(int i){
	unsigned int timerv = (unsigned int)clock();

	wf_step++;
	if(wf_step > 100){wf_step = 0;}

	polled_js = js.poll();

	vector<shared_ptr<window> >::iterator witr;
	for(witr = app.app_windows.begin(); witr != app.app_windows.end(); witr++){
		shared_ptr<window> this_window(*witr);
		if(!this_window || !this_window.get() || !this_window->driver)
			continue;
		this_window->driver->beginScene(true, true, 0, this_window->videodata);

	}

	sequences[current_sequence]->render();
	if(sequences[current_sequence]->sequence_over){
		current_sequence++;

		// this is all crashy crashy an stuff
		/*if(current_sequence > sequences.size()-2){
			if(app.app_windows[0] && app.app_windows[0]->driver)
				app.app_windows[0]->driver->removeAllTextures();
		}*/

		if(current_sequence > sequences.size()-1){
			current_sequence = 0;
			if(app.app_windows[0] && app.app_windows[0]->driver)
				app.app_windows[0]->driver->removeAllTextures();

		}

		sequences[current_sequence]->reset();
		sequences[current_sequence]->load();
	}

	// *** FPS ***
	if (font){
		if(showfps){
			irr::core::stringw drawstr(i);
			font->draw(drawstr, irr::core::rect<irr::s32>(10,10,0,0), irr::video::SColor(255,255,255,255));
		}
	}

	if(js.number_of_joysticks > 0 && polled_js){
		if(js.joystates[0].rgbButtons[6]){ // reset
			current_sequence = 0;
			sequences[current_sequence]->reset();
			sequences[current_sequence]->load();

			bool firstone = true;
			for(auto w = app.app_windows.begin(); app.app_windows.size() != 1; w = app.app_windows.begin()){
				if(firstone){ firstone = false; continue; }
				(*w)->close();
			}

		}
	}

	for(witr = app.app_windows.begin(); witr != app.app_windows.end(); witr++){
		shared_ptr<window> this_window(*witr);
		if(!this_window || !this_window.get() || !this_window->driver)
			continue;

		this_window->smgr->drawAll();
		this_window->driver->endScene();
	}
}

void engine::repos_cams(void){
	vector<shared_ptr<window> >::iterator itr;
	for(itr = app.app_windows.begin(); itr != app.app_windows.end(); itr++){
		(*itr)->cams.this_window = *itr;
		(*itr)->cams.reposition();
	}
}

void engine::center_cams(void){
	vector<shared_ptr<window> >::iterator itr;
	for(itr = app.app_windows.begin(); itr != app.app_windows.end(); itr++){
		shared_ptr<window> this_window(*itr);
		this_window->cams.center();
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

bool engine::projectile_step(boost::shared_ptr<map_object> originated_object, bullet &thebullet){
	thebullet.x += thebullet.inertia_x;
	thebullet.y += thebullet.inertia_y;

	if(++(thebullet.counter) > thebullet.fade_after){
		return false;
	}

	map<int, boost::shared_ptr<map_object> >::iterator mo_itr;
	for(mo_itr = app.eng.gd.loaded_objects.begin(); mo_itr != app.eng.gd.loaded_objects.end(); mo_itr++){
		boost::shared_ptr<map_object> this_object = mo_itr->second;

		if(!this_object->is_solid)
			continue;

		if(thebullet.x > this_object->x && thebullet.x < this_object->x1 &&
			thebullet.y > this_object->y1 && thebullet.y < this_object->y){
					
			if(this_object != originated_object){
				this_object->hit_by_projectile(thebullet);
				return false;
			}
		}
	}

	return true;
}
