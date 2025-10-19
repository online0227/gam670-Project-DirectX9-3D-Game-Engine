#ifndef _LIGHTING_H_
#define _LIGHTING_H_

/* Header for the Lighting Module
 *
 * consists of Lighting declaration
 *             Light declaration
 *
 * Lighting.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "ILighting.h"
#include "math.h"      // for Vector, Colour

//-------------------------------- Lighting --------------------------------
//
// Lighting manages the lighting component of the model
//
class IText;

class Lighting : public ILighting {

	static const int MAX_LIGHTS = 8;

    IKeyboard* keyboard;        // points to the keyboard interface
    IMouse*    mouse;           // points to the mouse interface
    IJoystick* joystick;        // points to the joystick interface
    ILight* source[MAX_LIGHTS]; // points to the light sources

    int noLights;               // number of lights
    Colour ambientLight;        // ambient light

	ILight* pointLight;         // points to the point light
	ILight* spotLight;          // points to the spot light
	ILight* directionalLight;   // points to the directional light
	IText*  textPoint;          // points to the point light text
	IText*  textSpot;           // points to the spot light text
	IText*  textDirectional;    // points to the directional light text

    Lighting(IKeyboard* k, IMouse* m, IJoystick* j);
	Lighting(const Lighting& l);            // prevents copying
    Lighting& operator=(const Lighting& l); // prevents assignment
    bool    add(ILight*);
    bool    remove(ILight*);
    virtual ~Lighting();

  public:
	friend ILighting* CreateLighting(IKeyboard* k, IMouse* m, IJoystick* j);
    bool   setup(int now);
	void   setupDeviceLights(int maxLights);
    void   update(int now);
    const  Colour* ambient() const;
	void   restore(int now);
	void   Delete() { delete this; }
	friend class Light;
};

//-------------------------------- Light -----------------------------------
//
// Light holds the data for a single light source in the lighting system
//
class Light : public ILight {

    static Lighting* lighting;
	IDeviceLight*  deviceLight_;
    LightType type;
	bool   on;
    int    lastToggle;
    Colour diffuse_;
    Colour ambient_;
    Colour specular_;
    Vector position_;
    Vector direction_;
    float  range_;
    float  attenuation0_;
    float  attenuation1_;
    float  attenuation2_;
    float  phi_;
    float  theta_;
    float  falloff_;

	Light(LightType t, bool, Colour d, Colour a, Colour s, Vector p, Vector h, 
	 float r, float a0, float a1, float a2, float ph, float th, float f);

public:
	friend ILight* CreateLight(LightType t, bool o, Colour d, Colour a, 
	 Colour s, Vector p, Vector h, float r, float a0, float a1, float a2, 
	 float ph, float th, float f);
	IDeviceLight* deviceLight() const { return deviceLight_; }
	LightType isType() const { return type; }
	bool   isOn() const { return on; }
	Colour diffuse() const { return diffuse_; }
	Colour ambient() const { return ambient_; }
	Colour specular() const { return specular_; }
	Vector position() const { return position_; }
	Vector direction() const { return direction_; }
	float  range() const { return range_; }
	float  attenuation0() const { return attenuation0_; }
	float  attenuation1() const { return attenuation1_; }
	float  attenuation2() const { return attenuation2_; }
	float  phi() const { return phi_; }
	float  theta() const { return theta_; }
	float  falloff() const { return falloff_; }
	bool   toggle(int now);
	void   restore(int now);
	void   Delete() { delete this; }
	friend class Lighting;
};

#endif
