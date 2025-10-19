#ifndef _I_INPUT_H_
#define _I_INPUT_H_

/* Interface to the Input Devices
 *
 * consists of IKeyboard interface
 *             IMouse interface
 *             IJoystick interface
 *
 * Input.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

//------------------------------- IKeyboard ------------------------------
//
class IKeyboard {
  public:
    virtual bool setup()              = 0;
    virtual void update()             = 0;
    virtual bool pressed(int k) const = 0;
    virtual void suspend()            = 0;
    virtual bool restore()            = 0;
    virtual void release()            = 0;
    virtual void Delete()             = 0;
};

extern "C"
IKeyboard* CreateKeyboard(void*);

//------------------------------- IMouse ---------------------------------
//
typedef enum Button {
    LEFT_BUTTON,
    RIGHT_BUTTON
} Button;

class IMouse {
  public:
    virtual bool setup()                                             = 0;
    virtual void update()                                            = 0;
    virtual int  pressed(Button b) const                             = 0;
    virtual void displacement(int& d_xx, int& d_yy, int& r_xx) const = 0;
	virtual void displacementForMyGame(int& disp_xx, int& disp_yy) const  = 0;
    virtual void suspend()                                           = 0;
    virtual bool restore()                                           = 0;
    virtual void release()                                           = 0;
    virtual void Delete()                                            = 0;
};

extern "C"
IMouse* CreateMouse(void*);

//---------------------------------- IJoystick ---------------------------
//
typedef enum ControllerButton {
    TRIGGER,
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_4,
    BUTTON_5,
    BUTTON_6,
    BUTTON_7,
    BUTTON_8,
    BUTTON_9,
    BUTTON_10,
    NORTH,
    EAST,
    WEST,
    SOUTH
} ControllerButton;

class IJoystick {
  public:
    virtual bool setup()                                          = 0;
    virtual void update()                                         = 0;
	virtual void update(int percent)                              = 0;
	virtual void applyForce(int i)                                = 0;
    virtual void handle(int& xx, int& yy, int& zz, int& rz) const = 0;
    virtual bool trigger() const                                  = 0;
    virtual bool pressed(ControllerButton b) const                = 0;
    virtual void suspend()                                        = 0;
    virtual bool restore()                                        = 0;
    virtual void release()                                        = 0;
    virtual void Delete()                                         = 0;
};

extern "C"
IJoystick* CreateJoystick(void*);

#endif
