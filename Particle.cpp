#include "Particle.h"
#include <cstdlib>

const DWORD Particle::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
ParticleSystem* ParticleSystem::address_[];
int ParticleSystem::numpt = NULL;

ParticleSystem::ParticleSystem() {
	_vb     = 0;
	_tex    = 0;
	_device = 0;

	if(numpt < MAX_PARTICLES)
		address_[numpt++] = this;

}

ParticleSystem::~ParticleSystem()
{
	::Release<IDirect3DVertexBuffer9*>(_vb);
	::Release<IDirect3DTexture9*>(_tex);
}

bool ParticleSystem::init(char* texFileName, IDirect3DDevice9* device) {
	
	HRESULT hr = 0;
	_device = device;
	

	hr = device->CreateVertexBuffer(
		_vbSize * sizeof(Particle),
		D3DUSAGE_DYNAMIC | D3DUSAGE_POINTS | D3DUSAGE_WRITEONLY,
		Particle::FVF,
		D3DPOOL_DEFAULT, // D3DPOOL_MANAGED can't be used with D3DUSAGE_DYNAMIC 
		&_vb,
		0);

	if(FAILED(hr))
	{
		::MessageBox(0, "CreateVertexBuffer() - FAILED", "PSystem", 0);
		return false;
	}

	hr = D3DXCreateTextureFromFile(
		device,
		texFileName,
		&_tex);

	if(FAILED(hr))
	{
		::MessageBox(0, "D3DXCreateTextureFromFile() - FAILED", "PSystem", 0);
		return false;
	}

	return true;

}

void ParticleSystem::reset()
{
	std::list<Attribute>::iterator i;
	for(i = _particles.begin(); i != _particles.end(); i++)
	{
		resetParticle( &(*i) );
	}
}

//add a particle into system, you will call this function when you try to add a particle
void ParticleSystem::addParticle()
{
	Attribute attribute;

	resetParticle(&attribute); //reset every particles for drawing

	_particles.push_back(attribute);
}

//set renderstates for drawing particles
void ParticleSystem::preRender()
{
	_device->SetRenderState(D3DRS_LIGHTING, false);
	_device->SetRenderState(D3DRS_POINTSPRITEENABLE, true);
	_device->SetRenderState(D3DRS_POINTSCALEENABLE, true); 
	_device->SetRenderState(D3DRS_POINTSIZE,  FtoDw(_size)); //set the size of particle
	_device->SetRenderState(D3DRS_POINTSIZE_MIN, FtoDw(0.0f)); //FtoDW cats floats to DWORD

	// control the size of the particle relative to distance
	_device->SetRenderState(D3DRS_POINTSCALE_A, FtoDw(0.0f));
	_device->SetRenderState(D3DRS_POINTSCALE_B, FtoDw(0.0f));
	_device->SetRenderState(D3DRS_POINTSCALE_C, FtoDw(1.0f));
		
	// use alpha from texture
	_device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	_device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

	_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    _device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

//resets renderstates to original values for futher rendering of other things.
void ParticleSystem::postRender()
{
	_device->SetRenderState(D3DRS_LIGHTING,          true);
	_device->SetRenderState(D3DRS_POINTSPRITEENABLE, false);
	_device->SetRenderState(D3DRS_POINTSCALEENABLE,  false);
	_device->SetRenderState(D3DRS_ALPHABLENDENABLE,  false);
}

void ParticleSystem::render()
{
	//
	// Remarks:  The render method works by filling a section of the vertex buffer with data,
	//           then we render that section.  While that section is rendering we lock a new
	//           section and begin to fill that section.  Once that sections filled we render it.
	//           This process continues until all the particles have been drawn.  The benifit
	//           of this method is that we keep the video card and the CPU busy.  

	if( !_particles.empty() )
	{
		//
		// set render states
		//

		preRender();
		
		_device->SetTexture(0, _tex);
		_device->SetFVF(Particle::FVF);
		_device->SetStreamSource(0, _vb, 0, sizeof(Particle));

		//
		// render batches one by one
		//

		// start at beginning if we're at the end of the vb
		if(_vbOffset >= _vbSize)
			_vbOffset = 0;

		Particle* v = 0;

		_vb->Lock(
			_vbOffset    * sizeof( Particle ),
			_vbBatchSize * sizeof( Particle ),
			(void**)&v,
			_vbOffset ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD);

		DWORD numParticlesInBatch = 0;	//for counting

		//
		// Until all particles have been rendered.
		//
		std::list<Attribute>::iterator i;
		for(i = _particles.begin(); i != _particles.end(); i++)
		{
			if( i->_isAlive )
			{
				//
				// Copy a batch of the living particles to the
				// next vertex buffer segment
				//
				v->_position = i->_position;
				v->_color    = (D3DCOLOR)i->_color;
				v++; // next element;

				numParticlesInBatch++; //increase batch counter

				// if this batch full?
				if(numParticlesInBatch == _vbBatchSize) 
				{
					//
					// Draw the last batch of particles that was
					// copied to the vertex buffer. 
					//
					_vb->Unlock();

					_device->DrawPrimitive(
						D3DPT_POINTLIST,
						_vbOffset,
						_vbBatchSize);

					//
					// While that batch is drawing, start filling the
					// next batch with particles.
					//

					// move the offset to the start of the next batch
					_vbOffset += _vbBatchSize; 

					// don't offset into memory thats outside the vb's range.
					// If we're at the end, start at the beginning.
					if(_vbOffset >= _vbSize) 
						_vbOffset = 0;       

					_vb->Lock(
						_vbOffset    * sizeof( Particle ),
						_vbBatchSize * sizeof( Particle ),
						(void**)&v,
						_vbOffset ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD);

					numParticlesInBatch = 0; // reset for new batch
				}	
			}
		}

		_vb->Unlock();

		// its possible that the LAST batch being filled never 
		// got rendered because the condition 
		// (numParticlesInBatch == _vbBatchSize) would not have
		// been satisfied.  We draw the last partially filled batch now.
		
		if( numParticlesInBatch )
		{
			_device->DrawPrimitive(
				D3DPT_POINTLIST,
				_vbOffset,
				numParticlesInBatch);
		}

		// next block
		_vbOffset += _vbBatchSize; 

		//
		// reset render states
		//

		postRender();
	}
}

bool ParticleSystem::isEmpty()
{
	return _particles.empty();
}

bool ParticleSystem::isDead()
{
	std::list<Attribute>::iterator i;
	for(i = _particles.begin(); i != _particles.end(); i++)
	{
		// is there at least one living particle?  If yes,
		// the system is not dead.
		if( i->_isAlive )
			return false;
	}
	// no living particles found, the system must be dead.
	return true;
}

void ParticleSystem::removeDeadParticles() {

		std::list<Attribute>::iterator i;

	i = _particles.begin();

	while( i != _particles.end() )
	{
		if( i->_isAlive == false )
		{
			// erase returns the next iterator, so no need to
		    // incrememnt to the next one ourselves.
			i = _particles.erase(i); 
		}
		else
		{
			i++; // next in list
		}
	}
}


//*****************************************************************************
// Laser System
//****************

ParticleGun::ParticleGun(ICamera* cam, IObject* obj_)
{
	camera          = cam;
	_size            = 6.0f;
	_vbSize          = 2048;
	_vbOffset        = 0;  
	_vbBatchSize     = 512; 
	obj = obj_;
}

void ParticleGun::resetParticle(Attribute* attribute)
{
	attribute->_isAlive  = true;
	
	Vector objPos;
	objPos = obj->position();
	Vector heading = camera->heading();
	


		
	// change to camera position
	attribute->_position = D3DXVECTOR3(objPos.x, objPos.y, objPos.z);

	attribute->_position.y += 26.0f; // slightly above camera to match fire(shot) to gunpoint


	// travels in the direction the camera is looking
	attribute->_velocity = D3DXVECTOR3(heading.x, heading.y, heading.z) * 1000.0f;

	attribute->_color = D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f);
		
	attribute->_age      = 0.0f; 
	attribute->_lifeTime = 1.0f; // lives for 1 seconds
}

