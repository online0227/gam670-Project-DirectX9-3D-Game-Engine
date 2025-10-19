#ifndef _I_AUDIO_H_
#define _I_AUDIO_H_

/* Interface to the Audio Module
 *
 * consists of IAudio interface
 *             ISound interface
 *
 * IAudio.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

//-------------------------------- IAudio --------------------------------
//
// IAudio is the interface to the audio component of the model
//
class IKeyboard;
class IMouse;
class IJoystick;
class IHUD;

class IAudio {
  public:
    virtual bool setup(int now)                       = 0;
    virtual void update(int now)                      = 0;
    virtual void toggle(int now, const wchar_t* file) = 0;
    virtual void play()                               = 0;
    virtual void suspend()                            = 0;
    virtual bool restore(int now)                     = 0;
	virtual void Delete()                             = 0;
};

extern "C"
IAudio* CreateAudio(IKeyboard* k, IMouse* m, IJoystick* j, IHUD* h);

//-------------------------------- ISound --------------------------------
//
// ISound is the interface to a sound source within the audio system
//
typedef enum SoundType {
    LOCAL_SOUND,
    OBJECT_SOUND,
    GLOBAL_SOUND
} SoundType;

struct Vector;
class IObject;
class ISegment;

class ISound {
  public:
	virtual ISegment* segment() const                         = 0;
	virtual SoundType isType() const                          = 0;
	virtual const  wchar_t* filename() const                  = 0;
    virtual void   setIdleFrequency(int f)                    = 0;
    virtual void   attenuate(int factor)                      = 0;
    virtual void   maxVolume()                                = 0;
    virtual void   minVolume()                                = 0;
    virtual void   dropFrequency(int delta)                   = 0;
    virtual void   increaseFrequency(int delta)               = 0;
	virtual void   attach(IObject* object, bool reset = true) = 0;
	virtual void   detach()                                   = 0;
	virtual bool&  isOn()                                     = 0;
	virtual bool&  isSetToStart()                             = 0;
	virtual bool&  isSetToStop()                              = 0;
	virtual bool   isContinuous() const                       = 0;
	virtual bool   hasGlobalFocus() const                     = 0;
	virtual bool   hasVolumeControl() const                   = 0;
	virtual bool   hasFrequencyControl() const                = 0;
    virtual float  volume() const                             = 0;
    virtual int    frequency() const                          = 0;
	virtual float  coneAngle() const                          = 0;
	virtual Vector position() const                           = 0;
	virtual Vector direction() const                          = 0;
	virtual bool   toggle(int now)                            = 0;
	virtual void   restore(int now)                           = 0;
	virtual void   Delete()                                   = 0;
};

extern "C"
ISound* CreateSound(const wchar_t* f, SoundType t, bool c, bool o, 
 float q = 0, float x = 0, float y = 0, float z = 0);

#endif
