#ifndef _SOUND_CARD_H_
#define _SOUND_CARD_H_

/* Header for the SoundCard Module
 *
 * consists of SoundCard declaration
 *             Segment declaration
 *
 * SoundCard.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include <fstream>
using namespace std;
#define WIN32_LEAN_AND_MEAN
#include <mmsystem.h> // dsound.h needs this header file
#define  INITGUID     // enables access to GUIDs:
                      // _IID_IDirectSound3DListener, etc.
#include <dsound.h>
#undef   INITGUID

#include "ISoundCard.h"

//------------------------------- SoundCard --------------------------------
//
// SoundCard represents the sound card component of the device set
//
// SoundCard simulates the sound card that facilitates the input, output and
// combination of audio signals to, from and on the hardware
//
class IAudio;

class SoundCard : public ISoundCard {

	HWND hwnd;                         // points to app window
    IDirectSound8*           dsd;      // points to direct sound device
    IDirectSoundBuffer*      primary;  // points to the primary buffer
                                       // cannot be IDirectSoundBuffer8
    IDirectSound3DListener8* listener; // for local sounds

    IAudio* audio;                     // points to the sound set

    SoundCard(IAudio* a);
    SoundCard(const SoundCard& s);
    SoundCard& operator=(const SoundCard& s);
    virtual ~SoundCard();

  public:
	friend ISoundCard* CreateSoundCard(IAudio* audio);
    bool   setup();
    void   play(const Vector& p, const Vector& h, const Vector& u);
    void   release();
	void   Delete() { delete this; }
};

//-------------------------------- Segment ----------------------------
//
// Segment represents a single sound on the sound card component
//
class ISound;

class Segment : public ISegment {

	static const int MAX_CHAR = 80;

    static IDirectSound8* dsd; // points to the direct sound device
    static long volumeRange;   // volume range in sound card units
    static long silence;       // zero volume in sound card units

    IDirectSoundBuffer*    baseBuffer; // initial buffer retrieved
    IDirectSoundBuffer8*   buffer;     // buffer for global sounds
    IDirectSound3DBuffer8* buffer3D;   // buffer for 3D sounds
    DSBUFFERDESC dsbd;                 // holds the buffer description
    DS3DBUFFER ds3Dbd;                 // holds the 3D buffer description

    ISound*  sound;                    // points to the sound source

    ifstream file;                // file object holding the sound data
    unsigned fileSize;            // number of bytes in the sound file
    unsigned short bitsPerSample; // number of bits per discrete sound sample
    bool     streaming;           // streaming or static sound?
    unsigned midBuffer;           // position of the middle of the buffer
    unsigned lastPlayPos;         // last cursor play position

    Segment(ISound* s);
    Segment(const Segment& s);
    Segment& operator=(const Segment& s);
    virtual ~Segment();

	void release();
    bool getFileFormat(WAVEFORMATEX& wfm);
    bool loadSilenceToBuffer();
    bool loadFileToBuffer(unsigned start, unsigned size, unsigned flags);

  public:
	friend ISegment* CreateSegment(ISound* sound);
    bool   setup();
    void   play();
    void   update();
    void   suspend();
	void   restore();
	void   Delete() { delete this; }
    friend class SoundCard;
};
#endif
