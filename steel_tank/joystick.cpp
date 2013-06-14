#include "stdafx.h"
#include "joystick.h"

joystick::joystick(void){
	parent_window = NULL;
	number_of_joysticks = 0;
	last_query_time = 0;
}
joystick::~joystick(void){}

HRESULT joystick::InitDirectInput(){
    HRESULT hr;

    if(FAILED(hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&g_pDI, NULL)))
        return hr;

    DIJOYCONFIG PreferredJoyCfg = {0};
    enumContext.pPreferredJoyCfg = &PreferredJoyCfg;
    enumContext.bPreferredJoyCfgValid = false;

    IDirectInputJoyConfig8* pJoyConfig = NULL;
    if(FAILED(hr = g_pDI->QueryInterface(IID_IDirectInputJoyConfig8, (void**)&pJoyConfig)))
        return hr;

    PreferredJoyCfg.dwSize = sizeof( PreferredJoyCfg );
    if( SUCCEEDED( pJoyConfig->GetConfig( 0, &PreferredJoyCfg, DIJC_GUIDINSTANCE)))
        enumContext.bPreferredJoyCfgValid = true;
    if(pJoyConfig) { (pJoyConfig)->Release(); pJoyConfig=NULL; }

	last_query_time = clock();
    if( FAILED( hr = g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY ) ) )
        return hr;

	/*if(parent_window){
		if(FAILED(hr = g_pJoystick->SetCooperativeLevel( parent_window, DISCL_EXCLUSIVE | DISCL_FOREGROUND)))
			return hr;
	}*/

    return S_OK;
}

void joystick::set_parent_window(HWND window){
	parent_window = window;
}

HWND joystick::get_parent_window(void){
	return parent_window;
}

bool joystick::poll(void){
	if((clock() - last_query_time) / CLOCKS_PER_SEC > 3){
		last_query_time = clock();
		g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY );
	}

    if( joystick_devices.empty() )
        return false;

	bool got_any = false;
	std::vector<LPDIRECTINPUTDEVICE8>::iterator jitr;
	int i=-1;
	for(jitr = joystick_devices.begin(); jitr != joystick_devices.end(); jitr++){
		i++;
		HRESULT hr = (*jitr)->Poll();
		if(FAILED(hr)){
			hr = (*jitr)->Acquire();
			while( hr == DIERR_INPUTLOST )
				hr = (*jitr)->Acquire();
			continue;
		}

	    hr = (*jitr)->GetDeviceState(sizeof(DIJOYSTATE2), &joystates[i]);	
		if(SUCCEEDED(hr)){
			got_any = true;
		}
	}

	return got_any;
}

BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext ){
	joystick *js = (joystick*)pContext;
    HRESULT hr;

	bool already_have = false;
	for(std::vector<GUID>::iterator jitr = js->joystick_guids.begin(); jitr != js->joystick_guids.end(); jitr++){
		if(*jitr == pdidInstance->guidInstance){
			already_have = true;
			break;
		}
	}

	if(already_have){
		return DIENUM_CONTINUE;
	}

	js->joystick_guids.push_back(pdidInstance->guidInstance);

    /*if( js->enumContext.bPreferredJoyCfgValid &&
        !IsEqualGUID( pdidInstance->guidInstance, js->enumContext.pPreferredJoyCfg->guidInstance ) )
        return DIENUM_CONTINUE;*/
	LPDIRECTINPUTDEVICE8 this_joystick_device;

    hr = js->g_pDI->CreateDevice( pdidInstance->guidInstance, &this_joystick_device, NULL);
    if(SUCCEEDED(hr)){
		if(SUCCEEDED(this_joystick_device->SetDataFormat( &c_dfDIJoystick2 ))){
			js->joystick_devices.push_back(this_joystick_device);
			DIJOYSTATE2 joystate;
			//memset(&joystate,0,sizeof(joystate));
			js->joystates.push_back(joystate);
			js->number_of_joysticks++;
		}
	}
    
	return DIENUM_CONTINUE;
}
