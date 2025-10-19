#ifndef _I_LIGHTING_H_
#define _I_LIGHTING_H_

/* Interface to the Lighting Module
 *
 * consists of ILighting interface
 *             ILight interface
 *
 * ILighting.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

//-------------------------------- ILighting --------------------------------
//
// ILighting is the interface to the lighting component of the model
//
struct Colour;
class IKeyboard;
class IMouse;
class IJoystick;

class ILighting {
  public:
    virtual bool  setup(int now)                     = 0;
	virtual void  setupDeviceLights(int maxLights) = 0;
    virtual void  update(int now)                    = 0;
    virtual const Colour* ambient() const            = 0;
	virtual void  restore(int now)                   = 0;
	virtual void  Delete()                           = 0;
};

extern "C"
ILighting* CreateLighting(IKeyboard* k, IMouse* m, IJoystick* j);

//-------------------------------- ILight -----------------------------------
//
// ILight is the interface to a light source within the lighting system
//
typedef enum LightType {
    POINT_LIGHT,
    SPOT_LIGHT,
    DIRECTIONAL_LIGHT
} LightType;

struct Vector;
class IDeviceLight;

class ILight {
  public:
	virtual IDeviceLight* deviceLight() const = 0;
	virtual LightType isType() const              = 0;
	virtual Colour diffuse() const                = 0;
	virtual Colour ambient() const                = 0;
	virtual Colour specular() const               = 0;
	virtual Vector position() const               = 0;
	virtual Vector direction() const              = 0;
	virtual float range() const                   = 0;
	virtual float attenuation0() const            = 0;
	virtual float attenuation1() const            = 0;
	virtual float attenuation2() const            = 0;
	virtual float phi() const                     = 0;
	virtual float theta() const                   = 0;
	virtual float falloff() const                 = 0;
	virtual bool  toggle(int now)                 = 0;
	virtual bool  isOn() const                    = 0;
	virtual void  restore(int now)                = 0;
	virtual void  Delete()                        = 0;
};

extern "C"
ILight* CreateLight(LightType t, bool o, Colour d, Colour a, Colour s, 
 Vector p, Vector h, float r, float a0, float a1 = 0, float a2 = 0, 
 float ph = 0, float th = 0, float f = 0);

#endif
