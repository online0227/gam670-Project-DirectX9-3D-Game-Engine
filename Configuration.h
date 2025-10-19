#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

/* Header for the Configuration Module
 *
 * consists of Selector declaration
 *             Window declaration
 *
 * Configuration.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include <dinput.h>
#include "IConfiguration.h"

//-------------------------------- Selector ------------------------------
//
// Selector collects the configuration information from the user
//
class IHost;

class Selector : public ISelector {

	static const int MAX_DESC    = 512;
	static const int JOY_BUTTONS = 128;

    static ISelector* address_; // points to this object itself
    LPDIRECTINPUT8    di;       // points to the Direct Input object
    HINSTANCE         hinst;    // points to the current application
	IHost*            host;     // points to the host system object

    // description of current configuration
    int id;              // selected display id
    int ir;              // selected resolution id
    int ip;              // selected pixel format id
    int width;           // selected width of client area
    int height;          // selected height of client area
    int titleBar;        // size of title bar
    int wndStyle;        // selected window style
    int wndExStyle;      // selected window style
	GUID guid;           // guid for the selected controller
	GUID ffguid;         // guid for the selected force effect
    int  but;            // button id for the trigger object
    bool yrev;           // reverse y-axis?
    bool zon;            // z-axis on?
	bool fBackOn;        // force feedback on?
	char curDisplay[MAX_DESC + 1];
	char curResolution[MAX_DESC + 1];
    int  prevCtr;
    int  prevTrg;
	int  prevEff;
    bool prevYrv;
    bool prevZon;
	bool prevFon;

    Selector(void* hinstance);
	Selector(const Selector&);
	Selector& operator=(const Selector&);
    virtual ~Selector();

  public:
    friend ISelector* CreateSelector(void* hinst);
    static ISelector* address() { return address_; }
    bool   select();
    bool   populateAdapterList(HWND hDbWnd);
    bool   populateDisplayModeList(HWND hDbWnd);
    bool   populateControllerObjectList(HWND hDbWnd);
    bool   configure(HWND hDbWnd);
	void   configureD(int& w, int& h, int& d, int& m, int& p, int& t) const;
	void   configureW(int& w, int& h, int& s, int& e) const;
    void   configureJ(void** lpguid, long& trg, bool& yrv, bool& zon,
	 bool& f, void** h) const;
    void   cleanUp(HWND hDbWnd);
	void   Delete() { delete this; }
};

BOOL CALLBACK dlgProc(HWND hDbWnd, UINT msg, WPARAM wp, LPARAM lp);
BOOL CALLBACK enumInputDevices(LPCDIDEVICEINSTANCE didesc, void* combobox);
BOOL CALLBACK enumEffects(LPCDIEFFECTINFO di, HWND combobox);

//-------------------------------- Window --------------------------------
//
// Window manages the window component of the user interface
//
class Window : public IWindow {

    static IWindow* address_; // points to this object itself
    HINSTANCE hinst;          // points to the current application
    HWND      hwnd;           // points to the main application window

	int show;                 // how to display the main application window
	int width;                // width of client area
	int height;               // height of client area
	int wndStyle;             // window style
	int wndExStyle;           // window style ex

    Window(void* hinstance, int s);
	Window(const Window&);
	Window& operator=(const Window&);
    virtual ~Window();

  public:
    friend IWindow* CreateGWindow(void* hinst, int show);
    static IWindow* address() { return address_; }
    bool   setup();
	void*  window() const { return (void*)hwnd; }
	void   Delete() { delete this; }
};

bool registerWindowClass(HINSTANCE hinst);
LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

#endif