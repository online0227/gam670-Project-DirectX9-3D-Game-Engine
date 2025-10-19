#ifndef _INPUT_H_
#define _INPUT_H_

/* Header for the Input DeviceSet
 *
 * consists of Keyboard declaration
 *             Mouse declaration
 *             Joystick declaration
 *
 * Input.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#define  WIN32_LEAN_AND_MEAN
#define  INITGUID   // enables access to GUIDs used by DirectInput
#include <dinput.h> // for basic Direct Input
#undef   INITGUID

#include "IInput.h" // for IKeyboard, IMouse, IJoystick

//------------------------------- Keyboard -----------------------------------
//
class Keyboard : public IKeyboard {

	static const int SIZE_KEYBOARD_PACKET = 256;

    HWND hwnd;                      // points to the main application window 
    LPDIRECTINPUT8 di;              // points to the Direct Input object
    LPDIRECTINPUTDEVICE8 keyboard;  // points to the Direct Input Keyboard

	bool key[SIZE_KEYBOARD_PACKET]; // complete set of keys

    Keyboard(void* hinstance);
	Keyboard(const Keyboard& k);            // prevents copying
	Keyboard& operator=(const Keyboard& k); // prevents assignment
    virtual ~Keyboard();

  public:
	friend IKeyboard* CreateKeyboard(void*);
    bool   setup();
    void   update();
    bool   pressed(int k) const;
    void   suspend();
    bool   restore();
    void   release();
	void   Delete() { delete this; }
};

//------------------------------- Mouse --------------------------------------
//
class Mouse : public IMouse {

    HWND hwnd;                  // points to the main application window 
    LPDIRECTINPUT8 di;          // points to the Direct Input object
    LPDIRECTINPUTDEVICE8 mouse; // points to the Direct Input mouse

	int disp_x;                 // right, left movement
    int disp_y;                 // ahead, back movement
    int rot_x;                  // wheel rotation
    int left;                   // left button presses
    int right;                  // right button presses

    Mouse(void* hinstance);
	Mouse(const Mouse& m);            // prevents copying
	Mouse& operator=(const Mouse& m); // prevents assignment
    virtual ~Mouse();

  public:
	friend IMouse* CreateMouse(void*);
    bool   setup();
    void   update();
    int    pressed(Button b) const;
    void   displacement(int& disp_xx, int& disp_yy, int& rot_xx) const;
	void displacementForMyGame(int& disp_yy, int& disp_xx) const;
    void   suspend();
    bool   restore();
    void   release();
	void   Delete() { delete this; }
};

//---------------------------------- Joystick --------------------------------
//
class Joystick : public IJoystick {

	static const int JOY_BUTTONS       = 128;
	static const int MAX_FORCE_EFFECTS = 20;

    HWND hwnd;                     // points to the main application window 
    LPDIRECTINPUT8 di;             // points to the Direct Input object
    LPDIRECTINPUTDEVICE8 joystick; // points to the Direct Input joystick

    GUID guid;                     // guid of the selected joystick device
    bool axisIsActive[4];          // is axis active?
    bool povIsActive;              // point of view is active?
    int  pointOfView[4];           // point of view
    bool reversey;                 // is the y-axis reversed?
    bool zAxisOn;                  // wish to use the z-axis?
    int  incx;                     // displacement along joystick x axis
    int  incy;                     // displacement along jotstick y axis
    int  incz;                     // displacement along jotstick y axis
    int  rotz;                     // rotation about joystick z axis
    long trgButton;                // index of the trigger button

    bool fBackOn;                  // feedback flag
    LPDIRECTINPUTEFFECT centre;    // self-centering spring
    int noForceEffects;                           // number of force effects
    LPDIRECTINPUTEFFECT force[MAX_FORCE_EFFECTS]; // force effects
    LPDIRECTINPUTEFFECT trgForce;                 // triggered force
    GUID trgFrGuid;                // GUID for triggered force effect

    bool button[JOY_BUTTONS];      // state of each joysick button

    Joystick(void* hinstance);
	Joystick(const Joystick& j);            // prevents copying
	Joystick& operator=(const Joystick& j); // prevents assignment
    virtual ~Joystick();

  public:
	friend IJoystick* CreateJoystick(void*);
    bool   setup();
    void   update();
	void   update(int percent);
	void   applyForce(int i);
    void   handle(int& xx, int& yy, int& zz, int& rz) const;
    bool   trigger() const;
    bool   pressed(ControllerButton b) const;
    void   suspend();
    bool   restore();
    void   release();
	void   Delete() { delete this; }
};

#endif
