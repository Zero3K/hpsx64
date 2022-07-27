
#include "DJoySticks.h"

#pragma comment(lib, "dinput8")
#pragma comment(lib, "dxguid")

IDirectInput8* DJoySticks::dev;									// the main DirectInput device

std::vector<LPDIRECTINPUTDEVICE8> DJoySticks::gameControllers;


std::vector<DIJOYSTATE> DJoySticks::gameControllerStates;

std::vector<DIDEVCAPS> DJoySticks::capabilities;

HWND DJoySticks::joy_hWnd;
HINSTANCE DJoySticks::joy_hInstance;


BOOL CALLBACK DJoySticks::staticEnumerateGameControllers(LPCDIDEVICEINSTANCE devInst, LPVOID pvRef)
{
	LPDIRECTINPUTDEVICE8 gameController;
	
	if ((dev->CreateDevice(devInst->guidInstance, &gameController, NULL)))
		return DIENUM_CONTINUE;
	else
	{
		// store the game controller
		gameControllers.push_back(gameController); // (std::make_pair<std::wstring, LPDIRECTINPUTDEVICE8>(devInst->tszProductName, std::move(gameController)));
		return DIENUM_CONTINUE;
	}
	
	//InputHandler* inputHandlerInstance = (InputHandler*)pvRef;
	//return inputHandlerInstance->enumerateGameControllers(devInst);
	
}

BOOL CALLBACK DJoySticks::staticSetGameControllerAxesRanges(LPCDIDEVICEOBJECTINSTANCE devObjInst, LPVOID pvRef)
{
	// the game controller
	LPDIRECTINPUTDEVICE8 gameController = (LPDIRECTINPUTDEVICE8)pvRef;
	gameController->Unacquire();
		
	// structure to hold game controller range properties
	DIPROPRANGE gameControllerRange;

	// set the range to -100 and 100
	gameControllerRange.lMin = 1;
	gameControllerRange.lMax = 256;

	// set the size of the structure
	gameControllerRange.diph.dwSize = sizeof(DIPROPRANGE);
	gameControllerRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);

	// set the object that we want to change		
	gameControllerRange.diph.dwHow = DIPH_BYID;
	gameControllerRange.diph.dwObj = devObjInst->dwType;

	// now set the range for the axis		
	if ((gameController->SetProperty(DIPROP_RANGE, &gameControllerRange.diph))) {
		return DIENUM_STOP;
	}

    // structure to hold game controller axis dead zone
	DIPROPDWORD gameControllerDeadZone;

	// set the dead zone to 1%
	gameControllerDeadZone.dwData = 100;

	// set the size of the structure
	gameControllerDeadZone.diph.dwSize = sizeof(DIPROPDWORD);
	gameControllerDeadZone.diph.dwHeaderSize = sizeof(DIPROPHEADER);

	// set the object that we want to change
	gameControllerDeadZone.diph.dwHow = DIPH_BYID;
	gameControllerDeadZone.diph.dwObj = devObjInst->dwType;

	// now set the dead zone for the axis
	if ((gameController->SetProperty(DIPROP_DEADZONE, &gameControllerDeadZone.diph)))
		return DIENUM_STOP;
	
	return DIENUM_CONTINUE;
}


bool DJoySticks::ReInit ()
{
	// clear the old data out
	Release ();
	
	// re-initialize
	return Init ( joy_hWnd, joy_hInstance );
}

bool DJoySticks::Init ( HWND hWnd, HINSTANCE hInstance )
{
	// save hWnd and hInstance for re-init
	joy_hWnd = hWnd;
	joy_hInstance = hInstance;
	
	if ( DirectInput8Create(
         hInstance,
         DIRECTINPUT_VERSION,
         IID_IDirectInput8,
         (void**) &dev,
         NULL
	) )
	{
		cout << "\nERROR: DirectInput8Create FAILED!!!\n";
	}
	
	if ( dev->EnumDevices(
         DI8DEVCLASS_GAMECTRL,
         &staticEnumerateGameControllers,
         NULL,
         DIEDFL_ATTACHEDONLY
	) )
	{
		cout << "\nERROR: EnumDevices FAILED!!!\n";
	}
	
	cout << "\nCount of game controllers: " << dec << gameControllers.size();
	
	LPDIRECTINPUTDEVICE8 gameController;
	//gameController.dwSize = sizeof(LPDIRECTINPUTDEVICE8);
	
	DIJOYSTATE gameControllerState;
	
	DIDEVCAPS capability;
	capability.dwSize = sizeof(DIDEVCAPS);
	
	//HWND hWnd;
	//hWnd = GetActiveWindow ();
	
	cout << "\nhWnd=" << dec << hWnd << " " << hex << hWnd;
	
	for ( int i = 0; i < gameControllers.size(); i++ )
	{
		gameController = gameControllers [ i ];
		
		if ( gameController->SetCooperativeLevel ( hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE ) )
		{
			cout << "\nERROR: Unable to SetCooperativeLevel for game pad#" << dec << i;
		}
		
		if ((gameController->SetDataFormat(&c_dfDIJoystick)))
		{
			cout << "\nERROR: Unable to SetDataFormat for game pad#" << dec << i;
		}
		
		capability.dwSize = sizeof(DIDEVCAPS);
		if ( gameController->GetCapabilities( & capability ) )
		{
			cout << "\nERROR: Unable to GetCapabilities for game pad#" << dec << i;
		}
		
		// add into capabilities list
		capabilities.push_back( capability );
		
		if ((gameController->EnumObjects(&staticSetGameControllerAxesRanges, gameController, DIDFT_AXIS)))
		{
			cout << "\nERROR: Unable to EnumObjects for game pad#" << dec << i;
		}
		
		if ( gameController->Acquire() )
		{
			cout << "\nERROR: Unable to Acquire for game pad#" << dec << i;
		}
		
		//ret = gameController->Poll();
		
		//cout << "\nPoll returned: " << hex << ret;
		
		
		if ((gameController->GetDeviceState(sizeof(DIJOYSTATE), &gameControllerState)))
		{
			cout << "\nERROR: Unable to GetDeviceState for game pad#" << dec << i;
		}
		
		gameControllerStates.push_back( gameControllerState );
	}
	
	return true;
}


bool DJoySticks::Update( int iPadNum )
{
	LPDIRECTINPUTDEVICE8 gameController;
	
	if ( iPadNum < gameControllers.size() )
	{
		//DIJOYSTATE gameControllerState;
		gameController = gameControllers [ iPadNum ];
		
		// just in case this is needed
		gameController->Poll();
		
		if ((gameController->GetDeviceState(sizeof(DIJOYSTATE), &gameControllerStates [ iPadNum ])))
		{
			cout << "\nERROR: Unable to GetDeviceState for game pad#" << dec << iPadNum;
			return false;
		}
	}

	return true;
}


bool DJoySticks::Release ()
{
	LPDIRECTINPUTDEVICE8 gameController;
	
	for ( int i = 0; i < gameControllers.size(); i++ )
	{
		gameController = gameControllers [ i ];
		
		gameController->Unacquire();
		gameController->Release();
	}
	
	// clear vectors
	gameControllers.clear();
	gameControllerStates.clear();
	capabilities.clear();
	
	return true;
}




