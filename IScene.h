#ifndef _I_SCENE_H_
#define _I_SCENE_H_

/* Interface to the Scene Module
 *
 * consists of IScene interface
 *             IObject interface
 *             ITexture interface
 *
 * IScene.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

//-------------------------------- IScene --------------------------------
//
// IScene is the interface to the scene component of the model
//
class IObject;
class ITexture;
class IKeyboard;
class IMouse;
class IJoystick;
class IAudio;
class IHUD;
class ICameras;

class IScene {
  public:
    virtual bool add(IObject* o)     = 0;
	virtual bool remove(IObject* o)  = 0;
    virtual bool add(ITexture* t)    = 0;
	virtual bool remove(ITexture* t) = 0;
	virtual bool setup(int now)      = 0;
    virtual void update(int now)     = 0;
	virtual void drawBackground()    = 0;
    virtual void drawOpaque()        = 0;
	virtual void drawTranslucent()   = 0;
    virtual void suspend()           = 0;
    virtual bool restore(int now)    = 0;
    virtual void Delete()            = 0;
	virtual IObject* returnCurrentHero() = 0;
	virtual IObject* returnCurrentTerrain() = 0;
};

extern "C"
IScene* CreateScene(IKeyboard* k, IMouse* m, IJoystick* j, IAudio* a, 
 IHUD* h, ICameras* c);

//-------------------------------- IObject -------------------------------
//
// IObject is the interface to a single object in the scene
//
class IGraphic;
struct Vector;
struct Matrix;
struct Colour;

class IObject {
  public:
    virtual IGraphic* graphic() const                               = 0;
	virtual void populateVB(void* vb) const                         = 0;
	virtual void populateIB(void* ib) const                         = 0;
	virtual void populateAB(void* ab) const                         = 0;
	virtual void add(ITexture* texture)                             = 0;//xx
    virtual void move(float x, float y, float z)                    = 0;

			virtual void velocity(float x, float y, float z)                = 0;
		virtual void accelerate(float x, float y, float z)              = 0;
		virtual void angularVelocity(float x, float y, float z)         = 0;
		virtual void angularAcceleration(float x, float y, float z)     = 0;
				virtual void update(float dt)                                   = 0;
						virtual Vector velocity() const                                 = 0;
		virtual Vector acceleration() const                             = 0;

    virtual void scale(float x, float y, float z)                   = 0;
    virtual void rotatex(float rad)                                 = 0;
    virtual void rotatey(float rad)                                 = 0;
    virtual void rotatez(float rad)                                 = 0;
	virtual void rotate(const Matrix& rot)                          = 0;
	virtual void orient()                                           = 0;
    virtual void align(IObject* object, float dxx, float dzz, ICameras* camera) const = 0;
    virtual Vector position() const                                 = 0;
	virtual Matrix rotation() const                                 = 0;
    virtual Vector orientation(char axis) const                     = 0;
	virtual Vector orientation(const Vector& v) const               = 0;
	virtual Matrix world() const                                    = 0;
	virtual void attach(IObject* newParent, bool reset = true)      = 0;
	virtual void detach()                                           = 0;
	virtual void Delete()                                           = 0;
	virtual void planetRotate(float rad)							= 0;
	//virtual Vector returnBoundingBoxMin()							= 0;
	//virtual Vector returnBoudningBoxMax()							= 0;
	//virtual void returnBoundingBoxMin() = 0;
	virtual Vector returnBoundingMin() = 0;
	virtual Vector returnBoungingMax() = 0;
	
};

IObject* CreateBox(float minx, float miny, float minz, float maxx, 
 float maxy, float maxz, Colour clr, ITexture* texture = 0, int tu = 1, 
 int tv = 1, bool antiAlias = false);

IObject* CreateBox(float minx, float miny, float minz, float maxx, 
 float maxy, float maxz, Colour* clr, ITexture** texture = 0, int tu = 1, 
 int tv = 1, bool antiAlias = false);

IObject* CreateSphere(float radius, int slices, int stacks, Colour c, 
 bool antiAlias = true);

IObject* CreateCylinder(float upper, float lower, float height, 
 int slices, int stacks, Colour c, bool antiAlias = false);

IObject* CreateTorus(float inner, float outer, int slices, int stacks, 
 Colour c, bool antiAlias = true);

IObject* CreateTeapot(Colour c, bool antiAlias = false);

IObject* CreateXFile(const char* filename, bool antiAlias = false);

typedef enum Orientation {
	SCREEN,
	VIEW_PLANE,
	VIEWPOINT,
	AXIAL,
	INVERSE
} Orientation;

IObject* CreateBillboard(Orientation orient, ICameras* cs, float minx, 
 float miny, float maxx, float maxy, Colour c, ITexture* texture);

extern "C"
IObject* CreateGrid(int min, int y, int max, int n, Colour clr, 
 bool antiAlias = false);

IObject* CreateTerrain(int cellSpacing, float depth, 
 const char* heightMap, ITexture* tFile = 0);

bool detectCollision(IObject* object_i, IObject* object_j, float& dt,
 Vector& normal, Vector& contact);

void collide(IObject* object_i, IObject* object_j, const Vector& c, 
 const Vector& n);

//-------------------------------- ITexture ------------------------------
//
// ITexture is the interface to a single texture
//
class IDeviceTexture;

class ITexture {
  public:
	virtual IDeviceTexture* deviceTexture() const = 0;
	virtual void Delete()                         = 0;
};

ITexture* CreateTexture(const char* file, unsigned flags = 0);

ITexture* CreateTexture(const char* file, unsigned flags,
 Colour brdrClr);

#endif
