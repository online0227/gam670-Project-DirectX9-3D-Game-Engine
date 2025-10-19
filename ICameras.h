#ifndef _I_CAMERAS_H_
#define _I_CAMERAS_H_

/* Interface to the Cameras Module
 *
 * ICameras.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

//-------------------------------- ICameras ---------------------------------
//
// ICameras is the interface to the viewpoints component of the model
//
typedef enum CameraType {
	FIRST_PERSON = 1,
	SECOND_PERSON = 2,
	THIRD_PERSON = 3
} CameraType;

struct Vector;
class IKeyboard;
class IMouse;
class IJoystick;
class IHUD;
class ICamera;

class ICameras {
  public:
    virtual bool   add(ICamera* o)    = 0;
	virtual bool   remove(ICamera* o) = 0;
    virtual bool   setup(int now)     = 0;
    virtual void   update(int now)    = 0;
    virtual Vector position() const   = 0;
    virtual Vector heading() const    = 0;
    virtual Vector top() const        = 0;
    virtual bool   restore(int now)   = 0;
	virtual void   Delete()           = 0;
	virtual void setCamera(ICamera* ca, bool isFirst)		  = 0;
	virtual CameraType returnType()		  = 0;
	virtual ICamera* returnCurCar()	= 0;
};

extern "C"
ICameras* CreateCameras(IKeyboard* k, IMouse* m, IJoystick* j, IHUD* h);

//-------------------------------- ICamera -------------------------------
//
// ICamera is the interface to a viewpoint component of the model
//



class ICamera {
  public:
    virtual bool   setup(int now)                       = 0;
    virtual void   update(int now)                      = 0;
    virtual Vector position() const                     = 0;
    virtual Vector heading() const                      = 0;
    virtual Vector top() const                          = 0;
	virtual void attach(IObject* newObject, bool reset = true) = 0;
	virtual void detach()                               = 0;
   virtual  bool   restore(int now)                     = 0;
	virtual void Delete()                               = 0;
	virtual CameraType cameraType()						    = 0;
};

extern "C"
ICamera* CreateCamera(Vector& v, Vector& d, Vector& u, IObject* obj, CameraType ca);

#endif
