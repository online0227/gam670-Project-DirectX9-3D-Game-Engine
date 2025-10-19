/* Lighting Module Implementation
 *
 * Lighting.cpp
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "IInput.h"        // for Keyboard, Mouse, Joystick interfaces
#include "IHUD.h"          // for Text interface
#include "IGraphicsCard.h" // for DeviceLight interface
#include "ModelSettings.h" // for LIGHT0, LIGHT1, LIGHT2
#include "Utilities.h"     // for error()
#include "Lighting.h"      // for Lighting and Light class declarations

//-------------------------------- Lighting -----------------------------
//
// Lighting manages the lighting component of the model
//
ILighting* CreateLighting(IKeyboard* k, IMouse* m, IJoystick* j) {

	return new Lighting(k, m, j);
}

// constructor initializes the light source pointers
//
Lighting::Lighting(IKeyboard* k, IMouse* m, IJoystick* j) : keyboard(k), 
 mouse(m), joystick(j) {

    noLights = 0;
	for (int i = 0; i < MAX_LIGHTS; i++)
        source[i] = NULL;

	pointLight       = NULL;
	spotLight        = NULL;
	directionalLight = NULL;
	textPoint        = NULL;
	textSpot         = NULL;
	textDirectional  = NULL;
	Light::lighting  = this;
}

// add adds a pointer to ILight *l to the lighting system
// and returns true if successful, false otherwise
//
bool Lighting::add(ILight* l) {

    int i;
    bool rc = false;

    for (i = 0; i < noLights; i++)
        if (!source[i]) {
            source[i] = l;
            i = noLights + 1;
            rc = true;
        }
    if (i == noLights && noLights < MAX_LIGHTS) {
        source[noLights++] = l;
        rc = true;
    }

    return rc;
}

// setup sets up each light at the start of the game
//
bool Lighting::setup(int now) {

    // ambient light: a little bit of light on everything, for when all 
	// "lights are out"
    ambientLight = Colour(AMBIENT_LIGHT, AMBIENT_LIGHT, AMBIENT_LIGHT);

    // Make a far away point-source light
    //
	pointLight = CreateLight(POINT_LIGHT, LIGHT0, 
	 Colour(0.7f, 0.7f, 0.7f, 1),
	 Colour(0.7f, 0.7f, 0.7f, 1), 
	 Colour(1, 1, 1, 1), 
	 Vector(500.f, 1000.f, 100.f), 
	 Vector(0,0,0), 
	 20000.0f, 
	 1);
	CreateText(0.1f, 0.61f, 1, 0.68f, TEXT_FLAGS_DEFAULT, "- point light");
	textPoint = CreateText(0, 0.61f, 0.2f, 0.68f, TEXT_FLAGS_DEFAULT, 
	 LIGHT0 ? "on" : "off");

    // Make a closer yellowish spotlight
    //
	spotLight = CreateLight(SPOT_LIGHT, LIGHT1,
	 Colour(0.7f, 0.7f, 0.1f, 1),
	 Colour(0.2f, 0.2f, 0.2f, 1),
	 Colour(1, 1, 1, 1),
	 Vector(-30.f, 100.f, 0.f),
	 Vector(3.f, -10.f, 0.f),
	 300.0f,
	 0.1f,
	 0.01f,
	 0,
	 .5f,
	 .3f,
	 0.5);
	CreateText(0.1f, 0.68f, 1, 0.75f, TEXT_FLAGS_DEFAULT, "- spot light");
	textSpot = CreateText(0, 0.68f, 1, 0.75f, TEXT_FLAGS_DEFAULT, 
	 LIGHT1 ? "on" : "off");

    // Make a distance directional light
    //
	directionalLight = CreateLight(DIRECTIONAL_LIGHT, LIGHT2,
	 Colour(0.0f, 0.8f, 0.6f, 1),
	 Colour(0.0f, 0.0f, 0.0f, 1),
	 Colour(1, 1, 1, 1),
	 Vector(10, 1000, 0),
	 normal(Vector(-10, -100, 0)),
	 0,
	 1);
 	CreateText(0.1f, 0.75f, 1, 0.82f, TEXT_FLAGS_DEFAULT, 
	 "- directional light");
	textDirectional = CreateText(0, 0.75f, 1, 0.82f, TEXT_FLAGS_DEFAULT, 
	 LIGHT2 ? "on" : "off");

    return noLights <= MAX_LIGHTS;
}

// setupDeviceLights sets up the deviceLight on the graphics card 
// for each light source in the lighting system and assigns index i
// to the ith deviceLight
//
void Lighting::setupDeviceLights(int maxLights) {

	for (int i = 0; i < noLights && i < maxLights; i++)
		source[i]->deviceLight()->setup(i);
}

// update responds to the lighting-related user key presses  
//
void Lighting::update(int now) {

	if (keyboard->pressed('J') && pointLight && pointLight->toggle(now))
		textPoint->set(pointLight->isOn() ?  "on" : "off");
	if (keyboard->pressed('K') && spotLight && spotLight->toggle(now))
		textSpot->set(spotLight->isOn() ?  "on" : "off");
    if (keyboard->pressed('L') && directionalLight && 
	 directionalLight->toggle(now))
		textDirectional->set(directionalLight->isOn() ?  "on" : "off");
}

// ambient returns the colour of the ambient light
//
const Colour* Lighting::ambient() const {

    return &ambientLight;
}

// restore reinitializes the time of the last toggle for each light
//
void Lighting::restore(int now) {

	for (int i = 0; i < noLights; i++)
		source[i]->restore(now);
}

// remove removes the pointer to ILight *l from the lighting system
// and returns true if successful, false otherwise
//
bool Lighting::remove(ILight* l) {

    bool rc = false;

    for (int i = 0; i < noLights; i++)
        if (source[i] == l) {
            source[i] = NULL;
            rc = true;
        }
    while (!source[noLights - 1])
        noLights--;

    return rc;
}

// destructor deletes the light sources
//
Lighting::~Lighting() {

    for (int i = 0; i < noLights; i++)
        if (source[i])
            source[i]->Delete();

	Light::lighting = NULL;
}

//-------------------------------- Light --------------------------------
//
// Light holds the data for a single light in the game
//
ILight* CreateLight(LightType t, bool o, Colour d, Colour a, Colour s, 
 Vector p, Vector h, float r, float a0, float a1, float a2, float ph, 
 float th, float f) {

	return new Light(t, o, d, a, s, p, h, r, a0, a1, a2, ph, th, f);
}

Lighting* Light::lighting = NULL;

Light::Light(LightType t, bool o, Colour d, Colour a, Colour s, Vector p, 
 Vector h, float r, float a0, float a1, float a2, float ph, float th,
 float f) : type(t), on(o), diffuse_(d), ambient_(a), specular_(s), 
 position_(p), direction_(h), range_(r), attenuation0_(a0), 
 attenuation1_(a1), attenuation2_(a2), phi_(ph), theta_(th), falloff_(f) {

	 if (lighting)
		 lighting->add(this);
	 else
		 error("Light 00:: Can\'t access the lighting system");

	 deviceLight_ = CreateDeviceLight(this);
	 lastToggle     = 0;
}

// toggle toggles the status of the light and update the graphics card
// representation if outside the latency period
//
bool Light::toggle(int now) {

	bool rc = false;

    if (now - lastToggle > LATENCY) {
        on         = !on;
        lastToggle = now;
		deviceLight_->update();
		rc = true;
    }

	return rc;
}

// restore reinitializes the time of the last toggle and update
// the device light to the current setting
//
void Light::restore(int now) {

	lastToggle = now;
	deviceLight_->update();
}

