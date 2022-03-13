
#include <iostream>
#include <vector>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

using namespace std;


class DJoySticks
{
public:
	static IDirectInput8* dev;									// the main DirectInput device

	static std::vector<LPDIRECTINPUTDEVICE8> gameControllers;

	
	static std::vector<DIJOYSTATE> gameControllerStates;
	
	static std::vector<DIDEVCAPS> capabilities;
	
	static HWND joy_hWnd;
	static HINSTANCE joy_hInstance;

	static BOOL CALLBACK staticEnumerateGameControllers(LPCDIDEVICEINSTANCE devInst, LPVOID pvRef);
	static BOOL CALLBACK staticSetGameControllerAxesRanges(LPCDIDEVICEOBJECTINSTANCE devObjInst, LPVOID pvRef);
	
	bool Init ( HWND hWnd, HINSTANCE hInstance );
	
	// re-initialize using the hWnd and hInstance that was already supplied
	bool ReInit ();
	
	bool Update( int iPadNum );
	
	bool Release ();
};

