/* HUD Module Implementation
 *
 * HUD.cpp
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "IInput.h"        // for Keyboard interface
#include "IGraphicsCard.h" // for Font interface
#include "math.h"          // for Vector
#include "UISettings.h"    // for TL_??, HUD_MOVE
#include "Utilities.h"     // for error()
#include "HUD.h"           // for HUD and Text class declarations

//-------------------------------- HUD -----------------------------------
//
// HUD manages the feedback component of the user interface
//
// The Heads Up Display manages the information to be superimposed on the
// display after the drawing of the scene has been completed
//
IHUD* CreateHUD(IKeyboard* k) {

	return new HUD(k);
}

HUD::HUD(IKeyboard* k) : keyboard(k) {

	// initialize all of the available text items
	noTexts = 0;
    for (int i = 0; i < MAX_TEXTS; i++)
        text[i] = NULL;

	tlx   = HUD_SPRITE_X;
	tly   = HUD_SPRITE_Y;
	rw    = HUD_SPRITE_W;
	rh    = HUD_SPRITE_H;
	file_ = HUD_SPRITE_IMAGE;
	validate();
    lastUpdate = 0;
	lastToggle = 0;
    on         = false;
    Text::hud  = this;
}

// validate keeps the hud parameters within the bounds of the
// display area
//
void HUD::validate() {

	if (tlx < TL_MIN) 
		tlx = TL_MIN;
    else if (tlx > TL_MAX) 
		tlx = TL_MAX;
    if (tly < TL_MIN) 
		tly = TL_MIN;
    else if (tly > TL_MAX) 
		tly = TL_MAX;
    if (rw < R_MIN) 
		rw = R_MIN;
    else if (rw + tlx > TL_MAX && rw < TL_MAX) 
		tlx = TL_MAX - rw;
    else if (rw + tlx > TL_MAX) 
		rw = TL_MAX - tlx;
    if (rh < R_MIN) 
		rh = R_MIN;
    else if (rh + tly > TL_MAX && rh < TL_MAX) 
		tly = TL_MAX - rh;
    else if (rh + tly > TL_MAX) 
		rh = TL_MAX - tly;
}

// add adds to the heads up display a pointer to text *t
// and returns true if successful, false otherwise
//
bool HUD::add(IText* t) {

    int i;
    bool rc = false;

    for (i = 0; i < noTexts; i++)
        if (!text[i]) {
            text[i] = t;
            i = noTexts + 1;
            rc = true;
        }
    if (i == noTexts && noTexts < MAX_TEXTS) {
        text[noTexts++] = t;
        rc = true;
    }

    return rc;
}

// setup sets the time of the last update
//
bool HUD::setup(int now) {

    lastUpdate = now;
	lastToggle = now;

    return true;
}

// position returns the absolute position of the top left corner of
// the hud on a width x height display
//
Vector HUD::position(int width, int height) const {

	return Vector(width * tlx, height * tly, 0);
}

// update toggles the on/off status of the hud if the user pressed 'F'
// and translates the hud if the user pressed 'G', 'D', 'E' or 'V'
//
void HUD::update(int now) {

	int dx = 0, dy = 0;
	int delta = now - lastUpdate;
	lastUpdate = now;

	// update the status for the HUD
	if (keyboard->pressed('F') && now - lastToggle > LATENCY) {
        on         = !on;
        lastToggle = now;
    }

	// translate the hud
	if (keyboard->pressed('G'))
		dx += delta;
//	if (keyboard->pressed('D'))
//		dx -= delta;
	if (keyboard->pressed('E'))
		dy -= delta;
	if (keyboard->pressed('V'))
		dy += delta;
	tlx += dx / HUD_MOVE;
	tly += dy / HUD_MOVE;
	validate();
}

// draw draws all of the text items within the heads up display
//
void HUD::draw() {

	// draw the text items
	IFont* font;
	for (int i = 0; i < noTexts; i++) {
		font = text[i]->font();
		if (font) font->draw(text[i]->text());
	}
}

// suspend suspends the font for each text item in the hud
//
void HUD::suspend() {

    IFont* font;
    for (int i = 0; i < noTexts; i++) {
        font = text[i]->font();
        if (font) font->suspend();
    }
}

// restore re-initializes the time of the last update and restores
// the font for each text item in the hud
//
bool HUD::restore(int now) {

    bool rc = false;

    lastUpdate = now;

    IFont* font;
    for (int i = 0; i < noTexts && rc; i++) {
        font = text[i]->font();
        if (font) 
            rc = font->restore();
    }

    return rc;
}

// remove removes the pointer to text *t from the heads up display and
// returns true if successful, false otherwise
//
bool HUD::remove(IText* t) {

    bool rc = false;

    for (int i = 0; i < noTexts; i++)
        if (text[i] == t) {
            text[i] = NULL;
            rc = true;
        }
    while (!text[noTexts - 1])
        noTexts--;

    return rc;
}

// release releases the font for each text item in the hud
//
void HUD::release() {

    IFont* font;
    for (int i = 0; i < noTexts; i++) {
        font = text[i]->font();
        if (font) font->release();
    }
}

// destructor destroys all of the text items
//
HUD::~HUD() {

    suspend(); 

    // destroy each text item
	for (int i = 0; i < noTexts; i++)
        if (text[i]) text[i]->Delete();

    Text::hud = NULL;
}

//-------------------------------- Text --------------------------------------
//
// Text is a text item within the heads up display
//
IText* CreateText(float x, float y, float xx, float yy, unsigned f, 
 const char* str) {

	return new Text(x, y, xx, yy, f, str);
}

HUD* Text::hud  = NULL;

Text::Text(float x, float y, float xx, float yy, unsigned flags, 
 const char* str) :  tlx(x), tly(y), brx(xx), bry(yy) {

    font_ = CreateFont_(this, flags);
    text_ = NULL;

    if (tlx < TL_MIN) tlx = TL_MIN;
    else if (tlx > TL_MAX) tlx = TL_MAX;
    if (tly < TL_MIN) tly = TL_MIN;
    else if (tly > TL_MAX) tly = TL_MAX;
    if (brx < TL_MIN) brx = TL_MIN;
    else if (brx > TL_MAX) brx = TL_MAX;
    if (bry < TL_MIN) bry = TL_MIN;
    else if (bry > TL_MAX) bry = TL_MAX;

	if (str) set(str);

	if (!hud)
        error("Text::00 HUD is inaccessible");
    else if (!hud->add(this))
        error("Text::01 Could not add text to the HUD");
}

// topLeftX returns the relative top left x coordinate of the text
//
float Text::topLeftX() const { 
	
	return hud->topLeftX() + hud->width() * tlx; 
}

// topLeftY returns the relative top left y coordinate of the text
//
float Text::topLeftY() const { 
	
	return hud->topLeftY() + hud->height() * tly; 
}

// bottomRightX returns the relative bottom right x coordinate of the text
//
float Text::bottomRightX() const { 
	
	return hud->topLeftX() + hud->width() * brx; 
}

// bottomRightY returns the relative bottom right y coordinate of the text
//
float Text::bottomRightY() const { 
	
	return hud->topLeftY() + hud->height() * bry; 
}

// set copies str[] into the Text object
//
void Text::set(const char* str) {

    int len = strlen(str);
    if (text_)
        delete [] text_;
    text_ = new char[len + 1];
    strcopy(text_, str, len);
}

// destructor destroys the font associated with this text object
// and the text stored in the object and removes the text object
// from the HUD
//
Text::~Text() {

    if (font_)
        font_->Delete();
    if (text_)
        delete [] text_;
    if (hud)
        hud->remove(this);
    else
        error("Text::90 HUD is inaccessible");
}
