#pragma once
#define DIRECTINPUT_VERSION 0x0800
#define _CRT_SECURE_NO_DEPRECATE
#ifndef _WIN32_DCOM
#define _WIN32_DCOM
#endif

#include <windows.h>
#include <commctrl.h>
#include <basetsd.h>
#include <dinput.h>
#include <dinputd.h>
#include <assert.h>
#include <oleauto.h>
#include <shellapi.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )
#include <wbemidl.h>
#pragma warning( disable : 4995 ) // disable deprecated warning 
#include <vector>
#pragma warning( default : 4995 )
#include "time.h"

BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );


struct DI_ENUM_CONTEXT
{
	DIJOYCONFIG* pPreferredJoyCfg;
	bool bPreferredJoyCfgValid;
};


class joystick{
public:
	joystick(void);
	~joystick(void);

	std::vector<LPDIRECTINPUTDEVICE8>    joystick_devices;
	LPDIRECTINPUT8 g_pDI;
	std::vector<GUID> joystick_guids;

	DI_ENUM_CONTEXT enumContext;

	HRESULT InitDirectInput(void);

	HRESULT SetupForIsXInputDevice(void);
	bool IsXInputDevice( const GUID* pGuidProductFromDirectInput );
	void CleanupForIsXInputDevice(void);

	HRESULT UpdateInputState(void);
	VOID FreeDirectInput(void);

	void set_parent_window(HWND window);
	HWND get_parent_window(void);

	bool poll(void);

	int number_of_joysticks;

	std::vector<DIJOYSTATE2> joystates;

	clock_t last_query_time;

private:
	HWND parent_window;



};
