#ifndef _I_HUD_H_
#define _I_HUD_H_

/* Interface to the HUD Module
 *
 * consists of IHUD interface
 *             IText interface
 *
 * IHUD.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

//-------------------------------- HUD -----------------------------------
//
// IHUD is the interface to the feedback component of the user interface 
//
// The Heads Up Display manages the information to be superimposed on the
// display after the scene has been drawn
//
struct Vector;
class IKeyboard;

class IHUD {
  public:
    virtual bool   setup(int now)                        = 0;
	virtual Vector position(int width, int height) const = 0;
	virtual float  topLeftX() const                      = 0;
	virtual float  topLeftY() const                      = 0;
	virtual float  width() const                         = 0;
	virtual float  height() const                        = 0;
	virtual const  char* file() const                    = 0;
    virtual bool   isOn() const                          = 0;
	virtual void   update(int now)                       = 0;
    virtual void   draw()                                = 0;
    virtual void   suspend()                             = 0;
    virtual bool   restore(int now)                      = 0;
	virtual void   release()                             = 0;
    virtual void   Delete()                              = 0;
};

extern "C"
IHUD* CreateHUD(IKeyboard* k);

//-------------------------------- Text ------------------------------------
//
// Text is a text item on the HUD
//
class IFont;

class IText {
  public:
    virtual IFont* font() const          = 0;
    virtual void   set(const char* text) = 0;
	virtual float  topLeftX() const      = 0;
	virtual float  topLeftY() const      = 0;
	virtual float  bottomRightX() const  = 0;
	virtual float  bottomRightY() const  = 0;
	virtual const  char* text() const    = 0;
	virtual void   Delete()              = 0;
};

extern "C"
IText* CreateText(float x, float y, float xx, float yy, 
 unsigned f = 0, const char* str = 0);

#endif
