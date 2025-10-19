#ifndef _PARTICLE_H
#define _PARTICLE_H_

#include "IScene.h"
#include "ICameras.h"
#include <list>
#include <d3dx9.h>
#include "Utilities.h"
#include "math.h"



class ICamera;


	struct Particle
	{
		D3DXVECTOR3 _position;
		D3DCOLOR    _color;
		static const DWORD FVF;
	};

	struct Attribute
	{
		Attribute()
		{
			_lifeTime = 0.0f;
			_age      = 0.0f;
			_isAlive  = true;
		}

		D3DXVECTOR3 _position;     
		D3DXVECTOR3 _velocity;     
		D3DXVECTOR3 _acceleration; 
		float       _lifeTime;     // how long the particle lives for before dying  
		float       _age;          // current age of the particle  
		D3DXCOLOR   _color;        // current color of the particle   
		D3DXCOLOR   _colorFade;    // how the color fades with respect to time
		bool        _isAlive;      // if it is not alive, then kill the particle.
	};



class ParticleSystem {
	static const int MAX_PARTICLES = 100;
protected:

	static ParticleSystem* address_[MAX_PARTICLES]; //pointers to this object
	static int numpt;

	std::list<Attribute> _particles;
	IDirect3DDevice9*       _device;
	IDirect3DTexture9*      _tex;
	IDirect3DVertexBuffer9* _vb;
	int _maxParticles; // max allowed particles system can have
	float _emitRate;   // rate new particles are added to system
	float _size;       // size of particles

	DWORD _vbSize;      // size of vb
	DWORD _vbOffset;    // offset in vb to lock   
	DWORD _vbBatchSize; // number of vertices to lock starting at _vbOffset

	BoundingBox        _boundingBox; //where the particles are alive

	ICamera* camera;
	IObject* obj;

protected:
	virtual void removeDeadParticles();

public:
	ParticleSystem();
	static void address(ParticleSystem** pArray) {
		for(int i=0; i < numpt; i++) {
			pArray[i] = address_[i];
		}
	}
	virtual ~ParticleSystem();

	virtual bool init(char* texFileName, IDirect3DDevice9* device);
	virtual void reset();
	virtual void addParticle();
	virtual void resetParticle(Attribute* attribute) = 0;

	virtual void preRender();
	virtual void render();
	virtual void postRender();
	virtual void update(float timeDelta) = 0;

	bool isEmpty();
	bool isDead();
};


	class ParticleGun : public ParticleSystem
	{
	public:
		ParticleGun(ICamera* camera, IObject* obj);
		void resetParticle(Attribute* attribute);
		void update(float timeDelta);
	private:

	};

class Snow : public ParticleSystem{
	public:
		Snow(BoundingBox* boundingBox, int numParticles);
		void resetParticle(Attribute* attribute);
		void update(float timeDelta);
	};


static void ParticleSystemAddress(ParticleSystem** pa) {

	 ParticleSystem::address(pa);
}

#endif
