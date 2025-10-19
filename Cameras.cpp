/* Cameras Module Implementation
 *
 * Camera.cpp
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "IHUD.h"          // for HUD and IText
#include "IInput.h"        // for Keyboard, Mouse, Joystick
#include "IScene.h"        // for Object
#include "ModelSettings.h" // for CAMERA_?
#include "Utilities.h"     // for wsprintf()
#include "Cameras.h"       // for Camera class declaration

//-------------------------------- Cameras -------------------------------
//
// Cameras manages the set of viewpoint components in the model
//
ICameras* CreateCameras(IKeyboard* k, IMouse* m, IJoystick* j, IHUD* h) {

	return new Cameras(k, m, j, h);
}

// constructor initializes all cameras and sets pointers to input objects
//
Cameras::Cameras(IKeyboard* k, IMouse* m, IJoystick* j, IHUD* h) : 
 keyboard(k), mouse(m), joystick(j), hud(h) {

    for (int i = 0; i < MAX_CAMERAS; i++)
        camera[i] = NULL;

	indexManuallyChanged = false;
    noCameras        = 0;
	index            = 0;
    text             = NULL;
	Camera::cameras  = this;
	Camera::keyboard = keyboard;
	Camera::mouse    = mouse;
	Camera::joystick = joystick;
	
}

// add adds a pointer to Camera* c to the set of cameras
// and returns true if successful, false otherwise
//
bool Cameras::add(ICamera* c) {

    int i;
    bool rc = false;

    for (i = 0; i < noCameras; i++)
        if (!camera[i]) {
            camera[i] = c;
            i = noCameras + 1;
            rc = true;
        }
    if (i == noCameras && noCameras < MAX_CAMERAS) {
        camera[noCameras++] = c;
        rc = true;
    }

    return rc;
}

// setup creates the global camera(s) in the model
//
bool Cameras::setup(int now) {

    // create global cameras here
    CreateCamera(
     Vector(0, CAMERA_Y, CAMERA_Z),
     Vector(0, 0, CAMERA_H_Z),
     Vector(0, CAMERA_U_Y, 0), NULL, THIRD_PERSON);

	current = camera[index];

    text = CreateText(0, 0.1f, 1, 0.2f, TEXT_CENTER, ".....");

    for (int i = 0; i < noCameras; i++)
        if (camera[i])
            camera[i]->setup(now);

    return true;
}

void Cameras::setCamera(ICamera* ca, bool isFirst) {
		
        current = ca;
		indexManuallyChanged = true;
		
}

// update selects the current camera and updates it
//
void Cameras::update(int now) {

    // switch to the next camera in line
	if (keyboard->pressed(F8) && now - lastUpdate > LATENCY) {

		if(indexManuallyChanged == true) {
		indexManuallyChanged = false;
		}
		else if (index < noCameras - 1) 
			index++;
		else
			index = 0;
		
        current = camera[index];
		lastUpdate = now;
		
	}

    // update the current camera only
    current->update(now);

	if (hud->isOn()) {
		char desc[MAX_CHAR + 1];
		wsprintf(desc, "Camera: at (%d,%d,%d), heading (%d,%d,%d)", 
		(int)(current->position()).x, (int)(current->position()).y, 
		(int)(current->position()).z, (int)(100*(current->heading()).x), 
		(int)(100*(current->heading()).y), 
		(int)(100*(current->heading()).z));
		text->set(desc);
	}
}

// position returns the viewpoint of the current camera
//
Vector Cameras::position() const {
	
	return current->position();
}

// heading returns the look at direction of the current camera
//
Vector Cameras::heading() const {
	
	return current->heading();
}

// top returns the up direction of the current camera
//
Vector Cameras::top() const {
	
	return current->top();
}

// restore restores all of the cameras in the model
//
bool Cameras::restore(int now) {

    bool rc = false;

    for (int i = 0; i < noCameras; i++)
        if (camera[i])
            camera[i]->restore(now);

    return rc;
}

// remove removes the pointer to Camera *c from the set of cameras and
// returns true if successful, false otherwise
//
bool Cameras::remove(ICamera* c) {

    bool rc = false;

    for (int i = 0; i < noCameras; i++)
        if (camera[i] == c) {
            camera[i] = NULL;
            rc = true;
        }
    while (!camera[noCameras - 1])
        noCameras--;

    return rc;
}

// destructor
//
Cameras::~Cameras() {

    for (int i = 0; i < noCameras; i++)
        if (camera[i]) 
            camera[i]->Delete();

	Camera::cameras = NULL;
}

//-------------------------------- Camera ----------------------------------
//
// Camera holds the information for a single viewpoint of the model
//
ICameras*  Camera::cameras  = NULL;
IKeyboard* Camera::keyboard = NULL;
IMouse*    Camera::mouse    = NULL;
IJoystick* Camera::joystick = NULL;

ICamera* CreateCamera(Vector& v, Vector& d, Vector& u, IObject* obj, CameraType ca) {

	return new Camera(v, d, u, obj, ca);
}

// constructor initializes the time of the last update and adds the 
// camera to the set of cameras
//
Camera::Camera(Vector& v, Vector& d, Vector& u, IObject* obj, CameraType ca) : caType(ca), viewpt(v), direction(d),
 up(u), right(cross(u, d)), object(NULL) {



	if (cameras)
		cameras->add(this);
	else
		error("Camera 01:: Couldn\'t access camera set");

    lastUpdate = 0;
}

// setup initializes the time of the last update
//
bool Camera::setup(int now) {

    lastUpdate = now;
	forward = Vector(0.0f, 0.0f, 1.0f);
	eye = Vector(0.0f, 0.0f, -300.0f);

    return true;
}

// update adjusts the viewpoint and direction of the camera
// for time increment "delta" according to the keys pressed
// by the user
//
void Camera::update(int now) {

    int   joy_x = 0,     // joystick x-axis: turn left/right
          joy_y = 0,     // joystick y-axis: advance/retreat
          joy_z = 0,     // joystick z-axis: turn up/down
          joy_rz = 0;    // joystick rz-axis:
    int   mse_x = 0,     // mouse displacement forward-back-axis
          mse_y = 0,     // mouse displacement left-right-axis
          mse_z = 0;     // mouse displacement roll button
    float rxl = 0.0f, // rotate about xl nose up/down
          ryl = 0.0f, // rotate about yl nose right/left
          rzl = 0.0f, // rotate about zl roll cl/ccl
          dz  = 0.0f, // move forward/reverse
          dy  = 0.0f, // move up/down
          dx  = 0.0f; // pan right/left

	if (!lastUpdate) setup(now);

    int delta  = now - lastUpdate;
    lastUpdate = now;

    // joystick input rotates the camera
    joystick->handle(joy_x, joy_y, joy_z, joy_rz);

    // mouse input translates the viewpoint
	mouse->displacementForMyGame(mse_x, mse_y);

    // Translate the viewpoint

	if(caType == THIRD_PERSON) {

    if (keyboard->pressed('A')) //수정
        dx -= CAM_SPEED * delta;
    if (keyboard->pressed('D')) //수정
        dx += CAM_SPEED * delta;

    if (keyboard->pressed('W')) //수정
        dz += CAM_SPEED * delta;
   if (keyboard->pressed('S')) //수정
        dz -= CAM_SPEED * delta;

	}

	
    // Rotate the viewpoint about local axes of camera
    //
    // about xl axis - how much to pitch nose up/down
    // (Use the joystick data, if available)
    //if (joy_y)
    //    rxl = + JOY_ROTATION_FACTOR * ANG_CAM_SPEED * joy_y;

    //// (Use the joystick data, if available)
    //if (joy_rz)
    //    ryl = - JOY_ROTATION_FACTOR * ANG_CAM_SPEED * joy_rz;

    //if (joy_x)
    //    rzl = - JOY_ROTATION_FACTOR * ANG_CAM_SPEED * joy_x;


    // apply the translations and rotations
	//
    if (rxl || ryl || rzl) {
        // Rotate the Camera
        Matrix rot; // rotation matrix
        // LHS Convention
        rxl = - rxl;
        rzl = - rzl;
		rot        = ::rotate(direction, rzl);
        up        *= rot;
        right     *= rot;
		rot        = ::rotate(right, rxl);
        direction *= rot;
        up        *= rot;
		rot        = ::rotate(up, ryl);
        direction *= rot;
        right     *= rot;
		
    }

	//recalculate right, so that dx * right works properly
	Vector carUp = normal(up);
	Vector carHead = normal(direction);
	Vector newRight = cross(carUp, carHead);
	right = normal(newRight);

    if (dx || dy || dz) {
        // Translate the Camera
        viewpt = viewpt + dz * direction + dy * up  + dx * right;
    }

	// force feedback on camera rotations
	//
	if (joystick->pressed(BUTTON_3))
		joystick->update(100);
	if (joystick->pressed(BUTTON_4))
		joystick->update(0);
		//*******************************************************************
	    // adjust camera
	/* mouse look rotation */
	if (mse_x || mse_y || mse_z) {
        Matrix rot;
        // yaw left/right
		if (mse_y) { 
            direction *= rotatey(rot, -mse_y * ANG_CAM_SPEED);
        }
		// pitch up/down
		if (mse_x) {
			if(caType == THIRD_PERSON)
				direction.y -= mse_x * ANG_CAM_SPEED;
			else
				direction.y -= -mse_x * ANG_CAM_SPEED;
		}		



	}

	//make object always follows the camera
	if(object) {
		forward = object->position();
		forward.z += 1;
		


	}


	//*******************************************************************
}

