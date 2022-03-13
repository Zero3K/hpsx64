
#include <windows.h>


#ifndef _WINJOY_H_
#define _WINJOY_H_

namespace WinApi
{

	class Joysticks
	{
	public:
		
		JOYINFOEX joyinfo [ 2 ]; 
		UINT wNumDevs, wDeviceID; 
		BOOL bDev1Attached, bDev2Attached;
		
		JOYINFOEX jie;
		
		int InitJoysticks ();
		
		int ReadJoystick ( int DeviceNumber );
	};
};

#endif


