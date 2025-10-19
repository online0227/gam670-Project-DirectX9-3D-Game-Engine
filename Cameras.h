#ifndef _CAMERAS_H_
#define _CAMERAS_H_

/* Header for the Cameras Module
 *
 * Cameras.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "ICameras.h"
#include "math.h"    // for Vector

//-------------------------------- Cameras -------------------------------
//
// Cameras manages the set of viewpoint components in the model
//
class IText;

class Cameras : public ICameras {

	static const int MAX_CAMERAS = 10; 
	static const int MAX_CHAR    = 80;
	//static const ICameras camerasPt;

    ICamera*    camera[MAX_CAMERAS]; // points to the cameras in the model
    IHUD*      hud;                 // points to the heads up display
    IKeyboard* keyboard;            // points to the keyboard
    IMouse*    mouse;               // points to the mouse
    IJoystick* joystick;            // points to the joystick
	IText*     text;                // points to current camera text string
    ICamera*    current;             // points to the current camera

    int        noCameras;           // number of cameras in the model
	int        index;               // index of the current camera
	bool	   indexManuallyChanged;
    int        lastUpdate;          // time of last update


    Cameras(IKeyboard* k, IMouse* m, IJoystick* j, IHUD* h);
	Cameras(const Cameras&);
	Cameras& operator=(const Cameras&);
    virtual ~Cameras();
    bool add(ICamera* c);
    bool remove(ICamera* c);
	CameraType returnType()	{ return current->cameraType(); }


public:
	friend ICameras* CreateCameras(IKeyboard* k, IMouse* m, IJoystick* j, 
	 IHUD* h);
    bool   setup(int now);
    void   update(int now);
    Vector position() const;
    Vector heading() const;
    Vector top() const;
    bool   restore(int now);
	void   Delete() { delete this; }
	void setCamera(ICamera* ca, bool isFirst);
	ICamera* returnCurCar() { return current; }
	friend class Camera;
};

//-------------------------------- Camera ----------------------------------
//
// Camera holds information for a single viewpoint of the model
//
class IObject;

class Camera : public ICamera {

	static ICameras*  cameras;  // points to the set of cameras
	static IKeyboard* keyboard; // points to the keyboard
    static IMouse*    mouse;    // points to the mouse
    static IJoystick* joystick; // points to the joystick

    int      lastUpdate;   // time of last update
    Vector   viewpt;       // from where the camera is looking
    Vector   direction;    // direction the camera is pointing
    Vector   up;           // direction to the top of the camera
	Vector   right;        // direction to the right of camera
	IObject* object;       // points to the object attached to camera
	Vector forward;
	Vector eye;
	CameraType caType;

    Camera(Vector& v, Vector& d, Vector& u, IObject* obj, CameraType ca);
	Camera(const Camera& c);            // prevents copying
	Camera& operator=(const Camera& c); // prevents assignment
	virtual ~Camera();

  public:
	friend ICamera* CreateCamera(Vector& v, Vector& d, Vector& u, IObject* obj, CameraType ca);
    bool   setup(int now);
    void   update(int now);
    Vector position() const;
    Vector heading() const;
    Vector top() const;
	Vector rightSide() const;
	
	void   attach(IObject* newObject, bool reset);
	void   detach();
    bool   restore(int now);
	void   Delete() { delete this; }
	CameraType	cameraType() { return caType; }
	friend class Cameras;
};

#endif
