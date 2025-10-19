#ifndef _I_SOUND_CARD_H_
#define _I_SOUND_CARD_H_

/* Interface to the SoundCard Module
 *
 * consists of ISoundCard interface
 *             ISegment interface
 *
 * ISoundCard.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

//------------------------------- ISoundCard --------------------------------
//
// ISoundCard is the interface to the Sound Card component of the device set
//
struct Vector;
class IAudio;

class ISoundCard {
  public:
    virtual bool setup()                                              = 0;
    virtual void play(const Vector& p, const Vector& h, 
	 const Vector& u)                                                 = 0;
    virtual void release()                                            = 0;
	virtual void Delete()                                             = 0;
};

extern "C"
ISoundCard* CreateSoundCard(IAudio* audio);

//-------------------------------- Segment -------------------------------
//
// ISegment is the interface to a sound segment on the SoundCard
//
class ISound;

class ISegment {
  public:
    virtual bool setup()       = 0;
    virtual void play()        = 0;
    virtual void update()      = 0;
    virtual void suspend()     = 0;
	virtual void restore()     = 0;
	virtual void Delete()      = 0;
};

extern "C"
ISegment* CreateSegment(ISound* sound);

#endif
