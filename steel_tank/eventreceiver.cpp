#include "stdafx.h"
#include "eventreceiver.h"
#include "steel_tank.h"

bool MyEventReceiver::OnEvent(const irr::SEvent& event){
        if (event.EventType == irr::EET_MOUSE_INPUT_EVENT){
                switch(event.MouseInput.Event)
                {
				case irr::EMIE_LMOUSE_PRESSED_DOWN:
                        MouseState.LeftButtonDown = true;
						app.eng.clk = true;
                        break;

				case irr::EMIE_LMOUSE_LEFT_UP:
                        MouseState.LeftButtonDown = false;
                        break;

                case irr::EMIE_MOUSE_MOVED:
                        MouseState.Position.X = event.MouseInput.X;
                        MouseState.Position.Y = event.MouseInput.Y;
                        break;

                default:
                        break;
                }
        }

        if (event.EventType == irr::EET_JOYSTICK_INPUT_EVENT && event.JoystickEvent.Joystick == 0) {
                JoystickState = event.JoystickEvent;
        }

        return false;
}

const irr::SEvent::SJoystickEvent & MyEventReceiver::GetJoystickState(void){
        return JoystickState;
}

const MyEventReceiver::SMouseState & MyEventReceiver::GetMouseState(void){
        return MouseState;
}


MyEventReceiver::MyEventReceiver(){}
