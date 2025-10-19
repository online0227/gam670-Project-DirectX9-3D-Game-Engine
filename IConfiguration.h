#ifndef _I_CONFIGURATION_H_
#define _I_CONFIGURATION_H_

/* Interface to the Configuration Module
 *
 * consists of ISelector interface
 *             IWindow interface
 *
 * IConfiguration.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

//-------------------------------- ISelector -----------------------------
//
// ISelector is the interface to the user selection component of the 
// user interface
//
class ISelector {
  public:
    virtual bool select()                                           = 0;
	virtual void configureD(int& w, int& h, int& d, int& m, int& f, 
	 int& t) const                                                  = 0; 
	virtual void configureW(int& w, int& h, int& s, int& e) const   = 0; 
    virtual void configureJ(void** lpguid, long& trg, bool& yrv, 
	 bool& zon, bool& f, void** h) const                            = 0;
    virtual void Delete()                                           = 0;
};

extern "C"
ISelector* CreateSelector(void* hinst);
extern "C"
ISelector* SelectorAddress();

//-------------------------------- IWindow -------------------------------
//
// IWindow is the interface to the window component of the user interface
//
class IWindow {
  public:
    virtual bool  setup()        = 0;
	virtual void* window() const = 0;
    virtual void  Delete()       = 0;
};

extern "C"
IWindow* CreateGWindow(void* hinst, int show);
extern "C"
IWindow* WindowAddress();

#endif