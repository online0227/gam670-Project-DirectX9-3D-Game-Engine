/* Instructional Game Framework - Top Level
 *
 * TopLevel.cpp
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 *
 * originally based upon the work of Evan Weaver, Seneca College (2005)
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include "IGame.h"     // for CreateGame
#include "Utilities.h" // for error()

// Entry point for the application
//
int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hprev, LPSTR cp, int show) {

    int rc      = 0;
    IGame& game = *CreateGame(hinst, show);

    // has the game been created?
	if (!&game)
        error("TopLevel::00 Failed to create the game");

    // if so, setup the game
    else if (!game.setup())
		error("TopLevel::01 Failed to setup the game");

	else {
		// start the event iteration
		//
        // event iteration: check to see if Windows has a message
        // waiting for us, and if so, then handle it.  Otherwise,
        // make the next frame (provided that the devices are ready 
        // to make that frame).
        rc = 0;
        MSG msg;
        bool done = false;
        do {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    rc = msg.wParam;
                    done = true;
                }
                else {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            else if (game.isActive())
                game.update();
            else
                WaitMessage();  // don't hog CPU while awaiting
                                // reactivation
        } while (!done);
    }

    if (&game)game.Delete();

    return rc;  // msg.wParam on success, 0 on any failure
}
