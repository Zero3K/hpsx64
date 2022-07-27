

#include "WinJoy.h"

#pragma comment(lib, "winmm")


using namespace WinApi;



int Joysticks::InitJoysticks ()
{
	if((wNumDevs = joyGetNumDevs()) == 0) 
		return 2 /*ERR_NODRIVER*/; 
	
	bDev1Attached = joyGetPosEx(JOYSTICKID1,&joyinfo[0]) != JOYERR_UNPLUGGED; 
	bDev2Attached = wNumDevs == 2 && joyGetPosEx(JOYSTICKID2,&joyinfo[1]) != JOYERR_UNPLUGGED; 
	
	if(bDev1Attached || bDev2Attached)   // decide which joystick to use 
		wDeviceID = bDev1Attached ? JOYSTICKID1 : JOYSTICKID2; 
	else 
		return 4 /*ERR_NODEVICE*/;
		
	return 0;
}



int Joysticks::ReadJoystick ( int DeviceNumber )
{
	joyinfo[DeviceNumber].dwSize = sizeof ( joyinfo[DeviceNumber] );
	joyinfo[DeviceNumber].dwFlags = JOY_RETURNALL;
	return joyGetPosEx( DeviceNumber, &joyinfo[DeviceNumber] );
}