// position returns the position of the camera in world space
//
Vector Camera::position() const {

	return object ? viewpt * object->world() : viewpt;
}

// heading returns the direction of the camera in world space
//
//Vector Camera::heading() const {
//	
//    return object ? direction * object->rotation() : direction;
//}

Vector Camera::heading() const {
	
    return object ? -1 * direction: direction;
}

Vector Camera::rightSide() const {
	
    return object ? right * object->rotation() : right;
}

// top returns the up direction of the camera in world space
//
Vector Camera::top() const {
	
    return object ? up * object->rotation() : up;
}

// attach attaches the camera to Object* newObject
//
void Camera::attach(IObject* newObject, bool reset) {

	if (object) {
		viewpt    = position();
		direction = heading();
		up        = top();
		right     = cross(up, direction);
	}

	object = newObject;
	
	if (reset) {
		viewpt      = viewpt - object->position();
		Matrix rinv = object->rotation().transpose();
		viewpt     *= rinv;
		direction  *= rinv;
		up         *= rinv;
		right       = cross(up, direction);
	}
}

// detach detaches the camera from Object* object
//
void Camera::detach() {

	if (object) {
		viewpt    = position();
		direction = heading();
		up        = top();
		right     = cross(up, direction);
	}
	object = 0;
}

// restore reinitializes the time of the last update to the current time
//
bool Camera::restore(int now) {

    lastUpdate = now;

    return true;
}

// destructor removes the pointer to this camera from the set of cameras
//
Camera::~Camera() {

	if (cameras)
		cameras->remove(this);
	else
		error("Camera 90:: Couldn\'t access the set of cameras");
}
