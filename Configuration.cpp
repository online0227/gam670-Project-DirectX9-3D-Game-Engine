/* Configuration Module Implementation
 *
 * Interface.cpp
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "IGame.h"         // for GameAddress
#include "IGraphicsCard.h" // for Host interface
#include "UISettings.h"    // for WND_WIDTH, WND_HEIGHT, RUN_IN_WINDOW
#include "Utilities.h"     // for error()
#include "Configuration.h" // for Selector and Window class declarations
#include <new>             // for nothrow

//-------------------------------- Selector ------------------------------
//
// Selector collects the configuration information from the user
//
ISelector* Selector::address_ = NULL;

// CreateSelector creates a selector object for application hinst unless
// the object already exists, in which case CreateSelector returns the
// address of the existing object
//
ISelector* CreateSelector(void* hinst) {

    return SelectorAddress() ? SelectorAddress() :
		new (std::nothrow) Selector(hinst);
}

// SelectorAddress returns the address of the selector 
//
ISelector* SelectorAddress() {

    return Selector::address();
}

// constructor retrieves an interface to the DirectInput object
//
Selector::Selector(void* hinstance) : hinst((HINSTANCE)hinstance) {

    address_ = this;

    // retrieve the interface to the DirectInput object
    if (FAILED(DirectInput8Create(hinst, DIRECTINPUT_VERSION,
     IID_IDirectInput8, (void**)&di, NULL)))
        error("Selector::02 Unable to create a Direct Input object");

	yrev    = false;
	zon     = false;
	fBackOn = false;
	but     = 0;
	guid    = GUID_NULL;
	ffguid  = GUID_NULL;

    // initialize previous configuration
	curDisplay[0] = '\0';
	curResolution[0] = '\0';
    prevCtr = 0;
    prevTrg = 0;
	prevEff = 0;
    prevYrv = true;
    prevZon = true;
	prevFon = false;
}

// select displays a DialogBox and obtains the user's selections
//
bool Selector::select() {

    return DialogBox(hinst, MAKEINTRESOURCE(IDD_DLG), NULL, dlgProc) == TRUE;
}

// dlgProc handles each message "msg" to the Dialog Box window "hDbWnd"
//
BOOL CALLBACK dlgProc(HWND hDbWnd, UINT msg, WPARAM wp, LPARAM lp) {

    BOOL rc = FALSE;
    static bool firsttime = true;
    Selector& selector    = (Selector&)*SelectorAddress();

    // Process message msg
    switch (msg) {
      case WM_INITDIALOG:
        // make this window a layered window 
        SetWindowLong(hDbWnd, GWL_EXSTYLE, GetWindowLong(hDbWnd, GWL_EXSTYLE) 
         | WS_EX_LAYERED);
        // make this window 5% transparent
        SetLayeredWindowAttributes(hDbWnd, 0, (255 * 95) / 100, LWA_ALPHA);        
        // populate the display combo box
        rc = selector.populateAdapterList(hDbWnd);
        break;

      case WM_COMMAND:          // user accessed a dialog box control
        switch (LOWORD(wp)) {   // which control?
          case IDC_DISPLAY:     // accessed the display combo box
            // only process this if it is the first time or the user
            // has changed the selection.  This section resets the static
            // variable "firsttime".  Exiting resets it back to true
            if (firsttime && HIWORD(wp) == CBN_SETFOCUS ||
             HIWORD(wp) == CBN_SELCHANGE ) {
                firsttime = false;  // won't be 1st time again for this box!
                // populate resolutions combo box for the selected adapter
                if (rc = selector.populateDisplayModeList(hDbWnd))
                    // safe to activate GO button now
                    EnableWindow(GetDlgItem(hDbWnd, IDC_GO), TRUE);
            } else
                rc = TRUE;
            break;

          case IDC_CONTROLLER:  // user accessed the controller combo box
            if (HIWORD(wp) == CBN_SELCHANGE) {
                // populate the controller objects combo box
                selector.populateControllerObjectList(hDbWnd);
            }
            break;

          case IDC_GO:          // user pressed the Go button
            // configure the main application window and the devices for
			// the current selection and create the main application window
            if (selector.configure(hDbWnd)) {
                // deallocate Dialog Box data
                selector.cleanUp(hDbWnd);
                EndDialog(hDbWnd, TRUE);
                firsttime = true;
                rc = TRUE;
            }
            break;

          case IDCANCEL:        // user pressed Cancel button, or Esc, etc.
            // deallocate Dialog Box data
            selector.cleanUp(hDbWnd);
            EndDialog(hDbWnd, FALSE);
            firsttime = true;
            rc = TRUE;
            break;
        }
        break;
    }
    return rc;
}

// populateAdapterList populates the display and controller combo boxes
// with lists of the installed display adapters and controllers
//
bool Selector::populateAdapterList(HWND hDbWnd) {

    int dev, ctr;                 // index of line item (display, controller)
    char description[MAX_DESC+1]; // user friendly for display

    // create host object
    host = CreateHost(hDbWnd);

    // populate the display combo box with installed display adapters
    //
    int nd = host->adapterCount();
    for (int id = 0; id < nd; id++) {
		// retrieve display description
        if (host->displayDescription(id, description, MAX_DESC)) {
            // add description and index to display combo box
			dev = SendDlgItemMessage(hDbWnd, IDC_DISPLAY, CB_ADDSTRING, 0,
             (LPARAM)description);
            SendDlgItemMessage(hDbWnd, IDC_DISPLAY, CB_SETITEMDATA, dev, 
             id);
        }
    }
    // append a choice to run in a window
    dev = SendDlgItemMessage(hDbWnd, IDC_DISPLAY, CB_ADDSTRING, 0,
     (LPARAM)RUN_IN_WINDOW_DESC);
    SendDlgItemMessage(hDbWnd, IDC_DISPLAY, CB_SETITEMDATA, dev, 
     RUN_IN_WINDOW);
    // set the default display adapter to previous adapter
	dev = SendDlgItemMessage(hDbWnd, IDC_DISPLAY, CB_FINDSTRINGEXACT, -1,
	 (LPARAM)curDisplay);
	if (dev == CB_ERR) dev = 0;
    SendDlgItemMessage(hDbWnd, IDC_DISPLAY, CB_SETCURSEL, dev, 0L);

    // populate the joystick combo box with currently installed controllers
    //
    // start the list with a no controller option
    ctr = SendDlgItemMessage(hDbWnd, IDC_CONTROLLER, CB_ADDSTRING, 0, 
     (LPARAM)"No Controller");
    SendDlgItemMessage(hDbWnd, IDC_CONTROLLER, CB_SETITEMDATA, ctr, NULL);
    // enumerate the installed controllers and populate joystick combo box
    if (SUCCEEDED(di->EnumDevices(DI8DEVCLASS_GAMECTRL,
     (LPDIENUMDEVICESCALLBACK)enumInputDevices,
     (LPVOID)GetDlgItem(hDbWnd, IDC_CONTROLLER), DIEDFL_ATTACHEDONLY))) {
        int nc = SendDlgItemMessage(hDbWnd, IDC_CONTROLLER, CB_GETCOUNT, 
         0, 0L);
        // check if at least one controller is attached, previous selection
        // was a controller, and the previous selection is still in range
        if (nc > 1 && prevCtr != 0 && prevCtr < nc) {
            // then set the default to the previous controller
            SendDlgItemMessage(hDbWnd, IDC_CONTROLLER, CB_SETCURSEL, prevCtr,
             0L);
            // populate the trigger selection list
            populateControllerObjectList(hDbWnd);
            // check 'disable feedback' checkbox
            SendDlgItemMessage(hDbWnd, IDC_NOFF, BM_SETCHECK, prevFon ?
             BST_CHECKED : BST_UNCHECKED, 0L); 
            // set the 'reverse y axis' checkbox to the previous setting
            SendDlgItemMessage(hDbWnd, IDC_YRV, BM_SETCHECK, prevYrv ? 
             BST_CHECKED : BST_UNCHECKED, 0L);
            // set the 'use z axis' checkbox to the previous setting
            SendDlgItemMessage(hDbWnd, IDC_ZON, BM_SETCHECK, prevZon ?
             BST_CHECKED : BST_UNCHECKED, 0L);
            // set force effect selection to the previous setting
            SendDlgItemMessage(hDbWnd, IDC_FF, CB_SETCURSEL, prevEff, 0L);
            // enable the trigger combo box and the check boxes
			if (prevFon) {
				EnableWindow(GetDlgItem(hDbWnd, IDC_TRG), TRUE);
				EnableWindow(GetDlgItem(hDbWnd, IDC_FF), TRUE);
			}
			else {
				EnableWindow(GetDlgItem(hDbWnd, IDC_TRG), FALSE);
				EnableWindow(GetDlgItem(hDbWnd, IDC_FF), FALSE);
			}
            EnableWindow(GetDlgItem(hDbWnd, IDC_NOFF), TRUE);
            EnableWindow(GetDlgItem(hDbWnd, IDC_YRV), TRUE);
            EnableWindow(GetDlgItem(hDbWnd, IDC_ZON), TRUE);
        }else {
            // if not, then set the default to no controller
            SendDlgItemMessage(hDbWnd, IDC_CONTROLLER, CB_SETCURSEL, 0, 0L);
            // check 'disable feedback' checkbox
            SendDlgItemMessage(hDbWnd, IDC_NOFF, BM_SETCHECK, BST_CHECKED, 
             0L); 
            // uncheck the 'reverse y axis' checkbox
            SendDlgItemMessage(hDbWnd, IDC_YRV, BM_SETCHECK, BST_UNCHECKED, 
             0L);
            // uncheck the 'use z axis' checkbox
            SendDlgItemMessage(hDbWnd, IDC_ZON, BM_SETCHECK, BST_UNCHECKED, 
             0L);
            // set force effect selection to the previous setting
            SendDlgItemMessage(hDbWnd, IDC_FF, CB_SETCURSEL, 0, 0L);
            // disable the trigger combo box and the check boxes
            EnableWindow(GetDlgItem(hDbWnd, IDC_TRG), FALSE);
            EnableWindow(GetDlgItem(hDbWnd, IDC_NOFF), FALSE);
            EnableWindow(GetDlgItem(hDbWnd, IDC_FF), FALSE);
            EnableWindow(GetDlgItem(hDbWnd, IDC_YRV), FALSE);
            EnableWindow(GetDlgItem(hDbWnd, IDC_ZON), FALSE);
        }
    }
    else {
		// if enumeration failed, disable joystick combo box
        EnableWindow(GetDlgItem(hDbWnd, IDC_CONTROLLER), FALSE);
        // disable the trigger combo box and the check boxes
        EnableWindow(GetDlgItem(hDbWnd, IDC_TRG), FALSE);
        EnableWindow(GetDlgItem(hDbWnd, IDC_NOFF), FALSE);
        EnableWindow(GetDlgItem(hDbWnd, IDC_YRV), FALSE);
        EnableWindow(GetDlgItem(hDbWnd, IDC_ZON), FALSE);
    }

    return true;
}

// enumInputDevices retrieves and stores the GUID for an enumerated device
// and adds the description of the device to the combobox as a new line item
//
// The memory that is dynamically allocated for the GUID is deallocated
// in Selector::cleanUp()
//
// The 1st parameter holds the information about the device, the 2nd
// parameter receives the window handle for the combo box to which the
// description will be added.
//
BOOL CALLBACK enumInputDevices(LPCDIDEVICEINSTANCE didesc, void* combobox) {

    long device = (long)SendMessage((HWND)combobox, CB_ADDSTRING, 0,
     (LPARAM)didesc->tszInstanceName);
    if (device != CB_ERR) {
        LPGUID pGuid;
        if (pGuid = new GUID)
            *pGuid = didesc->guidInstance;

        SendMessage((HWND)combobox, CB_SETITEMDATA, device, (LPARAM)pGuid);
    }
    return DIENUM_CONTINUE;
}

// populateDisplayModeList populates the resolution combo box with the modes
// available on the selected display adapter
//
bool Selector::populateDisplayModeList(HWND hDbWnd) {

    bool rc = false;

    // empty the resolutions combo box
    SendDlgItemMessage(hDbWnd, IDC_RESOLUTION, CB_RESETCONTENT, 0, 0L);

    // retrieve the index for the selected device
    int dev = SendDlgItemMessage(hDbWnd, IDC_DISPLAY, CB_GETCURSEL, 0, 0L);
    if (dev == CB_ERR)
        error("Selector::30 No adapter selected", hDbWnd);
    else {
        // retrieve the id of the selected display device
        int id = SendDlgItemMessage(hDbWnd, IDC_DISPLAY, CB_GETITEMDATA,
         dev, 0L);
        if (id == RUN_IN_WINDOW) {
			// if window mode, disable resolutions combo box
            EnableWindow(GetDlgItem(hDbWnd, IDC_RESOLUTION), FALSE);
            rc = true;
        }
        else {
            // fullscreen modes
            unsigned fmMd;           // packed pixel format and mode
            bool noModes = true;     // no modes are available?
            char line[MAX_DESC + 1]; // description of mode 
            // populate the resolutions combo box with available modes
            //
            // retrieve the number of supported pixel formats
            int np = host->formatCount();
            for (int ip = 0; ip < np; ip++) { // for each pixel format...
                // retrieve the number of modes for the selected pixel format
                int nr = host->modeCount(id, ip);
                for (int ir = 0; ir < nr; ir++) { // for each mode...
                    // check if selected mode ir is supported on device id
                    if (host->getMode(id, ir, ip, fmMd, line)) {
                        // mode is supported - add it to the combo box
                        int res = SendDlgItemMessage(hDbWnd, IDC_RESOLUTION,
                         CB_ADDSTRING, 0, (LPARAM)line);
                        if (res >= 0) {
                            // store mode|format in data part of line item
                            SendDlgItemMessage(hDbWnd, IDC_RESOLUTION,
                             CB_SETITEMDATA, res, fmMd);
                        }
						// a mode is available
                        noModes = false;
                    }
                }
            }
            if (noModes)
                error("Selector::31 Selected display has no available modes",
                 hDbWnd);
			else {
                // set cursor to the default mode
				int res = SendDlgItemMessage(hDbWnd, IDC_RESOLUTION, 
				 CB_FINDSTRINGEXACT, -1, (LPARAM)curResolution);
				if (res == CB_ERR) res = 0;
				SendDlgItemMessage(hDbWnd, IDC_RESOLUTION, CB_SETCURSEL, 
				 res, 0L);
				// enable the resolutions combo box
				EnableWindow(GetDlgItem(hDbWnd, IDC_RESOLUTION), TRUE);
				rc = true;
			}
        }
    }

    return rc;
}

// populateControllerObjectList populates the trigger combo box with a list
// of the buttons available on the selected controller
//
bool Selector::populateControllerObjectList(HWND hDbWnd) {

    bool rc = false;
    int ctr;         // index of selected controller line item

    // empty the controller objects combo box
    SendDlgItemMessage(hDbWnd, IDC_TRG, CB_RESETCONTENT, 0, 0L);
    // disable all combo boxes and check boxes by default
    EnableWindow(GetDlgItem(hDbWnd, IDC_CONTROLLER), FALSE);
    EnableWindow(GetDlgItem(hDbWnd, IDC_TRG), FALSE);
    EnableWindow(GetDlgItem(hDbWnd, IDC_NOFF), FALSE);
    EnableWindow(GetDlgItem(hDbWnd, IDC_FF), FALSE);
    EnableWindow(GetDlgItem(hDbWnd, IDC_YRV), FALSE);
    EnableWindow(GetDlgItem(hDbWnd, IDC_ZON), FALSE);

    // retrieve the index corresponding to the selected controller
    ctr = SendDlgItemMessage(hDbWnd, IDC_CONTROLLER, CB_GETCURSEL, 0, 0L);
    if (ctr != CB_ERR && di) {
        // retrieve pointer to the GUID for the selected controller
        LPGUID lpguid = (LPGUID)SendDlgItemMessage(hDbWnd, IDC_CONTROLLER,
         CB_GETITEMDATA, ctr, 0);
        // create the device in order to interrogate it for its objects
        LPDIRECTINPUTDEVICE8 did = NULL;
        if (lpguid && SUCCEEDED(di->CreateDevice(*lpguid, &did, NULL))) {
            if (SUCCEEDED(did->SetDataFormat(&c_dfDIJoystick2))) {
                DIDEVICEOBJECTINSTANCE didoi; // holds object info
                DWORD dwOfs;                  // offset for object b
                int defTrg = 0;               // default is first object
                didoi.dwSize = sizeof didoi;
				// examine each object
                for (int b = 0; b < JOY_BUTTONS; b++) {
                    // offset for button b
                    dwOfs = DIJOFS_BUTTON(b);
                    // retrieve information about this button
                    if (SUCCEEDED(did->GetObjectInfo(&didoi, dwOfs,
                     DIPH_BYOFFSET))) {
                        // add the button name to the combo list
                        int trg = SendDlgItemMessage(hDbWnd, IDC_TRG,
                         CB_ADDSTRING, 0, (LPARAM)didoi.tszName);
                        // save the button number in data area of line item
                        SendDlgItemMessage(hDbWnd, IDC_TRG, CB_SETITEMDATA,
                         trg, b);
                        if (ctr == prevCtr && trg == prevTrg) 
							// make default the previously selected object
							defTrg = trg;
                    }
                }
				// set cursor to the default object
                SendDlgItemMessage(hDbWnd, IDC_TRG, CB_SETCURSEL, defTrg, 
                 0L);
            }

            // Feedback
            DIDEVCAPS didcaps;
            didcaps.dwSize = sizeof didcaps;
            if (SUCCEEDED(did->GetCapabilities(&didcaps)) &&
             (didcaps.dwFlags & DIDC_FORCEFEEDBACK) &&
             SUCCEEDED(did->EnumEffects(
             LPDIENUMEFFECTSCALLBACK(enumEffects),
             GetDlgItem(hDbWnd,IDC_FF), DIEFT_PERIODIC))) {
                SendDlgItemMessage(hDbWnd, IDC_FF, CB_SETCURSEL, prevEff,
				 0L);
                EnableWindow(GetDlgItem(hDbWnd, IDC_FF), TRUE);
                EnableWindow(GetDlgItem(hDbWnd, IDC_NOFF), TRUE);
            }

			// done with the controller device
            did->Release();
            // enable the trigger selection and check boxes
            EnableWindow(GetDlgItem(hDbWnd, IDC_CONTROLLER), TRUE);
            EnableWindow(GetDlgItem(hDbWnd, IDC_TRG), TRUE);
            EnableWindow(GetDlgItem(hDbWnd, IDC_YRV), TRUE);
            EnableWindow(GetDlgItem(hDbWnd, IDC_ZON), TRUE);
            rc = true;
        }
    }

    return rc;
}

// enumEffects is called by the operating system for each force
// feedback effect available on the game controller device.  The
// first parameter points to information about the effect, including
// its GUID, the second parameter points to user supplied data, which
// is a handle to the combo box.  This function adds the name of the
// effect as a line item to the combo box along with the address of
// the memory allocated for storing the effect's GUID.
//
BOOL CALLBACK enumEffects(LPCDIEFFECTINFO di, HWND combobox) {

    BOOL rc = DIENUM_CONTINUE;
    int i;

    if ((i = (long)SendMessage(combobox, CB_ADDSTRING, 0,
     (LPARAM)di->tszName)) != CB_ERR) {
        LPGUID pcopy = NULL;

        if (pcopy = new GUID)
            *pcopy = di->guid;

        SendMessage(combobox, CB_SETITEMDATA, i, (LPARAM)pcopy);
    }
    else
        rc = DIENUM_STOP;

    return rc;
}

// configure uses the properties selected by the user to configure the main
// application window and the devices in preparation for creating the window
// and setting up the devices 
//
bool Selector::configure(HWND hDbWnd) {

    bool rc = false;

    //----- display device properties --------------------------------------

    int dev;             // index for the selected display
    int res = 0;         // index for the selected resolution
	unsigned fmMd = 0;
	ir = 0;
 
    // Retrieve the index for the selected device
    dev = SendDlgItemMessage(hDbWnd, IDC_DISPLAY, CB_GETCURSEL, 0, 0L);
    if (dev == CB_ERR)
        error("Selector::50 No adapter selected", hDbWnd);
    else {
        // retrieve the display id for the selected display
        id = SendDlgItemMessage(hDbWnd, IDC_DISPLAY, CB_GETITEMDATA, dev, 
         0L);
        if (id == RUN_IN_WINDOW) {
            width  = WND_WIDTH;
            height = WND_HEIGHT;
			host->configure(fmMd, id, ir, ip, width, height, wndStyle,
			 wndExStyle);
        }
        else {
            // retrieve the index for the selected resolution
            res = SendDlgItemMessage(hDbWnd, IDC_RESOLUTION,
             CB_GETCURSEL, 0, 0L);
            if (res == CB_ERR)
                error("Selector::51 No resolution selected", hDbWnd);
            else {
                // extract the display properties from the mode line item
                fmMd = SendDlgItemMessage(hDbWnd, IDC_RESOLUTION,
                 CB_GETITEMDATA, res, 0L);
				host->configure(fmMd, id, ir, ip, width, height, wndStyle,
				 wndExStyle);
            }
        }
        if (id == RUN_IN_WINDOW) {
            // correct the dimensions to ensure no scene cutoffs
            RECT rect;
            rect.left   = 0;
            rect.top    = 0;
            rect.right  = width;
            rect.bottom = height;
            AdjustWindowRectEx(&rect, wndStyle, FALSE, wndExStyle);
            // approximate height of the caption bar
            titleBar = rect.bottom - rect.top - height;
            width    = rect.right - rect.left;
            height   = rect.bottom - rect.top;
        }
        else
            titleBar   = 0;
		// save this configuration for the next resetting, if any
		SendDlgItemMessage(hDbWnd, IDC_DISPLAY, CB_GETLBTEXT, dev, 
	     (LPARAM)curDisplay);
		SendDlgItemMessage(hDbWnd, IDC_RESOLUTION, CB_GETLBTEXT, res, 
		 (LPARAM)curResolution);
		rc = true;
    }

    //----- joystick device properties -------------------------------------

    int ctr;            // index for the selected controller
	int force;          // index for the selected force effect
    int trg = 0;        // index for the trigger object
	but     = 0;
	guid    = GUID_NULL;
	ffguid  = GUID_NULL;

    // retrieve the index for the selected controller
    ctr = SendDlgItemMessage(hDbWnd, IDC_CONTROLLER, CB_GETCURSEL, 0, 0L);
    if (ctr == CB_ERR)
        error("Selector::54 No controller selected", hDbWnd);
    else if (rc && ctr) {
        // retrieve the pointer to the guid for the selected controller
        LPGUID lpguid = (LPGUID)SendDlgItemMessage(hDbWnd, IDC_CONTROLLER,
         CB_GETITEMDATA, ctr, 0L);
		// store the guid for future retrieval
		guid = lpguid ? *lpguid : GUID_NULL;
        // retrieve the selected trigger button
        trg = SendDlgItemMessage(hDbWnd, IDC_TRG, CB_GETCURSEL, 0, 0L);
        if (trg != CB_ERR)
            but = SendDlgItemMessage(hDbWnd, IDC_TRG, CB_GETITEMDATA, trg, 
             0);
        // retrieve selected direction for y axis
        yrev = SendDlgItemMessage(hDbWnd, IDC_YRV, BM_GETCHECK, 0, 0L) ==
         BST_CHECKED;
        // retrieve selected use of z axis
        zon = SendDlgItemMessage(hDbWnd, IDC_ZON, BM_GETCHECK, 0, 0L) ==
         BST_CHECKED;
        // retrieve feedback flag
        fBackOn = SendDlgItemMessage(hDbWnd, IDC_NOFF, BM_GETCHECK, 0, 0L) !=
          BST_CHECKED;
		// retrieve the index for the selected force effect
		force = SendDlgItemMessage(hDbWnd, IDC_FF, CB_GETCURSEL, 0, 0L);
		if (force == CB_ERR) {
			error("Selector::55 No force effect selected", hDbWnd);
		}
		else {
			// retrieve the pointer to the guid for selected force effect
			LPGUID lpguid = (LPGUID)SendDlgItemMessage(hDbWnd, IDC_FF,
			 CB_GETITEMDATA, force, 0L);
			// store the guid for future retrieval
			ffguid = lpguid ? *lpguid : GUID_NULL;
		}

        // save configuration for next setting, if any
        prevCtr = ctr;
        prevTrg = trg;
        prevYrv = yrev;
        prevZon = zon;
		prevFon = fBackOn;
		prevEff = force;
    }

    return rc;
}

// configureD rerturns through its parameters the display configuration
//
void Selector::configureD(int &w, int &h, int &d, int &m, int &p, int &t)
 const {
	w = width;
	h = height;
	d = id;
	m = ir;
	p = ip;
	t = titleBar;
}

// configureW rerturns through its parameters the window configuration
//
void Selector::configureW(int &w, int &h, int &s, int &e)
 const {
	w = width;
	h = height;
	s = wndStyle;
	e = wndExStyle;
}

// configureJ returns through its parameters the joystick configuration
//
void Selector::configureJ(void** g, long &t, bool &y, bool &z, bool &f,
 void** h) 
 const {

	*g = (void*)&guid;
	t  = but;
	y  = yrev;
	z  = zon;
	f  = fBackOn;
	*h = (void*)&ffguid;
}

// cleanUp deallocates the memory used to store the controller GUIDs,
// empties the compbo boxes, and releases the handle to the reference 
// object before the dialog box ends
//
void Selector::cleanUp(HWND hDbWnd) {

    int nc;

    // deallocate controller GUIDs
    nc = SendDlgItemMessage(hDbWnd, IDC_CONTROLLER, CB_GETCOUNT, 0, 0L);
    for (int i = 0; i < nc; i++) {
        void* p = (void*)SendDlgItemMessage(hDbWnd, IDC_CONTROLLER,
         CB_GETITEMDATA, i, 0);
        if (p)
            delete p;
    }

    // empty the display combo box
    SendDlgItemMessage(hDbWnd, IDC_DISPLAY, CB_RESETCONTENT, 0, 0L);
    // empty the resolutions combo box
    SendDlgItemMessage(hDbWnd, IDC_RESOLUTION, CB_RESETCONTENT, 0, 0L);
    // empty controllers combo box
    SendDlgItemMessage(hDbWnd, IDC_CONTROLLER, CB_RESETCONTENT, 0, 0L);
    // empty controller objects combo box
    SendDlgItemMessage(hDbWnd, IDC_TRG, CB_RESETCONTENT, 0, 0L);

	host->Delete();
}

// destructor releases the pointer to the interface to the DirectInput
// object
//
Selector::~Selector() {

    #if GRAPHICS_API == DIRECT3D
    if (di) {
        di->Release();
        di = NULL;
    }

    #elif GRAPHICS_API == OPENGL
    #endif
}

//------------------------------ Window ------------------------------------
//
// Window manages the window component of the user interface
//
IWindow* Window::address_ = NULL;

// CreateWindow creates a window object for application hinst unless
// the object already exists, in which case CreateWindow returns the address
// of the existing object
//
IWindow* CreateGWindow(void* hinst, int show) {

	IWindow* window = WindowAddress();

	if (!window && registerWindowClass((HINSTANCE)hinst))
		window = new (std::nothrow) Window(hinst, show);

	return window;
}

// WindowAddress returns the address of the selector 
//
IWindow* WindowAddress() {

    return Window::address();
}

Window::Window(void* hinstance, int s) : hinst((HINSTANCE)hinstance), show(s) {

    address_ = this;
    hwnd     = NULL;

    // default configuration parameters
	width      = WND_WIDTH;
	height     = WND_HEIGHT;
	wndStyle   = 0;
	wndExStyle = 0;
}

// create the main application window
//
bool Window::setup() {

	SelectorAddress()->configureW(width, height, wndStyle, wndExStyle);

	// destroy the existing window, if it exists
	if (hwnd) {
        DestroyWindow(hwnd);
		show = SW_SHOW;
	}
    // (re)create the main application window
    if (!(hwnd = CreateWindowEx(wndExStyle, CLASS_NAME, WND_NAME,
     wndStyle, 0, 0, width, height, NULL, NULL, hinst, NULL)))
        error("Window::10 Unable to create application window");
    else {
        // how to display the window initially
        ShowWindow(hwnd, show);
        // sends the first WM_PAINT message to the window
        UpdateWindow(hwnd);
    }

    return hwnd != NULL;
}

// registerWindowClass registers the main application window class
//
bool registerWindowClass(HINSTANCE hinst) {

    // Define the "class" (type) of window we want to have
    // - a necessary step before we can create the main window

    WNDCLASS wc;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = ::wndProc; // this function will be called...
    wc.cbClsExtra = 0;          // ...to handle messages
    wc.cbWndExtra = 0;
    wc.hInstance = hinst;
    wc.hIcon = LoadIcon(hinst, IDI_APPLICATION);
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = CLASS_NAME;

    // Register the "class" for all subsequent calls to create
    // the main application window
    //
    return RegisterClass(&wc) != 0;
}

// wndProc handles the message "msg" sent to main application window "hwnd"
//
LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

    static bool quit     = false;
    IGame&      game     = *GameAddress();
    ISelector&  selector = *SelectorAddress();
	IWindow&    window   = *WindowAddress();

    switch (msg) {
      case WM_CREATE:    // called once when the window is first created
      case WM_SETCURSOR: // called whenever the mouse is moved to ...
        SetCursor(NULL); // ...lets us change the cursor if we want to
        break;

      // Sent when the user switches applications.  If focus was lost,
      // we need to restore the video memory and reload any surfaces.
      // If wp is TRUE, application just reactivated, if wp is FALSE
      // application just deactivated.
      case WM_ACTIVATEAPP:
        // game has been setup and is about to be activated
		if (game.isSetup() && !game.isActive() && wp) {
			game.restore();
		}
        // game is about to be de-activated
        else if (game.isActive() && !wp)
            game.suspend();
        break;

      case WM_KEYDOWN:
        switch (wp) {
          case VK_ESCAPE: 
            // close the main window and send a WM_DESTROY message
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            // prepare to quit
            quit = true;
            break;

          case VK_F1:
            // select new configuration - deactivate and post user message
            PostMessage(hwnd, WM_ACTIVATEAPP, 0, 0);
            PostMessage(hwnd, WM_USER, 0, 0);
            break;
		}
		break;

      case WM_USER:
        // ask the user to reselect options
        if (selector.select()) {
			// setup the new application window
			if (!window.setup())
				error("Window::20 Failed to setup application window");
			// reset the game
			else {
				game.reset();
				SetForegroundWindow((HWND)window.window());
			}
        }
		else
			SetForegroundWindow((HWND)window.window());
        break;

      case WM_SIZE:
        // resize the current window
		game.wndResize();
        break;

	  case WM_CLOSE:
	    quit = true;
      case WM_DESTROY:
        // need to respond to WM_DESTROY for the main window to shutdown properly
        if (quit)
            PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}

// destructor destroys the main application window
//
Window::~Window() {

	if (hwnd)
        DestroyWindow(hwnd);
}

