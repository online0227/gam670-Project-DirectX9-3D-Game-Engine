#ifndef _AUDIO_H_
#define _AUDIO_H_

/* Header for the Audio Module
 *
 * consists of Audio declaration
 *             Sound declaration
 *
 * Audio.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "IAudio.h" // for Audio and Sound interfaces
#include "IScene.h" // for Object interface
#include "math.h"   // for Vector

//-------------------------------- Audio --------------------------------
//
// Audio manages the sound sources that make up the audio system
//
class IText;

class Audio : public IAudio {

	static const int MAX_SOUNDS = 19;

    ISound* sound[MAX_SOUNDS]; // points to the sound sources
    IKeyboard* keyboard;       // points to the keyboard
    IMouse*    mouse;          // points to the mouse
    IJoystick* joystick;       // points to the joystick
	IHUD*      hud;            // points to the heads up display
	IText*     volume;         // points to the volume measure

	int numFire;

	ISound* background;        // points to background sound
	ISound* explosion[100];         // points to explosion 
    int noSounds;              // number of sound sources
	int lastUpdate;            // time of previous update

    Audio(IKeyboard* k, IMouse* m, IJoystick* j, IHUD* h);
    Audio(const Audio& s);            // prevents copying
    Audio& operator=(const Audio& s); // prevents assignment
    bool    add(ISound*);
    bool    remove(ISound*);
    virtual ~Audio();

    bool toggle(int now, ISound* sound);

  public:
	friend IAudio* CreateAudio(IKeyboard* k, IMouse* m, IJoystick* j,
	 IHUD* h);
    bool   setup(int now);
    void   update(int now);
    void   toggle(int now, const wchar_t* file);
    void   play();
    void   suspend();
    bool   restore(int now);
	void   Delete() { delete this; }
	friend class Sound;
};

//------------------------------- Sound ------------------------------------
//
// Sound holds the data that describes a sound source within the audio 
// system
//
class Sound : public ISound {

    static Audio* audio; // points to the audio system
    ISegment* segment_;  // points to the sound segment for this sound
    IObject*  object;    // points to the parent object, if any
    const wchar_t* file; // points to the sound file

    SoundType type;          // type of sound - local, object or global
    bool   on;               // is this sound on?
    bool   setToStart;       // is this sound ready to start playing?
    bool   setToStop;        // is this sound ready to stop playing?
    bool   continuous;       // is this sound continuous?
    bool   globalFocus;      // this sound has global focus?
    bool   controlVolume;    // this sound has volume control?
    bool   controlFrequency; // this sound has frequency control?
    long   idle;             // idle frequency of this sound
    long   freq;             // current frequency of this sound
    float  volume_;          // current volume (0.0f -> 1.0f) 
    float  cone;             // angle of the sound cone in degrees
	Vector position_;        // position of sound in world space
	Vector heading;          // direction of cone axis
    int    lastToggle;       // time of the last toggle

    Sound(const wchar_t* f, SoundType t, bool c, bool o, float q, 
	 float x, float y, float z);
	Sound(const Sound&);
	Sound& operator=(const Sound&);
    virtual ~Sound();

  public:
	friend ISound* CreateSound(const wchar_t* f, SoundType t, bool c, bool o, 
	 float q, float x, float y, float z);
	ISegment* segment() const { return segment_; }
	SoundType isType() const { return type; }
	const  wchar_t* filename() const { return file; }
    void   setIdleFrequency(int f);
    void   attenuate(int factor);
    void   maxVolume() { volume_ = 1.0f; }
    void   minVolume() { volume_ = 0.0f; }
    void   dropFrequency(int delta);
    void   increaseFrequency(int delta);
	void   attach(IObject* object, bool reset);
	void   detach();
	bool&  isOn() { return on; }
	bool&  isSetToStart() { return setToStart; }
	bool&  isSetToStop() { return setToStop; }
	bool   isContinuous() const { return continuous; }
	bool   hasGlobalFocus() const { return globalFocus; }
	bool   hasVolumeControl() const { return controlVolume; }
	bool   hasFrequencyControl() const { return controlFrequency; }
    float  volume() const { return volume_; }
    int    frequency() const { return freq; }
	float  coneAngle() const { return cone; }
	Vector position() const;
	Vector direction() const;
	bool   toggle(int now);
	void   restore(int now);
	void   Delete() { delete this; }
    friend class Audio;
};

#endif
