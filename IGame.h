#ifndef _I_GAME_H_
#define _I_GAME_H_

/* Interface to the Game Supervisor
 *
 * IGame.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

class ICameras;


//-------------------------------- IGame ---------------------------------
//
// IGame is the interface to the game supervisor
//
class IGame {
  public:
    virtual bool setup()                                           = 0;
	virtual bool reset()                                           = 0;
    // event loop
    virtual bool isActive() const                                  = 0;
	virtual bool isSetup() const                                   = 0;
    virtual void wndResize()                                       = 0;
    virtual void update()                                          = 0;
    // focus management
    virtual void suspend()                                         = 0;
    virtual void restore()                                         = 0;
	// destruction
    virtual void Delete()                                          = 0;
	virtual ICameras* returnC() = 0;
	
	
	
};

extern "C"
IGame* CreateGame(void* hinst, int show);
extern "C"
IGame* GameAddress();

#endif