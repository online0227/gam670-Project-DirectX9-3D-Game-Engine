/* Game Implementation - supervises all of the game components 
 *                       
 * Game.cpp
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "IGame.h"
#include "IConfiguration.h" // for WindowAddress()
#include "IInput.h"         // for Keyboard, Mouse, Joystick interfaces
#include "IHUD.h"           // for HUD interface
#include "IScene.h"         // for Scene interface
#include "ICameras.h"       // for Cameras interface
#include "ILighting.h"      // for Lighting interface
#include "IAudio.h"         // for Audio interface
#include "IGraphicsCard.h"  // for GraphicsCard interface
#include "ISoundCard.h"     // for SoundCard interface
#include "Utilities.h"      // for error()
#include "math.h"           // for Vector
#include "Game.h"           // for Game and Timer class declarations
#include <new>              // for nothrow
#include <mmsystem.h>       // for timeGetTime()

// timing factors
#define FPS_MAX        200  // fps maximum
#define UNITS_PER_SEC 1000  // time units in a second

//-------------------------------- Game -----------------------------------
//
// Game supervises all of the components of the game
//
IGame* Game::address_ = NULL;

// CreateGame creates the Game object unless it already exists
// in which case, only returns its address
//
IGame* CreateGame(void* hinst, int show) {

    return GameAddress() ? GameAddress() :
     new (std::nothrow) Game(hinst, show);
}

// GameAddress returns the address of the Game object
//
IGame* GameAddress() {

    return Game::address();
}

// constructor creates all of the components of the game
//
Game::Game(void* hinst, int show) {

    address_  = this;
	selector  = CreateSelector(hinst);
	window    = CreateGWindow(hinst, show);
    keyboard  = CreateKeyboard(hinst);
    mouse     = CreateMouse(hinst);
    joystick  = CreateJoystick(hinst);
	hud       = CreateHUD(keyboard);
    audio     = CreateAudio(keyboard, mouse, joystick, hud);
    cameras   = CreateCameras(keyboard, mouse, joystick, hud);
    scene     = CreateScene(keyboard, mouse, joystick, audio, hud, cameras);
    lighting  = CreateLighting(keyboard, mouse, joystick);
    display   = CreateDisplay(scene, lighting, hud, cameras); //particle implementation
    soundCard = CreateSoundCard(audio);
	timer     = new Timer(hud);
	isSetup_  = false;
	active    = false;
}

// setup sets up the game in its initial state - this function should
// only be called once
//
bool Game::setup() {

    bool rc = false;

    // this function should only be called once
    if (isSetup_)
        error("Game::10 setup has been called before");

	// ask the user to select the initial configuration
	else if(!selector->select()) 
		error("Game::11 Quit application before starting");

	// setup the window
	else if(!window->setup()) 
		error("Game::12 Failed to setup the window");

	else {
		int now = timeGetTime();

		// setup the mouse device - optional
		if (!mouse->setup())
			error("Game::13 Unable to setup the mouse device");

		// setup the joystick device - optional
		if (!joystick->setup())
			error("Game::14 Unable to setup the joystick device");

		// setup the keyboard device - necessary
		if (!keyboard->setup())
			error("Game::15 Unable to setup the keyboard device");

		// setup the camera - necessary
		else if(!cameras->setup(now))
			error("Game::17 Failed to setup the Camera");

		// setup the scene - necessary
		else if (!scene->setup(now))
			error("Game::16 Failed to setup the Scene");

		// setup the global light sources - necessary
		else if(!lighting->setup(now))
			error("Game::18 Failed to setup the Lighting");

		// setup the global audio sources - necessary
		else if(!audio->setup(now))
			error("Game::19 Failed to setup the Audio");

		// setup the heads up display - necessary
		else if (!hud->setup(now))
			error("Game::20 Unable to set up the heads up display");

		// setup the display device - necessary
		else if (!display->setup())
			error("Game::21 Unable to set up the Display");

		// setup the sound card - necessary
		else if (!soundCard->setup())
			error("Game::22 Unable to set up the Sound Card");

		else {
			timer->setup(now);
			active   = true;
			isSetup_ = true;
			rc = true;
		}
	}

    return rc;
}

// wndResize handles resizing of the window on the display object
//
void Game::wndResize() {

    if (active) display->wndResize();
}

// update updates the components, draws a frame of the game
// and starts/stops playing any pending sound effect
//
void Game::update() {

	int now = timeGetTime();
	if (active && timer->sufficient(now)) { 
		keyboard->update();
		mouse->update();
		joystick->update();
		scene->update(now);
		cameras->update(now);
		lighting->update(now);
		audio->update(now);
		timer->update(now);
		hud->update(now);
		const Vector& p = cameras->position();
		const Vector& h = cameras->heading();
		const Vector& t = cameras->top();
		display->draw(p, h, t);
		soundCard->play(p, h, t);
	}
}

// suspend prepares each component for deactivation
//
void Game::suspend() {

    audio->suspend();
    scene->suspend();
    display->suspend();
    joystick->suspend();
    mouse->suspend();
    keyboard->suspend();
	active = false;
}

// restore restores each component
//
void Game::restore() {

    keyboard->restore();
    mouse->restore();
    joystick->restore();
	display->restore();
	int now = timeGetTime();
	cameras->restore(now);
	scene->restore(now);
	audio->restore(now);
	lighting->restore(now);
	timer->restore(now);
	hud->restore(now);
	active = true;
}

// reset resets each components prior to re-activation
//
bool Game::reset() {

	bool rc = false;

	// reset the keyboard device
    if (!keyboard->setup())
        error("Game::80 Unable to set up the keyboard device");

	// reset the mouse device
    if (!mouse->setup())
        error("Game::81 Unable to set up the mouse device");

	// reset the joystick device
    if (!joystick->setup())
        error("Game::82 Unable to set up the joystick device");

	// reset the sound card
    if (!soundCard->setup())
        error("Game::83 Unable to set up the sound card");

    // reset the display device
    if (!display->setup())
        error("Game::84 Unable to set up the display device");

	else {
		int now = timeGetTime();
		timer->restore(now);
		// reset the model components
		cameras->restore(now);
		scene->restore(now);
		lighting->restore(now);
		audio->restore(now);
		// reset the feedback component
		hud->restore(now);
		rc = true;
	}

	return rc;
}

// destructor destroys each component
//
Game::~Game() {

	delete timer;
    joystick->Delete();
    mouse->Delete();
    keyboard->Delete();
    lighting->Delete();
    cameras->Delete();
    scene->Delete();
    audio->Delete();
    soundCard->Delete();
    display->Delete();
	hud->Delete();
	window->Delete();
	selector->Delete();
    address_ = NULL;
}

//---------------------------- Timer ---------------------------------------
//
// Timer keeps track of the frame rate and avoids flicker
//
// constructor initializes 
Timer::Timer(IHUD* h) : hud(h) {

    lastSet      = 0;
    framecount   = 0;
    framesPerSec = 0;
	strcopy(fpsstr, "?? fps", 6);
	text         = NULL;
}

// setup initializes framecount variables
//
void Timer::setup(int now) {

    lastSet      = now;
    framecount   = 0;
    framesPerSec = 0;
    text         = CreateText(0, 0, 0.9f, 0.1f, TEXT_RIGHT, fpsstr);
}

// sufficient reports whether enough time has elapsed to avoid any
// perception of flicker
//
bool Timer::sufficient(int now) const {

    // sufficient time has elapsed only if the elapsed time exceeds the
	// amount of time needed for a frame rate of FPS_MAX; that is, if
    // (timeGetTime() - lastSet)/UNITS_PER_SEC >= 1/FPS_MAX where 
	// FPS_MAX is significantly greater than the flicker fusion threshold
    return (now - lastSet) >= UNITS_PER_SEC/FPS_MAX;
}

// update updates the framecount
//
void Timer::update(int now) {

	// (Re)compute the frame rate once every second or so
    if (now - lastSet > UNITS_PER_SEC) {
        framesPerSec = (framecount * UNITS_PER_SEC) / (now - lastSet);
        framecount   = 0;
        lastSet      = now;
		wsprintf(fpsstr, "%02d fps", framesPerSec);
		if (hud->isOn())
			text->set(fpsstr);
    }
    else
        framecount++;
}

// restore re-initializes the time of the last frame and the frame count
//
void Timer::restore(int now) {

    lastSet    = now;
	framecount = 0;
}

