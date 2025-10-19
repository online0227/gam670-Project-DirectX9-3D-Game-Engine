/* SoundCard Module Implementation
 *
 * SoundCard.cpp
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "IConfiguration.h" // for WindowAddress
#include "IAudio.h"         // for Audio and Sound interfaces
#include "math.h"           // for Vector
#include "DeviceSettings.h" // for SOUND_DISTANCE_FACTOR
#include "Utilities.h"      // for error()
#include "SoundCard.h"      // for SoundCard and Segment class declarations

//------------------------------- SoundCard ----------------------------------
//
// SoundCard represents the sound card component of the device set
//
// SoundCard simulates the sound card that facilitates the input, output and
// combination of audio signals to, from and on the hardware
//
ISoundCard* CreateSoundCard(IAudio* audio) {

	return new SoundCard(audio);
}

// constructor initializes the instance pointers
//
SoundCard::SoundCard(IAudio* a) : audio(a) {

    dsd      = NULL;
    primary  = NULL;
    listener = NULL;
}

// setup creates the direct sound object, sets the cooperative level,
// retrieves access to the primary buffer, obtains the interface to
// the listener, sets the format of the primary buffer, sets the
// speaker configuration and the distance factor
//
bool SoundCard::setup() {

    bool rc = false;
	hwnd = (HWND)WindowAddress()->window();

    // obtain an interface to the direct sound object
    if (FAILED(DirectSoundCreate8(&DSDEVID_DefaultPlayback,
     &dsd, NULL))) {
        error("SoundCard::10 Couldn\'t create the sound device");
    }
    // bind the direct sound object to the application window
    else if (FAILED(dsd->SetCooperativeLevel(hwnd, DSSCL_PRIORITY))) {
        error("SoundCard::11 Couldn\'t set cooperative level");
        release();
    }
    // set the format of the primary buffer
    else {
        DSBUFFERDESC dsbd; // primary buffer description
        // setup the buffer description structure
        ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
        dsbd.dwSize  = sizeof(DSBUFFERDESC);
        dsbd.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER;
        // setup desired format
        WAVEFORMATEX wfm;
        ZeroMemory(&wfm, sizeof(WAVEFORMATEX));
        wfm.wFormatTag      = WAVE_FORMAT_PCM;
        wfm.nChannels       = 2;
        wfm.nSamplesPerSec  = 44100;
        wfm.wBitsPerSample  = 16;
        wfm.nBlockAlign     = wfm.wBitsPerSample / 8 * wfm.nChannels;
        wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;
        // gain access to the primary buffer
        if (FAILED(dsd->CreateSoundBuffer(&dsbd, &primary, NULL)))
         {
            error("SoundCard::12 Couldn\'t access primary buffer ");
            release();
        }
        // obtain an interface to the listener
        else if (FAILED(primary->QueryInterface(IID_IDirectSound3DListener,
         (void **)&listener))) {
            error("SoundCard::13 Couldn\'t get listener interface");
            primary->Release();
            release();
        }
        else {
            // set the speaker configuration
            dsd->SetSpeakerConfig(DSSPEAKER_COMBINED( DSSPEAKER_STEREO,
             DSSPEAKER_GEOMETRY_WIDE));
            // try to reset the format of the primary buffer
            if (FAILED(primary->SetFormat(&wfm))) {
                error("SoundCard::14 Couldn\'t reset format of primary "
                 "buffer");
            }
            // set the number of meters in a vector unit - default is 1.0
            listener->SetDistanceFactor(SOUND_DISTANCE_FACTOR,
             DS3D_DEFERRED);

            // set what is common to all sounds
            Segment::dsd         = dsd;
            Segment::volumeRange = DSBVOLUME_MAX - DSBVOLUME_MIN;
            Segment::silence     = DSBVOLUME_MIN;
            rc = true;
        }
    }

    return rc;
}

// play adjusts the listener and starts/stops the pending sounds in the
// audio system
//
void SoundCard::play(const Vector& p, const Vector& d, const Vector& u) {

    // update listener's position and orientation
    listener->SetPosition(p.x, p.y, p.z, DS3D_DEFERRED);
    listener->SetOrientation(d.x, d.y, d.z, u.x, u.y, u.z,
     DS3D_DEFERRED);

    // commit all deferred settings
    listener->CommitDeferredSettings();

    // start/stop playing the pending sounds
    audio->play();
}

// release detaches the interfaces to the COM objects
//
void SoundCard::release() {

    if (listener) {
        listener->Release();
        listener = NULL;
    }
    if (primary) {
        primary->Release();
        primary = NULL;
    }
    if (dsd) {
        dsd->Release();
        dsd = NULL;
    }
}

// destructor releases the pointers to the sound card
//
SoundCard::~SoundCard() {

    release();
}

//-------------------------------- Segment ----------------------------
//
// Segment represents a sound segment on the SoundCard
//
ISegment* CreateSegment(ISound* sound) {

	return new Segment(sound);
}

IDirectSound8* Segment::dsd = NULL; // set by SoundCard
long Segment::volumeRange   = 0;    // set by SoundCard
long Segment::silence       = 0;    // set by SoundCard

// constructor initializes the instance pointers
//
Segment::Segment(ISound* s) : sound(s) {

    baseBuffer  = NULL;
    buffer      = NULL;
    buffer3D    = NULL;
    fileSize    = 0;
    lastPlayPos = 0;
}

// setup retrieves an interface to the base sound buffer for the sound
// segment, extracts the interface to the sound buffer, loads the
// sound file into the buffer if the sound is static, and retrieves
// the interface to the 3d sound buffer if the sound is non-global
//
bool Segment::setup() {

    char str[81];
    bool rc = false;

    // extract the format from the sound file
    WAVEFORMATEX wfm;
    ZeroMemory(&wfm, sizeof(WAVEFORMATEX));
    if (!getFileFormat(wfm)) {
        wsprintf(str, "Segment::11 Sound file %ls is unreadable",
         sound->filename());
        error(str);
    }
    else {
        // should sound be streamed or static?
        streaming = fileSize > MAX_SOUND_BUFFER_SIZE;
        // setup the buffer description structure
        ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
        dsbd.dwSize      = sizeof(DSBUFFERDESC);
        dsbd.lpwfxFormat = &wfm;
        dsbd.dwFlags     = 0;
        switch(sound->isType()) {
            case GLOBAL_SOUND:
                break;
            case OBJECT_SOUND:
            case LOCAL_SOUND:
                dsbd.dwFlags |= DSBCAPS_CTRL3D;
                break;
            default:
                error("Segment::12 Unknown sound type");
                release();
        }
        if (sound->hasFrequencyControl()) dsbd.dwFlags |= DSBCAPS_CTRLFREQUENCY;
        if (sound->hasVolumeControl())    dsbd.dwFlags |= DSBCAPS_CTRLVOLUME;
        if (sound->hasGlobalFocus())      dsbd.dwFlags |= DSBCAPS_GLOBALFOCUS;
        // adjustments for a streaming buffer
        if (streaming) {
            // ensures best tracking of the current play position
            dsbd.dwFlags |= DSBCAPS_GETCURRENTPOSITION2;
            // buffer to hold 2 seconds of data
            dsbd.dwBufferBytes = wfm.nAvgBytesPerSec * 2;
            // set position of the middle of the buffer
            midBuffer = wfm.nAvgBytesPerSec;
        }
        else {
            // static sound
            dsbd.dwFlags |= DSBCAPS_STATIC;
            // buffer to hold entire sound
            dsbd.dwBufferBytes = fileSize;
        }
        // create the base sound buffer
        if (FAILED(dsd->CreateSoundBuffer(&dsbd, &baseBuffer, NULL))) {
             wsprintf(str, "Segment::13 Couldn\'t create sound buffer "
              "for file %ls", sound->filename());
            error(str);
        }
        // extract the regular sound buffer
        if (FAILED(baseBuffer->QueryInterface(IID_IDirectSoundBuffer8,
         (void **)&buffer))) {
             wsprintf(str, "Segment::14 Couldn\'t extract sound buffer"
              " for file %ls", sound->filename());
            error(str);
            release();
        }
        // fill buffer and initialize 
        else {
            // fill the buffer - streaming or static
            // with silence if streaming
            if (streaming)
                loadSilenceToBuffer();
            // with the entire sound if static
            else if (!loadFileToBuffer(0, fileSize, DSBLOCK_ENTIREBUFFER)) {
                wsprintf(str, "Segment::15 Couldn\'t fill the "
                 "buffer with sound file %ls", sound->filename());
                error(str);
                release();
            }
            // initialize to start sound if continuous
            sound->isSetToStart() = sound->isContinuous() && sound->isOn();
            // initialize frequency control
            if (sound->hasFrequencyControl()) {
                // store the idle frequency in the sound object
                DWORD freq;
                buffer->GetFrequency(&freq);
                sound->setIdleFrequency(freq);
                freq = sound->frequency();
                buffer->SetFrequency(freq);
            }
            // initialize 3d sound parameters
            if (sound->isType() == OBJECT_SOUND || sound->isType() == LOCAL_SOUND) {
                // extract the 3d sound buffer
                if (FAILED(buffer->QueryInterface(IID_IDirectSound3DBuffer8,
                 (void **)&buffer3D))) {
                     wsprintf(str, "SoundSegment::16 Couldn\'t extract 3d "
                      "sound buffer for file %ls", sound->filename());
                    error(str);
                }
                else {
                    // distance from listener where sound attenuation begins
                    buffer3D->SetMinDistance(DS3D_DEFAULTMINDISTANCE *
                     ATTENUATION_DISTANCE_FACTOR, DS3D_DEFERRED);
                    // outside cone (DSBVOLUME_MIN < < DSBVOLUME_MAX)
                    buffer3D->SetConeOutsideVolume(DSBVOLUME_MIN,
                     DS3D_DEFERRED);
                    // sound cones - interior, outside - in degrees -
                    // default is 360 for both if sound cone is 0
                    if (sound->coneAngle())
                        buffer3D->SetConeAngles(DWORD(INTERIOR_CONE *
                         sound->coneAngle()), DWORD(sound->coneAngle()), 
						 DS3D_DEFERRED);
                    // save the 3d settings in instance variable ds3Dbd
                    ds3Dbd.dwSize = sizeof(DS3DBUFFER);
                    buffer3D->GetAllParameters(&ds3Dbd);
                    // set the sound's position and orientation
                    update();
                }
            }
            // settings will be committed through listener in SoundCard
            rc = true;
        }
    }

    return rc;
}

// getWaveFileFormat fills wfm with the format defined in the fmt chunk
// of the file sound->filename() if the file is a .wav file in PCM format
// and sets fileSize and bitsPerSample
//
bool Segment::getFileFormat(WAVEFORMATEX& wfm) {

    bool rc = false;
    unsigned short formatTag, nChannels, blockAlign;
    unsigned       chunkSize, nSamplesPerSec, byteRate;
    char riff[4], format[4];

    if (sound->filename() != NULL) {
        file.open(sound->filename(), std::ios::in | std::ios::binary);
        if (file) {
            file.read(riff, sizeof(riff));    // should hold "RIFF"
            file.read((char*)&chunkSize,      sizeof(chunkSize));
            file.read((char*)&format,         sizeof(format));
            file.seekg(20);
            file.read((char*)&formatTag,      sizeof(formatTag));
            file.read((char*)&nChannels,      sizeof(nChannels));
            file.read((char*)&nSamplesPerSec, sizeof(nSamplesPerSec));
            file.read((char*)&byteRate,       sizeof(byteRate));
            file.read((char*)&blockAlign,     sizeof(blockAlign));
            file.read((char*)&bitsPerSample,  sizeof(bitsPerSample));
            // populate wfm
            if (riff[0] == 'R' && riff[1] == 'I' && riff[2] == 'F' && 
             riff[3] == 'F' && format[0] == 'W' && format[1] == 'A' && 
             format[2] == 'V' && format[3] == 'E' &&
             formatTag == WAVE_FORMAT_PCM) {
                wfm.wFormatTag      = formatTag;
                wfm.nChannels       = nChannels;
                wfm.nSamplesPerSec  = nSamplesPerSec;
                wfm.nAvgBytesPerSec = nSamplesPerSec * blockAlign;
                wfm.nBlockAlign     = blockAlign;
                wfm.wBitsPerSample  = bitsPerSample;
                fileSize = chunkSize - 36;
                // move to start of sound data
                file.seekg(44);
                rc = true;
            }
            else
                error("Segment::20 Unacceptable file format");
        }
        else
            error("Segment::21 Couldn\'t open sound file");
    }

    return rc;
}

// loadSilenceToBuffer fills the sound buffer with silence
//
bool Segment::loadSilenceToBuffer() {

    bool rc = false;
    void* start;  // address of start of locked data
    DWORD locked; // number of bytes locked

    // lock the entire sound buffer
    if (FAILED(buffer->Lock(0, midBuffer * 2, &start, &locked, NULL, NULL,
     0))) {
        error("Segment::21 Couldn\'t lock the sound buffer");
    }
    else {
        // silence value depends upon the quantization
        if (bitsPerSample == 8u)
            FillMemory(start, locked, 0x80);
        else
            ZeroMemory(start, locked);
        // unlock the buffer
        buffer->Unlock(start, locked, NULL, 0);
        rc = true;
    }

    return rc;
}

// loadFileToBuffer fills the sound buffer with data from the sound file
// assuming that the file is a .wav file
//
bool Segment::loadFileToBuffer(unsigned offset, unsigned size, 
 unsigned flags) {

    bool rc = false;
    void* start;  // address of the start of the locked data
    DWORD locked; // number of bytes that have been locked

    if (file.is_open()) {
        // lock the sound buffer
        if (FAILED(buffer->Lock(offset, size, &start, &locked, NULL, NULL, 
         flags))) {
            error("Segment::22 Couldn\'t lock the sound buffer");
        }
        else {
            unsigned bytesRead;
            // if static, always rewind to the start of the sound data
            if (!streaming)
                file.seekg(44);
			// otherwise, start reading from the current file position
			//
            // for each case read data from the file into the sound buffer
            file.read((char*)start, locked);
            bytesRead = file.gcount();
            // if streaming sound check for end of file
            if (streaming && bytesRead < locked) {
				// the buffer has room for more data
                file.clear();
                // rewind to start of sound data
                file.seekg(44);
                // fill rest of this half of the buffer
                file.read((char*)start + bytesRead, locked - bytesRead);
            }
            // unlock the buffer
            buffer->Unlock(start, locked, NULL, 0);
            rc = true;
        }
    }
    else
        error("Segment::23 The sound file is not open");

    return rc;
}

// update updates the position and the orientation of the segment for an
// object sound
//
void Segment::update() {

    // for streaming sound, check if time to stream more data into buffer
    if (buffer && streaming) {
        unsigned current;
        buffer->GetCurrentPosition((DWORD*)&current, NULL);
        if (current >= midBuffer && lastPlayPos < midBuffer ||
         current < lastPlayPos) {
            unsigned start = current >= midBuffer ? 0 : midBuffer;
            if (!loadFileToBuffer(start, midBuffer, 0)) {
                error("Segment::50 Failed to stream file");
                sound->isSetToStart() = false;
                sound->isSetToStop()  = true;
            }
        }
        lastPlayPos = current;
    }

    if (buffer3D) {
        Vector p = sound->position();
        Vector h = sound->direction();
        buffer3D->SetPosition(p.x, p.y, p.z, DS3D_DEFERRED);
        buffer3D->SetConeOrientation(h.x, h.y, h.z, DS3D_DEFERRED);
        // settings will be committed through listener in SoundCard
    }

    if (buffer && sound && sound->isOn()) {
        // update the frequency of the sound
        if (sound->hasFrequencyControl()) 
            buffer->SetFrequency(sound->frequency());

        // update the volume of the sound
        if (sound->hasVolumeControl()) {
            long volume = (long)(volumeRange * sound->volume() + silence);
            buffer->SetVolume(volume);
        }
    }
}

// play starts/stops playing the sound segment
//
void Segment::play() {

    // create the sound segment if it doesn't exist
    if (!buffer) setup();

    // start playing the sound if it is marked for starting
    if (buffer && sound->isSetToStart()) {
        buffer->Play(0, 0, sound->isContinuous() ? DSBPLAY_LOOPING : 0);
        sound->isSetToStart() = false;
        sound->isOn() = true; 
    }
    // stop playing the sound if it is marked for stopping
    else if(buffer && sound->isSetToStop()) {
        buffer->Stop();
        sound->isSetToStop() = false;
        sound->isOn() = false;
    }
}

// suspend suspends the playback of the sound segment and prepares
// the segment for future restoration
//
void Segment::suspend() {

    if (buffer && sound) {
        buffer->Stop();
		sound->isSetToStart() = sound->isContinuous() && sound->isOn();
        sound->isSetToStop()  = false;
    }
	// if 3D sound save its current parameters
    if (buffer3D) {
        ds3Dbd.dwSize = sizeof(DS3DBUFFER);
        buffer3D->GetAllParameters(&ds3Dbd);
    }
}

// restore restores the playback of any sound segment that is on
//
void Segment::restore() {

    char str[81];
    unsigned status;

    // create the sound segment if it doesn't exist
    if (!buffer) setup();

    if (FAILED(buffer->GetStatus((DWORD*)&status)))
        error("Segment::70 GetStatus failed");
    else if (status & DSBSTATUS_BUFFERLOST) {
        // restore the sound buffer
        int count = 0;
        do {
            if (buffer->Restore() == DSERR_BUFFERLOST)
                Sleep(10);
            count++;
        } while (buffer->Restore() == DSERR_BUFFERLOST || count == 100);
        if (buffer->Restore() == DSERR_BUFFERLOST) {
             wsprintf(str, "Segment::71 Couldn\'t restore sound buffer"
              " for file %ls", sound->filename());
            error(str);
            release();
        }
        else {
            // fill the buffer - static or streaming
            // with silence if streaming
            if (streaming) {
                loadSilenceToBuffer();
                if (!loadFileToBuffer(0, midBuffer, 0)) {
                    error("Segment::72 Failed to restart stream");
                    release();
                }
            }
            // with file contents if static
            else if (!loadFileToBuffer(0, fileSize, DSBLOCK_ENTIREBUFFER)) {
                wsprintf(str, "Segment::73 Couldn\'t refill the "
                 "buffer with sound file %ls", sound->filename());
                error(str);
                release();
            }
            // reinitialize 3d sound parameters
            if (buffer3D) {
                if (FAILED(buffer3D->SetAllParameters(&ds3Dbd, 
                 DS3D_IMMEDIATE)))
                    error("Segment::73 SetAllParameters Failed");
            }
            update();
        }
    }
    else {
        // reinitialize 3d sound parameters
        if (buffer3D) {
            if (FAILED(buffer3D->SetAllParameters(&ds3Dbd, 
             DS3D_IMMEDIATE)))
                error("Segment::74 SetAllParameters Failed");
        }
        update();
    }
}

// release detaches the interfaces to the segment COM object
//
void Segment::release() {

    if (baseBuffer) {
        baseBuffer->Release();
        baseBuffer = NULL;
    }
    if (buffer) {
        buffer->Release();
        buffer = NULL;
    }
    if (buffer3D) {
        buffer3D->Release();
        buffer3D = NULL;
    }
}

// destructor deactivates the audio path, unloads the segments and
// releases the pointers to the interfaces
//
Segment::~Segment() {

    release();
}


