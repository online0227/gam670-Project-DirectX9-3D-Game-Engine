/* Input DeviceSet - Implementation
 *
 * Input.cpp
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "IConfiguration.h" // for WindowAddress()
#include "UISettings.h"     // for symbolic names shared with dialog box
#include "DeviceSettings.h" // for SAMPLE_BUFFER_SIZE, ENTER, UP, PGDN
#include "Utilities.h"      // for error()
#include "Input.h"          // for Keyboard, Mouse, Joystick class
                            // declarations
#define  INITGUID
#include <cguid.h>          // for GUID_NULL
#undef   INITGUID

//------------------------------- Keyboard -----------------------------
//
IKeyboard* CreateKeyboard(void* hinstance) {

	return new Keyboard(hinstance);
}

// constructor retrieves interface to DirectInput object and initializes
// instance variables
//
Keyboard::Keyboard(void* hinstance) {

    // acquire an interface to DirectInput object for this application
    di = NULL;
    if (FAILED(DirectInput8Create((HINSTANCE)hinstance, 
	 DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&di, NULL))) {
        error("Keyboard::00 Unable to obtain an interface to Direct "
         "Input");
    }

    keyboard   = NULL;
    for (int i = 0; i < SIZE_KEYBOARD_PACKET; i++)
        key[i] = 0;
}

// setup accesses the keyboard, sets its data format and cooperative
// level sets the size of the keyboard buffer and acquires the keyboard
//
bool Keyboard::setup() {

    bool rc = false;
	hwnd = (HWND)WindowAddress()->window();

    // release the keyboard if still attached
    release();

    // obtain interface to the keyboard
    if (FAILED(di->CreateDevice(GUID_SysKeyboard, &keyboard, NULL)))
        error("Keyboard::10 Unable to obtain an interface to system "
         "keyboard");
    // set the data format for the keyboard data
    else if (FAILED(keyboard->SetDataFormat(&c_dfDIKeyboard))) {
        release();
        error("Keyboard::11 Unable to set the data format for keyboard");
    }
    // set the cooperative level
    else if (FAILED(keyboard->SetCooperativeLevel(hwnd, 
     DISCL_NONEXCLUSIVE | DISCL_FOREGROUND))) {
        release();
        error("Keyboard::12 Unable to set the cooperative level for "
         "keyboard");
    }
    else {
        // set the size of the keyboard buffer
        //
        // property struct consists of a header and a data member
        DIPROPDWORD dipdw;
        // property struct header
        // - size of enclosing structure
        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        // - always size of DIPROPHEADER
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        // - identifier for property in question - 0 for entire device
        dipdw.diph.dwObj        = 0;
        // - DIPH_DEVICE since entire device is involved
        dipdw.diph.dwHow        = DIPH_DEVICE;
        // property struct data member (takes a single word of data)
        // - the buffer size goes here
        dipdw.dwData            = SAMPLE_BUFFER_SIZE;

        // set the size of the buffer
        if (FAILED(keyboard->SetProperty(DIPROP_BUFFERSIZE, 
		 &dipdw.diph))) {
            release();
            error("Keyboard::13 Failed to set size of keyboard buffer");
        }
        // flush buffer: data currently in the buffer will be ignored
        else {
			// try to acquire the keyboard
			if (SUCCEEDED(keyboard->Acquire())) 
				update();
            rc = true;
        }
    }

    return rc;
}

// update retrieves the contents of the keyboard buffer and stores key
// press/release values in keys[] for subsequent polling by pressed()
//
void Keyboard::update() {

    HRESULT hr;
    DWORD items = SAMPLE_BUFFER_SIZE;
    DIDEVICEOBJECTDATA dod[SAMPLE_BUFFER_SIZE];

    if (keyboard) {
        hr = keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), dod, 
		 &items, 0);
		// if keyboard is lost, try to re-acquire it
        if (DIERR_INPUTLOST == hr && SUCCEEDED(keyboard->Acquire()))
            hr = keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), 
			 dod, &items, 0);
        if (SUCCEEDED(hr))
            for (DWORD i = 0; i < items; i++)
                key[dod[i].dwOfs] = !!(dod[i].dwData & 0x80);
    }
}

// pressed returns the key's state - true for pressed, false for not
// pressed note that not all keys are included in this table - it may
// be enhanced
//
bool Keyboard::pressed(int c) const {

    int k = -1;

    switch (c) {
      case 'A': k = DIK_A; break;
      case 'B': k = DIK_B; break;
      case 'C': k = DIK_C; break;
      case 'D': k = DIK_D; break;
      case 'E': k = DIK_E; break;
      case 'F': k = DIK_F; break;
      case 'G': k = DIK_G; break;
      case 'H': k = DIK_H; break;
      case 'I': k = DIK_I; break;
      case 'J': k = DIK_J; break;
      case 'K': k = DIK_K; break;
      case 'L': k = DIK_L; break;
      case 'M': k = DIK_M; break;
      case 'N': k = DIK_N; break;
      case 'O': k = DIK_O; break;
      case 'P': k = DIK_P; break;
      case 'Q': k = DIK_Q; break;
      case 'R': k = DIK_R; break;
      case 'S': k = DIK_S; break;
      case 'T': k = DIK_T; break;
      case 'U': k = DIK_U; break;
	  case 'V': k = DIK_V; break;
      case 'W': k = DIK_W; break;
      case 'X': k = DIK_X; break;
      case 'Y': k = DIK_Y; break;
      case 'Z': k = DIK_Z; break;
      case ' ': k = DIK_SPACE; break;
      case ENTER: k = DIK_RETURN; break;
      case UP   : k = DIK_UP;     break;
      case DOWN : k = DIK_DOWN;   break;
      case PGUP : k = DIK_PRIOR;  break;
      case PGDN : k = DIK_NEXT;   break;
      case LEFT : k = DIK_LEFT;   break;
      case RIGHT: k = DIK_RIGHT;  break;
      case NUM1:  k = DIK_NUMPAD1; break;
      case NUM2:  k = DIK_NUMPAD2; break;
      case NUM3:  k = DIK_NUMPAD3; break;
      case NUM4:  k = DIK_NUMPAD4; break;
      case NUM5:  k = DIK_NUMPAD5; break;
      case NUM6:  k = DIK_NUMPAD6; break;
      case NUM7:  k = DIK_NUMPAD7; break;
      case NUM8:  k = DIK_NUMPAD8; break;
      case NUM9:  k = DIK_NUMPAD9; break;
      case ESCAPE: k = DIK_ESCAPE; break;
      case F1:  k = DIK_F1;  break;
      case F2:  k = DIK_F2;  break;
      case F3:  k = DIK_F3;  break;
      case F4:  k = DIK_F4;  break;
      case F5:  k = DIK_F5;  break;
      case F6:  k = DIK_F6;  break;
      case F7:  k = DIK_F7;  break;
      case F8:  k = DIK_F8;  break;
      case F9:  k = DIK_F9;  break;
      case F10: k = DIK_F10; break;
      case F11: k = DIK_F11; break;
      case F12: k = DIK_F12; break;
    }

    return (k != -1) ? key[k] : 0;
}

// suspends unaquires the keyboard in preparation for loss of focus
//
void Keyboard::suspend() {

    if (keyboard) keyboard->Unacquire();
}

// restore re-acquires the keyboard prior to re-activation
//
bool Keyboard::restore() {

    bool rc = true;

    if (keyboard) {
		HRESULT hr = keyboard->Acquire();
		if (hr != S_OK && hr != S_FALSE && hr != DIERR_OTHERAPPHASPRIO) {
            release();
            error("Keyboard::70 Failed to re-acquire the keyboard");
            rc = false;
        }
    }

    return rc;
}

// release suspends the keyboard object and then detaches the interface
// to the keyboard
//
void Keyboard::release() {

    suspend();
	if (keyboard) {
        keyboard->Release();
        keyboard = NULL;
    }
    for (int i = 0; i < SIZE_KEYBOARD_PACKET; i++)
        key[i] = 0;
}

// destructor releases the keyboard object and detaches the interface to
// the Direct Input object
//
Keyboard::~Keyboard() {

    release();
    if (di) {
        di->Release();
        di = NULL;
    }
	hwnd = NULL;
}

//------------------------------- Mouse ---------------------------------
//
IMouse* CreateMouse(void* hinstance) {

	return new Mouse(hinstance);
}

// constructor retrieves interface to DirectInput object and initializes
// instance variables
//
Mouse::Mouse(void* hinstance) {

    // acquire an interface to DirectInput object for this application
    di = NULL;
    if (FAILED(DirectInput8Create((HINSTANCE)hinstance, 
	 DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&di, NULL))) {
         error("Mouse::00 Unable to obtain an interface to Direct"
		  "Input");
    }

    mouse  = NULL;
    left   = 0;
    right  = 0;
    disp_x = 0;
    disp_y = 0;
    rot_x  = 0;
}

// setup accesses the mouse, sets its data format and cooperative level
// sets the size of the mouse buffer and acquires the keyboard
//
bool Mouse::setup() {

    bool rc = false;
	hwnd = (HWND)WindowAddress()->window();

    // release the mouse object if previously attached
    release();

    // obtain interface to the mouse
    if (FAILED(di->CreateDevice(GUID_SysMouse, &mouse, NULL)))
        error("Mouse::10 Unable to obtain an interface to system"
		 "mouse");
    // set the data format for mouse data
    else if (FAILED(mouse->SetDataFormat(&c_dfDIMouse))) {
        release();
        error("Mouse::11 Unable to set the data format for mouse");
    }
    // set the cooperative level
    else if (FAILED(mouse->SetCooperativeLevel(hwnd, 
	 DISCL_NONEXCLUSIVE | DISCL_FOREGROUND))) {
        release();
        error("Mouse::12 Unable to set cooperative level for mouse");
    }
    else {
        // set the size of the mouse buffer
        //
        // proerty structure consists of a header and a data member
        DIPROPDWORD dipdw;
        // property header
        // - size of enclosing structure
        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        // - always size of DIPROPHEADER
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        // - identifier for property in question - 0 for entire device
        dipdw.diph.dwObj        = 0;
        // - DIPH_DEVICE since entire device is involved
        dipdw.diph.dwHow        = DIPH_DEVICE;
        // property data member (takes a single word of data)
        // - the buffer size goes here
        dipdw.dwData            = SAMPLE_BUFFER_SIZE;

        // set the buffer size
        if (FAILED(mouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph))){
            release();
            error("Mouse::13 Failed to set the buffer size");
        }
        // flush the buffer: data currently in buffer will be ignored
        else {
			// try to acquire the mouse
			if (SUCCEEDED(mouse->Acquire())) 
				update();
            rc = true;
        }
    }

    return rc;
}

// update retrieves the contents of the mouse buffer and accumulates the
// values for subsequent extraction by pressed() and displacement()
//
void Mouse::update() {

    HRESULT hr;
    DWORD items = SAMPLE_BUFFER_SIZE;
    DIDEVICEOBJECTDATA dod[SAMPLE_BUFFER_SIZE];

    if (mouse) {
        hr = mouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), dod, 
		 &items, 0);
		// try to re-acquire if lost
        if (DIERR_INPUTLOST == hr && SUCCEEDED(mouse->Acquire()))
            hr = mouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), dod,
             &items, 0);
        if (SUCCEEDED(hr)) {
            disp_x = disp_y = rot_x = left = right = 0;
            for (DWORD i = 0; i < items; i++) {
                switch (dod[i].dwOfs) {
                  case DIMOFS_BUTTON0:
                      left = (dod[i].dwData & 0x80) >> 7;
                      break;
                  case DIMOFS_BUTTON1:
                      right = (dod[i].dwData & 0x80) >> 7;
                      break;
                  case DIMOFS_X:
                      disp_x += dod[i].dwData;
                      break;
                  case DIMOFS_Y:
                      disp_y += dod[i].dwData;
                      break;
                  case DIMOFS_Z:
                      rot_x += dod[i].dwData;
                      break;
                }
            }
        }
    }
}

// pressed returns true if Button b is pressed, false otherwise
//
int Mouse::pressed(Button b) const {

    int rc = 0;

    if (b == LEFT_BUTTON)
        rc = left;
    else if (b == RIGHT_BUTTON)
        rc = right;

    return rc;
}

// displacement returns the change in position since the last update
//
void Mouse::displacement(int& disp_xx, int& disp_yy, int& rot_xx) const 
{

    disp_xx = disp_x;
    disp_yy = disp_y;
    rot_xx  = rot_x;
}

//	mouse->displacementForMyGame(dy, dx);
void Mouse::displacementForMyGame(int &disp_xx, int &disp_yy) const {
	disp_xx = disp_y;
	disp_yy = disp_x;
	
}
// suspends unaquires the mouse in preparation for loss of focus
//
void Mouse::suspend() {

    if (mouse) mouse->Unacquire();
}

// restore re-acquires the mouse
//
bool Mouse::restore() {

    bool rc = true;

    if (mouse) {
		HRESULT hr = mouse->Acquire();
		if (hr != S_OK && hr != S_FALSE && hr != DIERR_OTHERAPPHASPRIO) {
            release();
            error("Mouse::70 Failed to re-acquire the mouse");
            rc = false;
        }
    }

    return rc;
}

// release suspends the mouse object and then detaches the interface to
// the mouse
//
void Mouse::release() {

    suspend();
	if (mouse) {
        mouse->Release();
        mouse = NULL;
    }
    left   = 0;
    right  = 0;
    disp_x = 0;
    disp_y = 0;
    rot_x  = 0;
}

// destructor releases the mouse and detaches the interface to the
// Direct Input object
//
Mouse::~Mouse() {

    release();
    if (di) {
        di->Release();
        di = NULL;
    }
	hwnd = NULL;
}

//------------------------------- Joystick -----------------------------
//
IJoystick* CreateJoystick(void* hinstance) {

	return new Joystick(hinstance);
}

// constructor retrieves interface to DirectInput object and initializes
// instance variables
//
Joystick::Joystick(void* hinstance) {

    // acquire an interface to DirectInput object for this application
    di = NULL;
    if (FAILED(DirectInput8Create((HINSTANCE)hinstance, 
	 DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&di, NULL))) {
        error("Joystick::00 Unable to obtain an interface to Direct "
         "Input");
    }

    joystick  = NULL;
    guid      = GUID_NULL;
    trgButton = 0;
    reversey  = false;
    zAxisOn   = false;
	fBackOn   = false;
	centre    = NULL;
    trgForce  = NULL;
    trgFrGuid = GUID_NULL;
    noForceEffects = 0;
    for (int i = 0; i < MAX_FORCE_EFFECTS; i++)
        force[i] = NULL;
    release();
}

// setup accesses the joystick, sets up its data format and cooperative
// level sets the buffer size and acquires the controller
//
bool Joystick::setup() {

    bool rc = false;
	LPGUID lpguid;
	LPGUID ffguid;
	GUID guid;
	SelectorAddress()->configureJ((void**)&lpguid, trgButton, reversey, 
	 zAxisOn, fBackOn, (void**)&ffguid);
	guid      = lpguid ? *lpguid : GUID_NULL;
	trgFrGuid = ffguid ? *ffguid : GUID_NULL;
	hwnd      = (HWND)WindowAddress()->window();

    // release the joystick object if attached
    release();

    // no controller selected
    if (guid == GUID_NULL)
        rc = true;
    // obtain interface to the controller
    else if (FAILED(di->CreateDevice(*lpguid, &joystick, NULL)))
        error("Joystick::10 Unable to create an interface to joystick");
    // set the data format for the controller
    else if (FAILED(joystick->SetDataFormat(&c_dfDIJoystick2))) {
        release();
        error("Joystick::11 Unable to set the data format for joystick");
    }
    // set the cooperative level
    else if (FAILED(joystick->SetCooperativeLevel(hwnd,(fBackOn ?
     DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE) | DISCL_FOREGROUND))) {
        release();
        error("Joystick::12 Unable to set the behavior for joystick");
    }
    else {
        // retrieve the axes that are active on this device
        DIDEVICEOBJECTINSTANCE didoi;
        didoi.dwSize = sizeof didoi;
        if (SUCCEEDED(joystick->GetObjectInfo(&didoi, DIJOFS_X,
         DIPH_BYOFFSET)))
            axisIsActive[0] = true;
        if (SUCCEEDED(joystick->GetObjectInfo(&didoi, DIJOFS_Y,
         DIPH_BYOFFSET)))
            axisIsActive[1] = true;
        if (SUCCEEDED(joystick->GetObjectInfo(&didoi, DIJOFS_Z,
         DIPH_BYOFFSET)))
            axisIsActive[2] = true;
        if (SUCCEEDED(joystick->GetObjectInfo(&didoi, DIJOFS_RZ,
         DIPH_BYOFFSET)))
            axisIsActive[3] = true;
        // ignore what GetObjectInfo returned if we don't want z axis
        if (!zAxisOn) {
            axisIsActive[2] = false;
            axisIsActive[3] = false;
        }

        // Set the range, deadzone, and saturation for each axis

        DIPROPRANGE range;

        range.diph.dwSize = sizeof range;
        range.diph.dwHeaderSize = sizeof range.diph;
        range.diph.dwObj = DIJOFS_X;
        range.diph.dwHow = DIPH_BYOFFSET;
        range.lMin = -100;
        range.lMax =  100;

        DIPROPDWORD dead,
                    sat;

        dead.diph.dwSize = sizeof dead;
        dead.diph.dwHeaderSize = sizeof dead.diph;
        dead.diph.dwObj = DIJOFS_X;
        dead.diph.dwHow = DIPH_BYOFFSET;
        dead.dwData = 300; // hundredths of a percent [0,10000]

        sat = dead;
        sat.dwData = 9800;

        if (axisIsActive[0]) {
            joystick->SetProperty(DIPROP_RANGE, &range.diph);
            joystick->SetProperty(DIPROP_DEADZONE, &dead.diph);
            joystick->SetProperty(DIPROP_SATURATION, &sat.diph);
        }

        if (axisIsActive[1]) {
            range.diph.dwObj = DIJOFS_Y;
            dead.diph.dwObj  = DIJOFS_Y;
            sat.diph.dwObj   = DIJOFS_Y;
            joystick->SetProperty(DIPROP_RANGE, &range.diph);
            joystick->SetProperty(DIPROP_DEADZONE, &dead.diph);
            joystick->SetProperty(DIPROP_SATURATION, &sat.diph);
        }

        if (axisIsActive[2]) {
            range.diph.dwObj = DIJOFS_Z;
            dead.diph.dwObj  = DIJOFS_Z;
            sat.diph.dwObj   = DIJOFS_Z;
            joystick->SetProperty(DIPROP_RANGE, &range.diph);
            joystick->SetProperty(DIPROP_DEADZONE, &dead.diph);
            joystick->SetProperty(DIPROP_SATURATION, &sat.diph);
        }

        if (axisIsActive[3]) {
            range.diph.dwObj = DIJOFS_RZ;
            dead.diph.dwObj  = DIJOFS_RZ;
            sat.diph.dwObj   = DIJOFS_RZ;
            joystick->SetProperty(DIPROP_RANGE, &range.diph);
            joystick->SetProperty(DIPROP_DEADZONE, &dead.diph);
            joystick->SetProperty(DIPROP_SATURATION, &sat.diph);
        }

		// try to acquire the joystick
		joystick->Acquire();

		// setup feedback effects
		//
		if (fBackOn) {
            DIEFFECT dif;
			DWORD axes[2] = {DIJOFS_X, DIJOFS_Y};
            LONG dir[2]   = {1, 0};
            ZeroMemory(&dif, sizeof dif);

            // self-centering
            DICONDITION dic;

            dic.lOffset = 0;
            dic.lPositiveCoefficient = 0 * DI_FFNOMINALMAX;
            dic.lNegativeCoefficient = 0 * DI_FFNOMINALMAX;
            dic.dwPositiveSaturation = DI_FFNOMINALMAX;
            dic.dwNegativeSaturation = DI_FFNOMINALMAX;
            dic.lDeadBand = DI_FFNOMINALMAX * 10 / 100; //nil in first 10% 

            dif.dwSize  = sizeof dif;
            dif.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
            dif.dwGain  = DI_FFNOMINALMAX;
            dif.dwDuration = INFINITE;
            dif.dwTriggerButton = DIEB_NOTRIGGER;
            dif.cAxes = 1; // even if y has FF, only centre x axis
            dif.rgdwAxes = axes;
            dif.rglDirection = dir;
            dif.lpEnvelope = NULL;
            dif.cbTypeSpecificParams = sizeof dic;
            dif.lpvTypeSpecificParams = &dic;
            if (SUCCEEDED(joystick->CreateEffect(GUID_Spring, &dif,
             &centre, NULL)))
                 centre->Download();

            // triggered force effect - tied directly to the 
            // selected trigger button
            //
            // create user-selected effect, associate it with the 
            // trigger button; designed to shake a steering wheel
            DIPERIODIC dip;
            DIENVELOPE die;

            // if both x & y have FF, make shake come at 45 degree angle
            // otherwise just shake the x axis.
            //
            if (axisIsActive[0] && axisIsActive[1]) {
                dif.dwFlags = DIEFF_POLAR | DIEFF_OBJECTOFFSETS;
                dif.cAxes = 2;
                dir[0] = 45 * DI_DEGREES;
            }
            else { // assume only x axis has FF
                dif.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
                dif.cAxes = 1;
            }
            dif.dwGain = DI_FFNOMINALMAX;
            dif.dwDuration = 3 * DI_SECONDS / 2; // 1.5 seconds;
            dif.dwSamplePeriod = 0;
            dif.dwTriggerButton = DIJOFS_BUTTON(trgButton);
            dif.dwTriggerRepeatInterval = 0;
            dif.lpEnvelope = &die;
            dif.cbTypeSpecificParams = sizeof dip;
            dif.lpvTypeSpecificParams = &dip;
            dif.dwStartDelay = 0;

            dip.dwMagnitude = 3 * DI_FFNOMINALMAX / 10;
            dip.lOffset  = 0;
            dip.dwPhase  = 0;
            dip.dwPeriod = DI_SECONDS / 10; // 1/10th second

            die.dwSize = sizeof die;
            die.dwAttackLevel = 0;
            die.dwAttackTime  = DI_SECONDS / 2;
            die.dwFadeLevel   = 0;
            die.dwFadeTime    = DI_SECONDS / 2;

            if (trgFrGuid != GUID_NULL) {
                joystick->CreateEffect(trgFrGuid, &dif, &trgForce, 
                 NULL);
                // download this effect to the driver
                trgForce->Download();
                // don't start this effect until trigger is pulled
            }

            // create a ramp force
            //
            DIRAMPFORCE dirf;

            // if both x & y have FF, make shake come at 45 degree angle
            // otherwise just shake the x axis.
            //
            if (axisIsActive[0] && axisIsActive[1]) {
                dif.dwFlags = DIEFF_POLAR | DIEFF_OBJECTOFFSETS;
                dif.cAxes = 2;
                dir[0] = 45 * DI_DEGREES;
            }
            else { // assume only x axis has FF
                dif.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
                dif.cAxes = 1;
            }
            dif.dwGain = DI_FFNOMINALMAX;
            dif.dwDuration = 4 * DI_SECONDS; // 4 seconds;
            dif.dwSamplePeriod = 0;
            dif.dwTriggerButton = DIEB_NOTRIGGER;
            dif.dwTriggerRepeatInterval = 0;
            dif.lpEnvelope = &die;
            dif.cbTypeSpecificParams = sizeof dirf;
            dif.lpvTypeSpecificParams = &dirf;
            dif.dwStartDelay = 0;

            dirf.lStart = DI_FFNOMINALMAX / 2;
            dirf.lEnd   = - DI_FFNOMINALMAX / 2;

            die.dwSize = sizeof die;
            die.dwAttackLevel = DI_FFNOMINALMAX;
            die.dwAttackTime  = DI_SECONDS;
            die.dwFadeLevel   = DI_FFNOMINALMAX / 10;
            die.dwFadeTime    = DI_SECONDS;

            if (SUCCEEDED(joystick->CreateEffect(GUID_RampForce, &dif, 
             &force[noForceEffects], NULL)))
                // Download the force effect to the driver but don't 
                // play it
                force[noForceEffects]->Download();
            noForceEffects++;
        }
		
		rc = true;
    }

    return rc;
}

// update retrieves the current state of the controller and stores
// the axes' and button values for subsequent polling by handle(),
// trigger() and pressed()
//
void Joystick::update() {

    HRESULT hr;
    DIJOYSTATE2 state;

    incx = incy = incz = rotz = 0;

    if (joystick) {
        // make the current state available
        joystick->Poll();
        // retrieve the state of the controller
        hr = joystick->GetDeviceState(sizeof(DIJOYSTATE2), &state);
        if (DIERR_INPUTLOST == hr && SUCCEEDED(joystick->Acquire()))
            hr = joystick->GetDeviceState(sizeof(DIJOYSTATE2), &state);
        if (SUCCEEDED(hr)) {
            // current state components
            if (axisIsActive[0])
                incx = state.lX;
            if (axisIsActive[1])
                incy = reversey ? -state.lY : state.lY;
            if (axisIsActive[2])
                incz = state.lZ;
            if (axisIsActive[3])
                rotz = state.lRz;
            if (povIsActive)
                for (int i = 0; i < 4; i++)
                    pointOfView[0] = state.rgdwPOV[i];
            // buttons currently pressed
            for (int i = 0; i < JOY_BUTTONS; i++)
                button[i] = (state.rgbButtons[i] & 0x80) != 0;
        }
    }
}

// update sets the centering spring to percent of maximum restraint
//
void Joystick::update(int percent) {

    if (centre) {
        DICONDITION dic;
        dic.lOffset = 0;
        dic.lPositiveCoefficient = percent * DI_FFNOMINALMAX / 100;
        dic.lNegativeCoefficient = percent * DI_FFNOMINALMAX / 100;
        dic.dwPositiveSaturation = DI_FFNOMINALMAX;
        dic.dwNegativeSaturation = DI_FFNOMINALMAX;
        dic.lDeadBand = DI_FFNOMINALMAX * 10 / 100; //nil in first 5% 

        DIEFFECT dif;
        ZeroMemory(&dif, sizeof dif);
        dif.dwSize = sizeof dif;
        dif.cbTypeSpecificParams = sizeof dic;
        dif.lpvTypeSpecificParams = &dic;

        centre->SetParameters(&dif, DIEP_TYPESPECIFICPARAMS);
        centre->Start(1,0);
    }
}

// handle returns the current position/orientation of joystick handle
//
void Joystick::handle(int& ix, int& iy, int& iz, int& rz) const {

    ix = incx;
    iy = incy;
    iz = incz;
    rz = rotz;
}

// trigger returns the current state of the trigger button
//
bool Joystick::trigger() const {

    return button[trgButton];
}

// pressed returns true if ControllerButton is pressed, false otherwise
//
bool Joystick::pressed(ControllerButton b) const {

    bool rc = false;

    switch (b) {
      case BUTTON_1:  rc = button[0]; break;
      case BUTTON_2:  rc = button[1]; break;
      case BUTTON_3:  rc = button[2]; break;
      case BUTTON_4:  rc = button[3]; break;
      case BUTTON_5:  rc = button[4]; break;
      case BUTTON_6:  rc = button[5]; break;
      case BUTTON_7:  rc = button[6]; break;
      case BUTTON_8:  rc = button[7]; break;
      case BUTTON_9:  rc = button[8]; break;
      case BUTTON_10: rc = button[9]; break;
    }

    return rc;
}

// applyForce starts applying feedback force i on the controller
//
void Joystick::applyForce(int i) {

    if (i >= 0 && i < noForceEffects && force[i])
        force[i]->Start(1, 0);
}

// suspends unacquires the joystick in preparation for loss of focus
//
void Joystick::suspend() {

    // unload the force feedback effects
    if (centre)   centre->Unload();
    if (trgForce) trgForce->Unload();
    for (int i = 0; i < noForceEffects; i++)
        force[i]->Unload();

	// detach the controller
    if (joystick) joystick->Unacquire();
}

// restore re-acquires the joystick
//
bool Joystick::restore() {

    bool rc = false;

    if (joystick) {
		HRESULT hr = joystick->Acquire();
		if (hr != S_OK && hr != S_FALSE && hr != DIERR_OTHERAPPHASPRIO) {
            release();
            error("Joystick::70 Failed to re-acquire the joystick");
        }
        else {
            if (centre) {
                centre->Download();
                centre->Start(1, 0);
            }
            for (int i = 0; i < noForceEffects; i++)
                if (force[i]) {
                    force[i]->Download();
                }
            if (trgForce) {
                // force effect must be downloaded again
                trgForce->Download();
            }
            rc = true;
        }
    }

    return rc;
}

// release suspends the joystick and then detaches its interface
//
void Joystick::release() {

    suspend();
	if (joystick) {
        if (centre) {
            centre->Release();
            centre = NULL;
        }
        for (int i = 0; i < noForceEffects; i++)
            if (force[i]) {
                force[i]->Release();
                force[i] = NULL;
            }
        if (trgForce) {
            trgForce->Release();
            trgForce = NULL;
        }
        joystick->Release();
        joystick = NULL;
    }
    noForceEffects = 0;
    axisIsActive[0] = axisIsActive[1] = axisIsActive[2] 
	 = axisIsActive[3] = false;
    for (int i = 0; i < JOY_BUTTONS; i++)
        button[i] = false;
}

// destructor releases the controller and detaches the interface to the
// Direct Input object
//
Joystick::~Joystick() {

    release();
    if (di) {
        di->Release();
        di = NULL;
    }
	hwnd = NULL;
}
