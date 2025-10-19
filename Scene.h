#ifndef _SCENE_H_
#define _SCENE_H_

/* Header for the Scene Module
 *
 * consists of Scene declaration
 *             Object declaration
 *             Box declaration
 *             SoundBox declaration
 *             ViewingBox declaration
 *             Grid declaration
 *             Vertex declaration
 *             Texture declaration
 *
 * Scene.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "IScene.h"
#include "Body.h"  // for Frame
#include "Particle.h"





//-------------------------------- Scene ---------------------------------
//
// Scene manages the scene component of the model
//
class IText;
class ISound;
class ICamera;
enum CameraType;

class Scene : public IScene {

	static const int MAX_OBJECTS  = 50;
	static const int MAX_TEXTURES = 50;
	static const int MAX_CHAR     = 80;

    IObject*   object[MAX_OBJECTS];   // points to the objects in the scene
	ITexture*  texture[MAX_TEXTURES]; // points to the textures in the scene
    IKeyboard* keyboard;              // points to the keyboard object
    IMouse*    mouse;                 // points to the mouse object
    IJoystick* joystick;              // points to the joystick object
    IAudio*    audio;                 // points to the audio object
	ITexture*  background;            // points to the background texture
	IObject*   terrain;               // points to the terrain
    IHUD*      hud;                   // points to the heads up display
    IText*     heroX;              // points to spinner x axis orientation
    IText*     heroY;              // points to spinner y axis orientation
    IText*     heroZ;              // points to spinner z axis orientation
    IText*     heroPos;              // points to spinner z axis orientation
	ICamera*   camera;                // points to camera attached to object
	ICamera*   heroCamera;           // points to camera attached to hero
	ICameras*  cameras;               // points to the set of cameras

	ParticleSystem* ps[10]; //particle implementation

	IObject*   sun;
	IObject*   moon;
	
	IObject* mainHero;
	ITexture* tree;
	IObject* treeObj;

	int noObjects;                    // number of objects
	int noTextures;                   // number of textures
    int lastUpdate;                   // time that the scene was last updated

	int   backgroundWidth;    // width of the background image in pixels
	float backgroundRatio;    // height to width ratio of background image
	float backgroundTopLeftX; // x coordinate of top left corner ofthe image

 
    Scene(const Scene& s);            // prevents copying
    Scene& operator=(const Scene& s); // prevents assignment
    bool    add(IObject*);
    bool    remove(IObject*);
    bool    add(ITexture*);
    bool    remove(ITexture*);


protected:
   Scene(IKeyboard* k, IMouse* m, IJoystick* , IAudio* a, IHUD* h, ICameras* c);
   	IObject** const objects() const { return (IObject**)object; }
	int       numberObjects() const { return noObjects; }
	    virtual ~Scene();

  public:
	friend IScene* CreateScene(IKeyboard* k, IMouse* m, IJoystick* j, 
	 IAudio* a, IHUD* h, ICameras* c);
    bool   setup(int now);
    void   update(int now);
	void   drawBackground();
    void   drawOpaque();
	void   drawTranslucent();
    void   suspend();
    bool   restore(int now);
	void   Delete() { delete this; }
	IObject* returnCurrentHero() { return mainHero; }
	IObject* returnCurrentTerrain() { return terrain; }
	friend class Object;
	friend class Texture;
};

//-------------------------------- AnimatedScene -------------------------
//
// AnimatedScene is a Scene with moving Objects that may collide with one
// another
//
class AnimatedScene : public Scene {

	IObject** const object; // points to arrays of objects
	int noObjects;          // current number of objects
	int lastUpdate;         // time that the scene was last updated

    AnimatedScene(IKeyboard* k, IMouse* m, IJoystick* , ICameras* c,
	 IAudio* a, IHUD* h);
    AnimatedScene(const Scene& s);            // prevents copying
    AnimatedScene& operator=(const Scene& s); // prevents assignment

  public:
	friend IScene* CreateScene(IKeyboard* k, IMouse* m, IJoystick* j, 
	 ICameras* c, IAudio* a, IHUD* h);
    bool setup(int now);
	void update(int now);
    bool restore(int now);
	friend class Object;
};

//-------------------------------- Object --------------------------------
//
// Object holds the information for a single object in the scene
//
class ISound;
class Vertex;
enum Shape;

class Object : public Body {

	static IScene* scene;  // points to the scene manager
    IGraphic* visual;      // points to the graphic representation
    Vertex*   vertex;      // defines unique vertices
	short*    index;       // defines primitives in terms of vertices
	unsigned* attribute;   // defines surfaces in terms of primitives
	int       nPrimitives; // number of primitives in the set
	int       nVertices;   // number of vertices currently stored
	int       nIndices;    // number of indices currently stored
	int       nSurfaces;   // number of surfaces currently stored

    void release();

	void add();                          // adds object to the scene
    Object(const Object& o);             // prevent copies
    Object& operator=(const Object& o);  // prevent assignment

  protected:
	Object(Shape shape, float* dimension, int* partition, Colour clr, 
	 bool antiAlias);
	Object(Shape shape, int noPrimitives, int noVertices, Colour clr, 
	 ITexture* texture, bool antiAlias);
    Object(Shape shape, int noPrimitives, int noVertices, Colour* clr, 
	 ITexture** texture, int nSubsets, bool antiAlias);
	Object(Shape shape, const char* filename, bool antiAlias);
     void add(Vector p1, Vector p2, Vector p3, Vector p4, Vector n);
	int  add(float x, float y, float z, float nx, float ny, float nz, 
	 float tu = 0, float tv = 0);
    void add(Vector p1, Vector p2, Vector p3, Vector p4, Vector n, int tu, 
	 int tv);
	void add(int vertex);
	Vector local(int i) const;
    virtual ~Object();

  public:
	friend IObject* CreateSphere(float radius, int slices, int stacks, 
	 Colour c, bool antiAlias);
	friend IObject* CreateCylinder(float upper, float lower, float height, 
	 int slices, int stacks, Colour c, bool antiAlias);
	friend IObject* CreateTorus(float inner, float outer, int slices, 
	 int stacks, Colour c, bool antiAlias);
	friend IObject* CreateTeapot(Colour c, bool antiAlias);
	friend IObject* CreateXFile(const char* filename, bool antiAlias);
	IGraphic* graphic() const { return visual; }
	void   populateVB(void* vb) const;
	void   populateIB(void* ib) const;
	void   populateAB(void* ab) const;
	void   add(ITexture* texture);
	void   orient() {}
	void   align(IObject*, float, float, ICameras*) const {}
	void   Delete() { delete this; }
//	void returnBoundingBoxMin() {}
	
	friend class Scene;
};

//-------------------------------- Billboard -----------------------------
//
// Billboard is a two dimensional rectangle that always faces the current
// camera 
//
class Billboard : public Object {

	ICameras* cameras; // points to the set of cameras
	Orientation type;  // method of orientation

  protected:
	Billboard(Orientation orient, ICameras* cs, float minx, float miny, 
	 float maxx, float maxy, Colour c, ITexture* texture);
	virtual ~Billboard() {}
  public:
	friend IObject* CreateBillboard(Orientation orient, ICameras* cs, 
	 float minx, float miny, float maxx, float maxy, Colour c,
	 ITexture* texture);
	void orient();
};

//-------------------------------- Box -------------------------------------
//
// Box is an Object identifiable by two points - lower close left, and
// upper faraway right - and a colour
//
class Box : public Object {

  protected:
    Box(float minx, float miny, float minz, float maxx, float maxy,
     float maxz, Colour c, ITexture* texture, int tu, int tv, 
	 bool antiAlias);
    Box(float minx, float miny, float minz, float maxx, float maxy,
     float maxz, Colour* c, ITexture** texture, int tu, int tv, 
	 bool antiAlias);
	virtual ~Box() {}

  public:
	friend IObject* CreateBox(float minx, float miny, float minz, 
	 float maxx, float maxy, float maxz, Colour c, ITexture* texture, 
	 int tu, int tv, bool antiAlias);
	friend IObject* CreateBox(float minx, float miny, float minz, 
	 float maxx, float maxy, float maxz, Colour* c, ITexture** texture, 
	 int tu, int tv, bool antiAlias);
};

//-------------------------------- Grid ------------------------------------
//
// Grid is a set of mutually perpendicular lines in a plane defined 
// parallel to the x-y plane
//
class Grid : public Object {

  protected:
    Grid(int min, int y, int max, int n, Colour clr, bool antiAlias);
	virtual ~Grid() {}

  public:
	friend IObject* CreateGrid(int min, int y, int max, int n, Colour clr, 
	 bool antiAlias);
};

//-------------------------------- Terrain -------------------------------
//
// A Terrain is an Object that consists of a grid of triangles where the
// height of each vertex is specified using a height map
//
class ifstream;

class Terrain : public Object {

    int rows;       // number of vertices in the z direction
    int cols;       // number of vertices in the x direction
    int spacing;    // spacing between vertices in either x or z direction
//	Vector min;
//	Vector max;

	Terrain(std::ifstream& fp, unsigned width, unsigned height, 
	 unsigned bitdepth, int cellSpacing, float depth, ITexture* tFile);

    float greyScale(void* pixel, unsigned int depthInBytes);

  public:
    friend IObject* CreateTerrain(int cellSpacing, float depth, 
	 const char* heightMap, ITexture* tFile);
    void align(IObject* object, float dxx, float dzz, ICameras* camera) const;
	//particle implementation
//	void returnBoundingBoxMin() {  }
//	Vector returnBoudningBoxMax() { return max; }
};

//-------------------------------- Vertex ----------------------------------
//
// Vertex holds the data that describes a single vertex of a graphics
// primitive
//
class Vertex {

    float x;   // x coordinate in the local frame
    float y;   // y coordinate in the local frame
    float z;   // z coordinate in the local frame
    float nx;  // x coordinate of normal in local frame
    float ny;  // y coordinate of normal in local frame
    float nz;  // z coordinate of normal in local frame
    float tu;  // u coordinate of texture
    float tv;  // v coordinate of texture

  public:
    Vertex(float xx = 0, float yy = 0, float zz = 0, float nxx = 0, 
	 float nyy = 0, float nzz = 0, float ttu = 0, float ttv = 0);
	Vector position() const;
};

//-------------------------------- Texture -------------------------------
//
// A Texture holds the address of the deviceTexture for a single texture
//
class Texture : public ITexture {

	static IScene* scene;           // points to the scene manager
	IDeviceTexture* deviceTexture_; // points to the representation on the
	                                // graphics card 

	Texture(const char* file, unsigned flags, Colour brdrClr);
	Texture(const Texture&);
	Texture& operator=(const Texture&);
	virtual ~Texture();

  public:
	friend ITexture* CreateTexture(const char* file, unsigned flags);
	friend ITexture* CreateTexture(const char* file, unsigned flags,
	 Colour brdrClr);
	IDeviceTexture* deviceTexture() const { return deviceTexture_; }
	void Delete() { delete this; }
	friend class Scene;
};

#endif
