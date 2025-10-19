#ifndef _HUD_H_
#define _HUD_H_

/* Header for the HUD Module
 *
 * consists of HUD declaration
 *             Text declaration
 *
 * HUD.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "IHUD.h"

//-------------------------------- HUD -----------------------------------
//
// HUD manages the feedback component of the user interface
//
// The Heads Up Display manages the information to be superimposed on the
// display after the scene has been drawn
//
class HUD : public IHUD {

	static const unsigned MAX_TEXTS = 20;

    IKeyboard* keyboard;    // points to the Keyboard object
    IText* text[MAX_TEXTS]; // points to each text item on the HUD

    int   noTexts;     // number of text items
    int   lastUpdate;  // last time display status was updated
	int   lastToggle;  // last time display status was toggled
    bool  on;          // is HUD being displayed?

	// relative coordinates [0.0 -> 1.0] with respect to client area
	float tlx;         // horizontal top left coordinate
    float tly;         // vertical top left coordinate
    float rw;          // width
    float rh;          // height
    const char* file_; // points to hud background image file

    HUD(IKeyboard* k);
    HUD(const HUD& h);            // prevents copying
    HUD& operator=(const HUD& h); // prevents assignment
    bool    add(IText* t); 
    bool    remove(IText* t); 
    virtual ~HUD();

	void validate();

  public:
	friend IHUD* CreateHUD(IKeyboard* k);
    bool   setup(int now);
	Vector position(int width, int height) const;
	float  topLeftX() const { return tlx; }
	float  topLeftY() const { return tly; }
	float  width() const { return rw; }
	float  height() const { return rh; }
    bool   isOn() const { return on; }
	const  char* file() const { return file_; }
	void   update(int now);
    void   draw();
    void   suspend();
    bool   restore(int now);
	void   release();
	void   Delete() { delete this; }
	friend class Text;
};

//-------------------------------- Text ------------------------------------
//
// Text holds the text for a single item on the HUD
//
class Text : public IText {

    static HUD* hud;  // points to the Heads Up Display object
    IFont* font_;     // points to the font used to display this object
    char*  text_;     // points to the text string

	// relative coordinates [0 -> 1] within the hud
	float  tlx;      // horizontal top left
    float  tly;      // vertical top left
    float  brx;      // horizontal bottom right
    float  bry;      // vertical bottom right

    Text(float x, float y, float xx, float yy, unsigned f, 
	 const char* str);
    Text(const Text& v);
    Text& operator=(const Text& v);
    virtual ~Text();

  public:
	friend IText* CreateText(float x, float y, float xx, float yy, 
	 unsigned f, const char* str);
    IFont* font() const { return font_; }
    void   set(const char* text);
	float  topLeftX() const;
	float  topLeftY() const;
	float  bottomRightX() const;
	float  bottomRightY() const;
	const  char* text() const { return text_; }
	void   Delete() { delete this; }
    friend class HUD;
};

#endif
