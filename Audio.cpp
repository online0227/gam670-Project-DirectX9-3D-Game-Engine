/* Audio Module Implementation
 *
 * Audio.cpp
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "IInput.h"        // for Keyboard, Mouse, Joystick interfaces
#include "IHUD.h"          // for CreateText
#include "ISoundCard.h"    // for Segment interface
#include "ModelSettings.h" // for SOUND?, F2, F3, F4, F5, UP, DOWN
#include "Utilities.h"     // for error()
#include "Audio.h"         // for Audio and Sound class declarations

//------------------------------- Audio ---------------------------------
//
// Audio manages the audio component of the model
//
IAudio* CreateAudio(IKeyboard* k, IMouse* m, IJoystick* j, IHUD* h) {

	return new Audio(k, m, j, h);
}

// constructor initializes the pointers to the sound sources
//
Audio::Audio(IKeyboard* k, IMouse* m, IJoystick* j, IHUD* h) : keyboard(k), 
 mouse(m), joystick(j), hud(h) {

    noSounds = 0;
    for (int i = 0; i < MAX_SOUNDS; i++)
        sound[i] = NULL;

	background   = NULL;
	//explosion    = NULL;
	lastUpdate   = 0;
    Sound::audio = this;
	numFire = 0;
}

// add adds a pointer to ISound *s to the audio system
// and returns true if successful, false otherwise
//
bool Audio::add(ISound* s) {

    int i;
    bool rc = false;

    for (i = 0; i < noSounds; i++)
        if (!sound[i]) {
            sound[i] = s;
            i = noSounds + 1;
            rc = true;
        }
    if (i == noSounds && noSounds < MAX_SOUNDS) {
        sound[noSounds++] = s;
        rc = true;
    }

    return rc;
}

// setup initializes the reference time and creates the initial sounds
//
bool Audio::setup(int now) {

	lastUpdate = now;

    // define initial global sounds here
	//
    background  = CreateSound(L"terran3.wav", GLOBAL_SOUND, true, SOUND0);

	for(int i=0; i < 100; i++)
	 explosion[i]   = CreateSound(L"q3plasma.wav", GLOBAL_SOUND, false, SOUND1);

	CreateText(0, 0.9f, 1, 0.99f, TEXT_FLAGS_DEFAULT, "Global Volume");
 	volume      = CreateText(0.35f, 0.9f, 1, 0.99f, TEXT_FLAGS_DEFAULT, "");

    return noSounds <= MAX_SOUNDS;
}

// toggle toggles ISound *sound if sufficent time has elapsed
//
bool Audio::toggle(int now, ISound* sound) {

    return sound && sound->toggle(now);
}

// toggle toggles the sound stored in file
//
void Audio::toggle(int now, const wchar_t* file) {

    for (int i = 0; i < noSounds; i++)
        if (sound[i] && !wcscmp(sound[i]->filename(), file))
			sound[i]->toggle(now);
}

// update adjusts the sound sources according to user interventions
// and updates the sound segments on the sound card
//
void Audio::update(int now) {

    long dz = 0;
    int ix, iy, iz, rz;
    int delta = now - lastUpdate;

    lastUpdate = now;

    // joystick input
    ix = iy = iz = rz = 0;
    joystick->handle(ix, iy, iz, rz);
    if (iy)
        dz = (long)(iy * JOY_DISPLACEMENT_FACTOR);

    // keyboard input
    if (keyboard->pressed(F2))
        toggle(now, background);
	
	if (mouse->pressed(LEFT_BUTTON) || joystick->trigger()) {
		toggle(now, explosion[numFire++]);

		if(numFire == 19) // should be 'MAX_SOUND - 1'
			numFire = 0;
	}
			
    if (keyboard->pressed(F4))
        background->attenuate(-1);
    if (keyboard->pressed(F5))
        background->attenuate(1);
	if (keyboard->pressed(F6))
		for (int i = 0; i < noSounds; i++)
			if (sound[i] && sound[i]->isType() != GLOBAL_SOUND)
				sound[i]->attenuate(-1);
	if (keyboard->pressed(F7)) 
		for (int i = 0; i < noSounds; i++)
			if (sound[i] && sound[i]->isType() != GLOBAL_SOUND)
				sound[i]->attenuate(1);
    if (keyboard->pressed(F12))
		background->dropFrequency(delta);
	else
		background->increaseFrequency(delta);

    // update the sound segments
    ISegment* segment;
    for (int i = 0; i < noSounds; i++) {
        segment = sound[i]->segment();
        if (segment)segment->update();
    }

	if (hud->isOn()) {
		int i;
		char str[41];
		for (i = 0; i < background->volume() * 40 && background->isOn(); i++)
			str[i] = '|';
		str[i] = '\0';
		volume->set(str);
	}
}

// play starts/stops playing the sound segments on the sound card
//
void Audio::play() {

    // start/stop the sound segments that are ready to start/stop
    ISegment* segment;
    for (int i = 0; i < noSounds; i++) {
        segment = sound[i]->segment();
        if (segment)segment->play();
    }
}

// suspend suspends the sound segments on the sound card
//
void Audio::suspend() {

    // suspend each sound segment
    ISegment* segment;
    for (int i = 0; i < noSounds; i++) {
        segment = sound[i]->segment();
        if (segment)segment->suspend();
    }
}

// restore re-initializes the time of the last update
//
bool Audio::restore(int now) {

    lastUpdate = now;

    // restore each sound segment
    ISegment* segment;
    for (int i = 0; i < noSounds; i++) {
        sound[i]->restore(now);
        segment = sound[i]->segment();
        if (segment)segment->restore();
    }

    return true;
}

// remove removes the pointer to ISound *s from the audio system and
// returns true if successful, false otherwise
//
bool Audio::remove(ISound* s) {

    bool rc = false;

    for (int i = 0; i < noSounds; i++)
        if (sound[i] == s) {
            sound[i] = NULL;
            rc = true;
        }
    while (!sound[noSounds - 1])
        noSounds--;

    return rc;
}

// destructor deletes all of the sounds in the audio system
//
Audio::~Audio() {

    for (int i = 0; i < noSounds; i++)
        if (sound[i])
            sound[i]->Delete();
}

//------------------------------- Sound ------------------------------------
//
// Sound holds the data for a single sound source in the audio system
//
ISound* CreateSound(const wchar_t* f, SoundType t, bool c, bool o, 
 float q, float x, float y, float z) {

	return new Sound(f, t, c, o, q, x, y, z);
}

Audio* Sound::audio = NULL;

// constructor receives sound properties and adds a pointer to the sound
// within the audio system
//
Sound::Sound(const wchar_t* f, SoundType t, bool c, bool o, float q, 
 float x, float y, float z) : file(f), type(t), continuous(c), on(o), 
 cone(q), heading(Vector(x, y, z)) {

    if (audio)
        audio->add(this);
    else
        error("Sound::00 Can\'t access the Audio System");

    segment_   = CreateSegment(this);
    setToStart = continuous;
    setToStop  = false;
	lastToggle = 0;
	volume_    = START_VOLUME;
    idle       = 0;
    freq       = 0;
	object     = NULL;
}

// setIdleFrequency sets the idle frequency at FREQ_RANGE below received
// frequency and initializes the current frequency to this idle frequency
//
void Sound::setIdleFrequency(int frequency) {

     idle = frequency - FREQ_RANGE;
     freq = idle;
}

// attenuate attenuates the volume of the sound 
//
void Sound::attenuate(int factor) {

    volume_ += factor * DELTA_VOLUME;
    if (volume_ > 1.0f)
        volume_ = 1.0f;
    else if (volume_ < 0.0f)
        volume_ = 0.0f;
}

// drop reduces the sound frequency gradually
//
void Sound::dropFrequency(int delta) {

    if (freq > idle) {
        long newfreq = (long)(freq - FREQ_DROP_VELOCITY * delta);
        freq = newfreq < idle ? idle : newfreq;
    }
}

// increaseFrequency raises the frequency gradually
//
void Sound::increaseFrequency(int delta) {

    if (freq < idle + FREQ_RANGE) {
        long newfreq = (long)(freq + FREQ_RISE_VELOCITY * 
         (delta < 0 ? - delta : delta));
        freq = newfreq > idle + FREQ_RANGE ? idle + FREQ_RANGE : newfreq;
    }
}

// attach attaches the sound to Object* newObject
//
void Sound::attach(IObject* newObject, bool reset) {

	if (object) {
		position_ = position();
		heading   = direction();
	}
	object = newObject;
	if (reset) {
		position_ = position_ - object->position();
		heading  *= object->rotation().transpose();
	}
}

// detach detaches the sound from Object* object
//
void Sound::detach() {

	if (object) {
		position_ = position();
		heading   = direction();
	}
	object = 0;
}

// position returns the position of the sound in world space
//
Vector Sound::position() const {
	
	return object ? object->position() : position_;
}

// direction returns the direction of the sound in world space
//
Vector Sound::direction() const {
	
    return object ? heading * object->rotation() : heading;
}

// toggle toggles the sound provided that the latency period has elapsed
//
bool Sound::toggle(int now) {

	bool rc = false;

    if (now - lastToggle > LATENCY) {
        //if (on)
          //  setToStop = true;
       // else
            setToStart = true;
        on         = !on;
        lastToggle = now;
		rc = true;
    }

	return rc;
}

// restore reinitializes the time of the last toggle
//
void Sound::restore(int now) {

	lastToggle = now;
}

// destructor deletes the sound segment on the sound card and removes
// the pointer to the sound from the audio system
//
Sound::~Sound() {

    if (segment_)
		segment_->Delete();
    if (audio)
        audio->remove(this);
    else
        error("Sound::90 Can\'t access the Audio System");
}


