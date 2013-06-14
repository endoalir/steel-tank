#pragma once
#include <irrlicht.h>

class MyEventReceiver : public irr::IEventReceiver {
public:
        struct SMouseState
        {
                irr::core::position2di Position;
                bool LeftButtonDown;
                SMouseState() : LeftButtonDown(false) { }
        } MouseState;

        virtual bool OnEvent(const irr::SEvent& event);
        const irr::SEvent::SJoystickEvent & GetJoystickState(void);
        const SMouseState & GetMouseState(void);

        MyEventReceiver();

private:
        irr::SEvent::SJoystickEvent JoystickState;
};