void ParticleGun::update(float timeDelta)
{
	std::list<Attribute>::iterator i;

	for(i = _particles.begin(); i != _particles.end(); i++)
	{
		i->_position += i->_velocity * timeDelta;

		i->_age += timeDelta;

		if(i->_age > i->_lifeTime) // kill 
			i->_isAlive = false;
	}
	removeDeadParticles();
}

//*****************************************************************************
// Snow System
//***************

Snow::Snow(BoundingBox* boundingBox, int numParticles)
{
	_boundingBox   = *boundingBox;
	_size          = 5.0f;
	_vbSize        = 2048; //how much particles one vertex buffer can store
	_vbOffset      = 0; //where is start point
	_vbBatchSize   = 512; //how much particles one segment can store
	
	for(int i = 0; i < numParticles; i++)
		addParticle();
}

void Snow::resetParticle(Attribute* attribute)
{
	attribute->_isAlive  = true;

	// get random x, z coordinate for the position of the snow flake.
	GetRandomVector(
		&attribute->_position,
		&_boundingBox._min,
		&_boundingBox._max);

	// no randomness for height (y-coordinate).  Snow flake
	// always starts at the top of bounding box.
	attribute->_position.y = _boundingBox._max.y; 

	// snow flakes fall downwards and slightly to the left
	attribute->_velocity.x = GetRandomFloat(0.0f, 1.0f) * -3.0f;
	attribute->_velocity.y = GetRandomFloat(0.0f, 1.0f) * -10.0f;
	attribute->_velocity.z = 0.0f;

	// white snow flake
	attribute->_color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0);
}

void Snow::update(float timeDelta)
{
	std::list<Attribute>::iterator i;
	for(i = _particles.begin(); i != _particles.end(); i++)
	{
		i->_position += i->_velocity * timeDelta;

		// is the point outside bounds? (kill if particles exist outside of Bounding box
		if( _boundingBox.isPointInside( i->_position ) == false ) 
		{
			// nope so kill it, but we want to recycle dead 
			// particles, so respawn it instead.
			resetParticle( &(*i) );
		}
	}
}
