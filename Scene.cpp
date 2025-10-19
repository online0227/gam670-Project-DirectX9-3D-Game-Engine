/* Scene Module Implementation
 *
 * Scene.cpp
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include <fstream>
using namespace std;
#include "IInput.h"        // for Keyboard, Mouse, Joystick interfaces
#include "IAudio.h"        // for Audio and Sound interfaces
#include "ICameras.h"      // for Camera
#include "IHUD.h"          // for HUD and Text interfaces
#include "IGraphicsCard.h" // for Graphic and DeviceTexture interfaces
#include "ModelSettings.h" // for FLOOR, MOUSE_BUTTON_SCALE, ROLL_SPEED
#include "Utilities.h"     // for error()
#include "Scene.h"         // for Scene, Object, Box, SoundBox, Grid,
                           // Vertex, Texture class declarations

//-------------------------------- Scene ---------------------------------
//
// Scene manages the scene component of the model
//
IScene* CreateScene(IKeyboard* k, IMouse* m, IJoystick* j, IAudio* a, 
 IHUD* h, ICameras* c) {

	return new Scene(k, m, j, a, h, c);
}

Scene::Scene(IKeyboard* k, IMouse* m, IJoystick* j, IAudio* a, IHUD* h, ICameras* c) :
 keyboard(k), mouse(m), joystick(j), audio(a), hud(h), cameras(c) {

    noObjects = 0;
    for (int i = 0; i < MAX_OBJECTS; i++)
        object[i] = NULL;

	noTextures = 0;
    for (int i = 0; i < MAX_TEXTURES; i++)
        texture[i] = NULL;
	background         = NULL;
	backgroundTopLeftX = 0;
	backgroundWidth    = BACKGROUND_WIDTH;
	backgroundRatio    = (float) BACKGROUND_HEIGHT / BACKGROUND_WIDTH;

	/* 내껏 */
	tree = NULL;

    heroX  = NULL;
    heroY  = NULL;
    heroZ  = NULL;
	heroPos = NULL;

    lastUpdate     = 0;
	Object::scene  = this;
	Texture::scene = this;

	//particle implementation
	/* you can't get particle system pointer address here.
	because of that, the first particle system initialization is done in Display::setup.
	if you try to receive a pointer to particle system here, it means that you will receive a NULL pointer.
	Look at the Game::setup() and see the Order.
	*/
	
	
}

// add adds a pointer to Object *o to the scene
// and returns true if successful, false otherwise
//
bool Scene::add(IObject* o) {

    int i;
    bool rc = false;

    for (i = 0; i < noObjects; i++)
        if (!object[i]) {
            object[i] = o;
            i = noObjects + 1;
            rc = true;
        }
    if (i == noObjects && noObjects < MAX_OBJECTS) {
        object[noObjects++] = o;
        rc = true;
    }

    return rc;
}

// add adds a pointer to ITexture *t to the scene
// and returns true if successful, false otherwise
//
bool Scene::add(ITexture* t) {

    int i;
    bool rc = false;

    for (i = 0; i < noTextures; i++)
        if (!texture[i]) {
            texture[i] = t;
            i = noTextures + 1;
            rc = true;
        }
    if (i == noTextures && noTextures < MAX_TEXTURES) {
        texture[noTextures++] = t;
        rc = true;
    }

    return rc;
}

// setup creates the objects and textures in the initiale scene
//
bool Scene::setup(int now) {

    lastUpdate = now;

	

	// create colours
	Colour white(1, 1, 1, 1);
	Colour black(0, 0, 0, 1);
	Colour clr919(0.9f, 0.1f, 0.9f, 1);
	Colour clr118(0.1f, 0.1f, 0.8f, 1);
	Colour clr181(0.1f, 0.8f, 0.1f, 1);
	Colour clr111(0.1f, 0.1f, 0.1f, 1);
	Colour clr881(0.8f, 0.8f, 0.1f, 1);
	Colour clr199(0.1f, 0.9f, 0.9f, 1);
	Colour clr631(0.6f, 0.3f, 0.1f, 1);
	Colour clr361(0.3f, 0.6f, 0.1f, 1);
	Colour clr136(0.1f, 0.3f, 0.6f, 1);
	Colour clr168(0.1f, 0.6f, 0.8f, 1);
	Colour clr668(0.6f, 0.6f, 0.8f, 1);
	Colour clr821(0.8f, 0.2f, 0.1f, 1);
	Colour clr861(0.8f, 0.6f, 0.1f, 1);
	Colour clt136(1.0f, 1.0f, 1.0f, 0);
	Colour red(1, 0, 0, 1);
	Colour grey(0.7f, 0.7f, 0.7f, 1);

	tree = CreateTexture("tree01S.dds"); //tree environment

	// create textures
	background = CreateTexture(BACKGROUND);

    // initial objects
	Colour clr[] = {clr919, clr181, clr118, clr881, clr111, clr199}; 

	sun = CreateSphere(30.0f, 20, 20, red); 
    sun->move(-500, 500, 0);

	moon = CreateSphere(30.0f, 20, 20, grey); 
    moon->move(500, -500, 0);

	// put trees on the plate
	add(CreateBillboard(VIEWPOINT, cameras, -50, -50, 50, 50, clt136, tree));
	object[noObjects-1]->move(-100, 30, 200);

	add(CreateBillboard(VIEWPOINT, cameras, -50, -100, 50, 50, clt136, tree));
	object[noObjects-1]->move(-310, 30, -255);

	add(CreateBillboard(VIEWPOINT, cameras, -25, -25, 50, 50, clt136, tree));
	object[noObjects-1]->move(110, 30, -150);

	add(CreateBillboard(VIEWPOINT, cameras, -75, -75, 50, 50, clt136, tree));
	object[noObjects-1]->move(450, 30, 220);


	mainHero = CreateXFile("cannonS.x");
	mainHero->scale(0.3f, 0.3f, 0.3f);

	
	
	mainHero->move(0, 0, 0);

    // Terrain and hero objects
    //
	ITexture* terrainTexture = CreateTexture("terrain2.bmp");
    terrain = CreateTerrain(4, 50, "heightmap.bmp", terrainTexture);
    terrain->move(0, FLOOR + 20, 100);


	heroCamera = CreateCamera(Vector(100, 100, 200), Vector(0, 0, -1), Vector(0, 1, 0), mainHero, SECOND_PERSON);

	heroCamera->attach(mainHero, false);
	cameras->setCamera(heroCamera, true);




    terrain->align(mainHero, 0, 1, cameras);

    heroX = CreateText(0, 0.30f, 1, 0.47f);
    heroY = CreateText(0, 0.37f, 1, 0.54f);
    heroZ = CreateText(0, 0.44f, 1, 0.61f);
    heroPos = CreateText(0, 0.51f, 1, 0.68f);

    return noObjects <= MAX_OBJECTS;
}

// update updates the position and orientation of each object for time "now"
// according to the keys pressed
//
void Scene::update(int now) {

    int delta;
    int dr = 0;  // roll the right box around its x axis
    int ds = 0;  // spin the right box around its y axis
    int dt = 0;  // roll the top   box around its z axis
	float df = 0;      // move hero along x direction
	float dg = 0;      // move hero along y direction
	int   joy_x = 0,     // joystick x-axis: turn left/right
          joy_y = 0,     // joystick y-axis: advance/retreat
          joy_z = 0,     // joystick z-axis: turn up/down
          joy_rz = 0;    // joystick rz-axis:
	static int dw = 1; // direction control on hero

    delta = now - lastUpdate;
    lastUpdate = now;

	::ParticleSystemAddress(ps);

	for(int i=0; i < noObjects; i++)
		object[i]->orient();

    // joystick input rotates the camera
    joystick->handle(joy_x, joy_y, joy_z, joy_rz);



	
	switch(cameras->returnType()) {
		case SECOND_PERSON:
			if (keyboard->pressed('D'))
				dg += delta;
			if (keyboard->pressed('A'))
				dg -= delta;
			if (keyboard->pressed('S'))
				df -= delta;
			if (keyboard->pressed('W'))
				df += delta;

			if (joy_y)
				 df += -joy_y;
		    if (joy_x)
				 dg -= -joy_x;

			if(mouse->pressed(LEFT_BUTTON)) {
				ps[0]->addParticle(); //particle implementation
			}

			break;


		case THIRD_PERSON:
			if (keyboard->pressed(NUM6))
				dg += delta;
			if (keyboard->pressed(NUM4))
				dg -= delta;
			if (keyboard->pressed(NUM5))
				df -= delta;
			if (keyboard->pressed(NUM8))
				df += delta;
			break;

		default:
			break;
	}

	sun->planetRotate(CONSTANT_ROLL * 2);
	moon->planetRotate(CONSTANT_ROLL * 2);



    if (hud->isOn()) {
        char str[MAX_CHAR + 1];
        Vector v;
        v = mainHero->orientation('x');
        wsprintf(str, "Hero x orientation %d, %d, %d", 
         (int)(100 * v.x), (int)(100 * v.y), (int)(100 * v.z));
        heroX->set(str);
        v = mainHero->orientation('y');
        wsprintf(str, "Hero y orientation %d, %d, %d", 
         (int)(100 * v.x), (int)(100 * v.y), (int)(100 * v.z));
        heroY->set(str);
        v = mainHero->orientation('z');
        wsprintf(str, "Hero z orientation %d, %d, %d", 
         (int)(100 * v.x), (int)(100 * v.y), (int)(100 * v.z));
        heroZ->set(str);
		v = mainHero->position();
        wsprintf(str, "Hero position %d, %d, %d", 
         (int)(v.x), (int)(v.y), (int)(v.z));
        heroPos->set(str);
    }

	// zoom into the background image for the scene 
	//
	backgroundTopLeftX += delta * BACKGROUND_SPEED;
	if ((int)backgroundTopLeftX > BACKGROUND_TOP_LEFT_XMAX)
		backgroundTopLeftX = 0;

    // move the hero across the terrain
	if (terrain && mainHero && (dg || df)) {
		terrain->align(mainHero, dg * dw * MOVE_SPEED, df * dw * MOVE_SPEED, cameras);
	}

	// force feedback
	//
	if (joystick->pressed(BUTTON_5))
		joystick->applyForce(0);


		ps[0]->update(delta*0.001f); //particle implementation
		ps[1]->update(delta*0.01f);
}

// drawBackground draws the background image for the scene
//
void Scene::drawBackground() {

	if (background) {
		IDeviceTexture* deviceTexture;
		deviceTexture = background->deviceTexture();
		if (deviceTexture)
			deviceTexture->draw((int)backgroundTopLeftX, 
			 (int)(backgroundTopLeftX * backgroundRatio),
			 (int)(backgroundWidth  - 2 * backgroundTopLeftX),
			 (int)((backgroundWidth - 2 * backgroundTopLeftX) * backgroundRatio));
	}
}

// drawOpaque draws the graphic representations of the objects in the
// scene that are opaque
//
void Scene::drawOpaque() {

	IGraphic* graphic;
    for (int i = 0; i < noObjects; i++) {
        graphic = object[i]->graphic();
        if (graphic && graphic->opaque())
            graphic->draw(object[i]);
    }
}

// drawTranslucent draws the graphic representations of the objects in
// the scene that are translucent
//
void Scene::drawTranslucent() {

    IGraphic* graphic;
    for (int i = 0; i < noObjects; i++) {
        graphic = object[i]->graphic();
        if (graphic && !graphic->opaque())
            graphic->draw(object[i]);
    }
}

// suspend suspends the graphical representation of each object
// and each texture
//
void Scene::suspend() {

    IGraphic* graphic;
    for (int i = 0; i < noObjects; i++) {
        graphic = object[i]->graphic();
        if (graphic) graphic->suspend(); 
    }

	IDeviceTexture* deviceTexture;
	for (int i = 0; i < noTextures; i++) {
		deviceTexture = texture[i]->deviceTexture();
		if (deviceTexture) deviceTexture->suspend();
	}
}

// restore re-initializes the time of the last update
//
bool Scene::restore(int now) {

    lastUpdate = now;

    return true;
}

// remove removes the pointer to IObject *o from the scene and
// returns true if successful, false otherwise
//
bool Scene::remove(IObject* o) {

    bool rc = false;

    for (int i = 0; i < noObjects; i++)
        if (object[i] == o) {
            object[i] = NULL;
            rc = true;
        }
    while (!object[noObjects - 1])
        noObjects--;

    return rc;
}

// remove removes the pointer to ITexture *t from the scene and
// returns true if successful, false otherwise
//
bool Scene::remove(ITexture* t) {

    bool rc = false;

    for (int i = 0; i < noTextures; i++)
        if (texture[i] == t) {
            texture[i] = NULL;
            rc = true;
        }
    while (!texture[noTextures - 1])
        noTextures--;

    return rc;
}

// destructor deletes all of the objects in the scene
//
Scene::~Scene() {

    for (int i = 0; i < noObjects; i++)
        if (object[i]) 
            object[i]->Delete();
}

//-------------------------------- Object ----------------------------------
//
// An Object is a member of the scene that can be manipulated and has a 
// graphic representation that can be drawn
//
IScene* Object::scene = NULL;

// constructor creates a 3-D object that is represented by a set of
// graphics primitives
//
Object::Object(Shape shape, float* dimension, int* partition, Colour clr, 
 bool antiAlias) {

	add(); // add this object to the scene

	vertex    = NULL;
	index     = NULL;
	attribute = NULL;
	visual    = CreateMesh(shape, dimension, partition, clr, antiAlias);
}

Object::Object(Shape shape, const char* filename, bool antiAlias) {

	add(); // add this object to the scene

	vertex    = NULL;
	index     = NULL;
	attribute = NULL;
	visual    = CreateMesh(shape, filename, antiAlias);
}

Object::Object(Shape shape, int noPrimitives, int noVertices, Colour clr, 
 ITexture* texture, bool antiAlias) : 
 nPrimitives(noPrimitives), nVertices(noVertices) {
    
	add(); // add this object to the scene

    // determine total number of indices
	switch (shape) {
        case POINT_LIST:     nIndices = nPrimitives;     break;
        case LINE_LIST:      nIndices = 2 * nPrimitives; break;
        case LINE_STRIP:     nIndices = nPrimitives + 1; break;
        case TRIANGLE_LIST:  nIndices = 3 * nPrimitives; break;
        case TRIANGLE_STRIP: nIndices = nPrimitives + 2; break;
        case TRIANGLE_FAN:   nIndices = nPrimitives + 1; break;
        default: nIndices = nPrimitives;
    }
	// allocate memory for lists
    vertex    = new Vertex[nVertices];
	index     = new short[nIndices];
	attribute = NULL;
	// temporary allocation
	IDeviceTexture* devTex = (texture ? texture->deviceTexture() : NULL);
	// create graphic representation
    visual = CreateGraphic(shape, nPrimitives, nVertices, sizeof(Vertex), 
	 nIndices, clr, devTex, antiAlias);
	// initialize counters for sequential construction of the set of
	// primitives from the constructors of the derived classes
	nVertices = 0;
	nIndices  = 0;
	nSurfaces = 0;
}

Object::Object(Shape shape, int noPrimitives, int noVertices, 
 Colour* clr, ITexture** texture, int nSubsets, bool antiAlias) : 
 nPrimitives(noPrimitives), nVertices(noVertices) {

	add(); // add this object to the scene

    // determine total number of indices
	switch (shape) {
        case POINT_LIST:     nIndices = nPrimitives;     break;
        case LINE_LIST:      nIndices = 2 * nPrimitives; break;
        case LINE_STRIP:     nIndices = nPrimitives + 1; break;
        case TRIANGLE_LIST:  nIndices = 3 * nPrimitives; break;
        case TRIANGLE_STRIP: nIndices = nPrimitives + 2; break;
        case TRIANGLE_FAN:   nIndices = nPrimitives + 1; break;
        default: nIndices = nPrimitives;
    }
	// allocate memory for lists
    vertex    = new Vertex[nVertices];
	index     = new short[nIndices];
	attribute = new unsigned[nPrimitives];
	// temporary allocation
	IDeviceTexture** devTex = new IDeviceTexture*[nSubsets];
	for (int i = 0; i < nSubsets; i++)
		if (texture && texture[i])
			devTex[i] = texture[i]->deviceTexture();
		else
			devTex[i] = NULL;
	// create graphic representation
    visual = CreateMesh(nSubsets, nPrimitives, nVertices, clr, devTex,
	 antiAlias);
	// finished with device texture addresses
	delete [] devTex;
	// initialize counters for sequential construction of the set of
	// primitives from the constructors of the derived classes
	nVertices = 0;
	nIndices  = 0;
	nSurfaces = 0;
}

// add adds the current object to the scene
//
void Object::add() {
    
    if (scene)
        scene->add(this);
    else
        error("Object::00 Can\'t access the scene object");
}

// add constructs a surface from a pair of triangle primitives by adding
// to the vertex, index and attribute lists
//
void Object::add(Vector p1, Vector p2, Vector p3, Vector p4, Vector n) {

    #if ZAXIS_DIRECTION == INTO_SCREEN
    // order triangle vertices in clockwise arrangement
    int v1, v2, v3, v4;
    // add four vertices to describe the side
    v1 = add(p1.x, p1.y, p1.z, n.x, n.y, n.z, 0, 0);
    v2 = add(p2.x, p2.y, p2.z, n.x, n.y, n.z, 0, 1);
    v3 = add(p3.x, p3.y, p3.z, n.x, n.y, n.z, 1, 1);
    v4 = add(p4.x, p4.y, p4.z, n.x, n.y, n.z, 1, 0);
    // add six indices to describe the two triangles 
    add(v1);
    add(v2);
    add(v3);
    add(v1);
    add(v3);
    add(v4);
	// associates the two triangles with the surface
	if (attribute) {
		attribute[2 * nSurfaces]     = nSurfaces;
		attribute[2 * nSurfaces + 1] = nSurfaces;
		nSurfaces++;
	}
    #elif ZAXIS_DIRECTION == OUT_OF_SCREEN
    // order triangle vertices in counter-clockwise arrangement
    add(p1.x, p1.y, p1.z, n.x, n.y, n.z, 0, 0);
    add(p3.x, p3.y, p3.z, n.x, n.y, n.z, 1, 1);
    add(p2.x, p2.y, p2.z, n.x, n.y, n.z, 0, 1);
    add(p1.x, p1.y, p1.z, n.x, n.y, n.z, 0, 0);
    add(p4.x, p4.y, p4.z, n.x, n.y, n.z, 1, 0);
    add(p3.x, p3.y, p3.z, n.x, n.y, n.z, 1, 1);
    #endif
}

// add adds a Vertex to the list of vertices for the object
//
int Object::add(float x, float y, float z, float nx, float ny, 
 float nz, float tu, float tv) {

	vertex[nVertices] = Vertex(x, y, z, nx, ny, nz, tu, tv);

	return nVertices++;
}

// local returns the vector part of vertex i
//
Vector Object::local(int i) const {

	return vertex[i].position();
}

// populateVB fills the vertex buffer at vb with vertex data
//
void Object::populateVB(void* vb) const {

	Vertex* p = (Vertex*)vb;

	for (int i = 0; i < nVertices; i++)
		*p++ = vertex[i];
}

// populateIB fills the index buffer at ib with index data
//
void Object::populateIB(void* ib) const {

	short* p = (short*)ib;

	for (int i = 0; i < nIndices; i++)
		*p++ = index[i];
}

// populateAB fills the attribute buffer at ab with index data
//
void Object::populateAB(void* ab) const {

	unsigned* p = (unsigned*)ab;

	for (int i = 0; i < nPrimitives; i++)
		*p++ = attribute[i];
}

// add constructs a surface from a pair of triangle primitives by adding
// to the vertex, index and attribute lists
//
void Object::add(Vector p1, Vector p2, Vector p3, Vector p4, Vector n,
 int tu, int tv) {

    #if ZAXIS_DIRECTION == INTO_SCREEN
    // order triangle vertices in clockwise arrangement
    int v1, v2, v3, v4;
    // add four vertices to describe the side
    v1 = add(p1.x, p1.y, p1.z, n.x, n.y, n.z,         0,         0);
    v2 = add(p2.x, p2.y, p2.z, n.x, n.y, n.z,         0, (float)tv);
    v3 = add(p3.x, p3.y, p3.z, n.x, n.y, n.z, (float)tu, (float)tv);
    v4 = add(p4.x, p4.y, p4.z, n.x, n.y, n.z, (float)tu,         0);
    // add six indices to describe the two triangles 
    add(v1);
    add(v2);
    add(v3);
    add(v1);
    add(v3);
    add(v4);
	// associates the two triangles with the surface
	if (attribute) {
		attribute[2 * nSurfaces]     = nSurfaces;
		attribute[2 * nSurfaces + 1] = nSurfaces;
		nSurfaces++;
	}
    #elif ZAXIS_DIRECTION == OUT_OF_SCREEN
    // order triangle vertices in counter-clockwise arrangement
    add(p1.x, p1.y, p1.z, n.x, n.y, n.z, 0, 0);
    add(p3.x, p3.y, p3.z, n.x, n.y, n.z, 1, 1);
    add(p2.x, p2.y, p2.z, n.x, n.y, n.z, 0, 1);
    add(p1.x, p1.y, p1.z, n.x, n.y, n.z, 0, 0);
    add(p4.x, p4.y, p4.z, n.x, n.y, n.z, 1, 0);
    add(p3.x, p3.y, p3.z, n.x, n.y, n.z, 1, 1);
    #endif
}

// add adds a vertex number to the index list
//
void Object::add(int vertex) {

	index[nIndices++] = vertex;
}

// add adds a texture to the object
//
void Object::add(ITexture* texture) {

	if (texture)
		visual->add(texture->deviceTexture());
}

Object::~Object() {

    if (visual)
        delete visual;
    if (vertex)
        delete [] vertex;
	if (index)
		delete [] index;
    if (scene)
        scene->remove(this);
    else
        error("Object::90 Can\'t access the Scene component");
}

//-------------------------------- Stock Shapes --------------------------
//
// Sphere is an Object identifiable by a radius and two tesselation 
// parameters
//
IObject* CreateSphere(float radius, int slices, int stacks, Colour clr, 
 bool antiAlias) {

	float dimension[1] = {radius};
	int   partition[2] = {slices, stacks};
	
	return new Object(SPHERE, dimension, partition, clr, antiAlias);
}

// Cylinder is an Object identifiable by an upper radius, a lower radius
// a height and two tesselation parameters 
//
IObject* CreateCylinder(float upper, float lower, float height, 
 int slices, int stacks, Colour clr, bool antiAlias) {
	
	float dimension[3] = {upper, lower, height};
	int   partition[2] = {slices, stacks};

	return new Object(CYLINDER, dimension, partition, clr, antiAlias);
}

// Torus is an Object identifiable by an inner radius, an outer radius
// and two tesselation parameters 
//
IObject* CreateTorus(float inner, float outer, int slices, int stacks,
 Colour clr, bool antiAlias) {
	
	float dimension[2] = {inner, outer};
	int   partition[2] = {slices, stacks};

	return new Object(TORUS, dimension, partition, clr, antiAlias);
}

// Teapot is an Object that represents the Utah Teapot 
//
IObject* CreateTeapot(Colour clr, bool antiAlias) {

	return new Object(TEAPOT, NULL, NULL, clr, antiAlias);
}

// XFile is an Object that is described in X-File format 
//
IObject* CreateXFile(const char* filename, bool antiAlias) {

	return new Object(X_FILE, filename, antiAlias);
}

//-------------------------------- Box -------------------------------------
//
// Box is an Object identifiable by two points - lower close left, and
// upper faraway right - and a colour
//
IObject* CreateBox(float minx, float miny, float minz, float maxx, 
 float maxy, float maxz, Colour c, ITexture* texture, int tu, int tv, 
 bool antiAlias) {

	return new Box(minx, miny, minz, maxx, maxy, maxz, c, texture, tu, 
	 tv, antiAlias);
}

IObject* CreateBox(float minx, float miny, float minz, float maxx, 
 float maxy, float maxz, Colour* c, ITexture** texture, int tu, int tv, 
 bool antiAlias) {

	return new Box(minx, miny, minz, maxx, maxy, maxz, c, texture, tu,
	 tv, antiAlias);
}

// constructor builds the Box from its two extreme points
//
Box::Box(float minx, float miny, float minz, float maxx, float maxy,
 float maxz, Colour c, ITexture* texture, int tu, int tv, bool antiAlias)
 : Object(TRIANGLE_LIST, 12, 24, c, texture, antiAlias) {

    Vector p1 = Vector(minx, miny, ZAXIS_DIRECTION * minz),
           p2 = Vector(minx, maxy, ZAXIS_DIRECTION * minz),
           p3 = Vector(maxx, maxy, ZAXIS_DIRECTION * minz),
           p4 = Vector(maxx, miny, ZAXIS_DIRECTION * minz),
           p5 = Vector(minx, miny, ZAXIS_DIRECTION * maxz),
           p6 = Vector(minx, maxy, ZAXIS_DIRECTION * maxz),
           p7 = Vector(maxx, maxy, ZAXIS_DIRECTION * maxz),
           p8 = Vector(maxx, miny, ZAXIS_DIRECTION * maxz);
    add(p1, p2, p3, p4, Vector(0, 0, ZAXIS_DIRECTION * -1), tu, tv); // front
    add(p4, p3, p7, p8, Vector(1, 0,                    0), tu, tv); // right
    add(p8, p7, p6, p5, Vector(0, 0, ZAXIS_DIRECTION *  1), tu, tv); // back
    add(p6, p2, p1, p5, Vector(-1, 0,                   0), tu, tv); // left
    add(p1, p4, p8, p5, Vector(0, -1,                   0), tu, tv); // bottom
    add(p2, p6, p7, p3, Vector(0, 1,                    0), tu, tv); // top
}

// constructor builds the Box from its two extreme points
//
Box::Box(float minx, float miny, float minz, float maxx, float maxy,
 float maxz, Colour* c, ITexture** texture, int tu, int tv, 
 bool antiAlias) : Object(TRIANGLE_LIST, 12, 24, c, texture, 6, 
 antiAlias) {

    Vector p1 = Vector(minx, miny, ZAXIS_DIRECTION * minz),
           p2 = Vector(minx, maxy, ZAXIS_DIRECTION * minz),
           p3 = Vector(maxx, maxy, ZAXIS_DIRECTION * minz),
           p4 = Vector(maxx, miny, ZAXIS_DIRECTION * minz),
           p5 = Vector(minx, miny, ZAXIS_DIRECTION * maxz),
           p6 = Vector(minx, maxy, ZAXIS_DIRECTION * maxz),
           p7 = Vector(maxx, maxy, ZAXIS_DIRECTION * maxz),
           p8 = Vector(maxx, miny, ZAXIS_DIRECTION * maxz);
    add(p1, p2, p3, p4, Vector(0, 0, ZAXIS_DIRECTION * -1), tu, tv); // front
    add(p4, p3, p7, p8, Vector(1, 0,                    0), tu, tv); // right
    add(p8, p7, p6, p5, Vector(0, 0, ZAXIS_DIRECTION *  1), tu, tv); // back
    add(p6, p2, p1, p5, Vector(-1, 0,                   0), tu, tv); // left
    add(p1, p4, p8, p5, Vector(0, -1,                   0), tu, tv); // bottom
    add(p2, p6, p7, p3, Vector(0, 1,                    0), tu, tv); // top
}

//-------------------------------------- Grid ------------------------------
//
// A Grid is a set of mutually perpendicular lines in a plane defined 
// parallel to the x-y plane
//
IObject* CreateGrid(int min, int y, int max, int n, Colour clr, 
 bool antiAlias) {

	return new Grid(min, y, max, n, clr, antiAlias);
}

Grid::Grid(int min, int y, int max, int n, Colour clr, bool antiAlias) : 
 Object(LINE_LIST, 2 * n, 4 * n, clr, NULL, antiAlias) {

    int i, inc = (max - min) / (n - 1);
    for (i = min; i <= max; i += inc) {
        add(add((float)min, (float)y, (float)i, 0, 1, 0));
        add(add((float)max, (float)y, (float)i, 0, 1, 0));
        add(add((float)i, (float)y, (float)min, 0, 1, 0));
        add(add((float)i, (float)y, (float)max, 0, 1, 0));
    }
}

//-------------------------------- Terrain ---------------------------------
//
// A Terrain is an Object that consists of a grid of triangles where the
// height of each vertex is specified using a height map
//
IObject* CreateTerrain(int cellSpacing, float depth, const char* heightMap, 
 ITexture* tFile) {

    // open the height bitmap file
    std::ifstream fp(heightMap, std::ios::in | std::ios::binary);

    // garbage info
    unsigned char  cjunk;
    unsigned short sjunk;
    unsigned int   djunk;

    // useful info
    unsigned int   width;     // width of the bitmap
    unsigned int   height;    // height of the bitmap
    unsigned short bitdepth;  // bit depth 1, 4, 8, 16, 24, 32
    unsigned int   offset;    // offset to bitmap data in bytes

    // read the file header
    fp.read((char*)&(sjunk),  sizeof(sjunk));
    fp.read((char*)&(djunk),  sizeof(djunk));
    fp.read((char*)&(sjunk),  sizeof(sjunk));
    fp.read((char*)&(sjunk),  sizeof(sjunk));
    fp.read((char*)&(offset), sizeof(offset));

    // read the info header
    fp.read((char*)&(djunk),    sizeof(djunk));
    fp.read((char*)&(width),    sizeof(djunk));
    fp.read((char*)&(height),   sizeof(djunk));
    fp.read((char*)&(sjunk),    sizeof(sjunk));
    fp.read((char*)&(bitdepth), sizeof(sjunk));
    fp.read((char*)&(djunk),    sizeof(djunk));
    fp.read((char*)&(djunk),    sizeof(djunk));
    fp.read((char*)&(djunk),    sizeof(djunk));
    fp.read((char*)&(djunk),    sizeof(djunk));
    fp.read((char*)&(djunk),    sizeof(djunk));
    fp.read((char*)&(djunk),    sizeof(djunk));

    // move to start of image - skip bytes if offset > 54
    for (int i = 54; i < (int)offset; i++)
        fp.read((char*)&(cjunk), sizeof(cjunk));

    return new Terrain(fp, width, height, bitdepth, cellSpacing, depth, 
	 tFile);
}

Terrain::Terrain(std::ifstream& fp, unsigned width, unsigned height, 
 unsigned bitdepth, int cellSpacing, float depth, ITexture* tFile) : 
 Object(TRIANGLE_LIST, 2 * (width - 1) * (height - 1), 
 4 * (width - 1) * (height - 1), Colour(1, 1, 1, 1), tFile, true), 
 cols(width), rows(height), spacing(cellSpacing) {

    // allocate space for reading one full scan line
    unsigned depthInBytes = (unsigned) bitdepth / TERRAIN_CHAR_BIT;
    unsigned bufferSize   = (unsigned)((width * bitdepth)
     / float(TERRAIN_CHAR_BIT));
    unsigned char* buffer = new (std::nothrow) unsigned char[bufferSize];

    // compute the vertices from (rows, cols, spacing)
	// and add each vertex to the list of vertices
    float x, y, z = - 0.5f * (rows - 1) * spacing;
    float uInc = 1.0f / (cols - 1), vInc = 1.0f / (rows - 1), u, v = 1.0f;
    for (int i = 0; i < rows; i++) {
        u = 0;
        x = - 0.5f * (cols - 1) * spacing;
        fp.read((char*)buffer, bufferSize);
        for (int j = 0; j < cols; j++) {
            y = depth * greyScale(buffer + depthInBytes * j, depthInBytes)
             - depth * 0.5f;
            // add vertex
			add(x, y, z, 0, 1, 0, u, v);
            x += spacing;
            u += uInc;
        }
        z += spacing;
        v -= vInc;
    }
    // de-allocate space
    delete [] buffer;

    // Compute the indices
    int k = 0;
    for (int i = 0; i < rows - 1; i++, k++) {
        for (int j = 0; j < cols - 1; j++, k++) {
            add(k);
            add(k + cols);
            add(k + cols + 1);
            add(k);
            add(k + cols + 1);
            add(k + 1);
        }
    }

		// define the bounding box for the terrain
	float minx = 0.5f * (cols - 1) * spacing;
	float miny = 0.5f * depth;
	float minz = 0.5f * (rows - 1) * spacing;

	//particle implementation

	setBoundingBox(-minx, -miny, -minz, minx, miny, minz);
}

// greyScale returns the grey scale equivalent of the data at pixel
// in the range [0.0f, 1.0f]
//
float Terrain::greyScale(void* pixel, unsigned int depthInBytes) {

    int grey = HEIGHT_MAP_FLOOR * 100;
    unsigned char* buffer = (unsigned char*) pixel;

    switch (depthInBytes) {
        case 1u:
			error("Terrain 20:: 8 bit height mapping is not implemented");
            break;
        case 2u:
			error("Terrain 21:: 16 bit height mapping is not implemented");
            break;
        case 3u:
            // Pixel data stored in reverse order on file
            struct {
                unsigned char blue;
                unsigned char green;
                unsigned char red;
            } p;
            memcpy((char*)&p, buffer, 3);
            // there is no unique formula for converting from
            // rgb to grayscale - a common one is
            // .30 * r + .59 * g + .11 * b
            // here, we do not compensate for g predominance
            // but use even weighting
            grey = 33 * p.red + 33 * p.green + 33 * p.blue;
            if (grey < HEIGHT_MAP_FLOOR * 100)
               grey = HEIGHT_MAP_FLOOR * 100;
            break;
        case 4u:
			error("Terrain 22:: 32 bit height mapping is not implemented");
            break;
    }

    return grey / 25500.f; // 255 * 100 => value in range [0.0f, 1.0f]
}

// align aligns the traversing object with the terrain by retrieving
// the elevation of the terrain corresponding to the new position of
// the object where delx and delz define its movements in the world x
// and z directions respectively, by orienting the object in the
// direction of its movement with respect to the world y axis, and
// by inclining the object so that it moves along the face of its
// host triangle within the terrain.
// This function does not alter the position or orientation of the
// object if the values of the movement parameters place the object
// at or beyond the boundary of the terrain
//
void Terrain::align(IObject* object, float delx, float delz, ICameras* camera) const {
	float tempz=delz * 3;
	float tempx=delx * 3;

    Frame* frame = (Frame*)object;
	float dx, dz, dely, slope, mPath, mTrrn, denom;
    Vector n, heading(0), centre = position();

    // retrieve the position of the traversing object
    Vector position = object->position();
	// update the x and z componwents of the position
	// the y component is determined below
    position.x += delx;
    position.z += delz;


    // determine row, column indices corresponding to the new position
    int i = int((position.x - centre.x) / spacing + (cols - 1) / 2.0f); //물체의 위치에서 현재 테레인의 위치를 빼서 스페이싱 간격으로 나눈다.
    int j = int((position.z - centre.z) / spacing + (rows - 1) / 2.0f);

    // ensure that traversing object is within the terrain boundaries
    if (i >= BORDER && i < cols - BORDER && j >= BORDER && j < rows -
		BORDER) {
        // determine the world coordinates of the
        // corners of the quad that contains (x,z)
        Vector aa = local(cols * j + i) * world();
        Vector ab = local(cols * (j + 1) + i) * world();
        Vector bb = local(cols * (j + 1) + i + 1) * world();
        Vector ba = local(cols * j + i + 1) * world();

        // determine the quad triangle that contains the current position
        if (abs(bb.x - aa.x) > TINY_VALUE && position.z > (bb.z - aa.z)
         * (position.x - aa.x) / (bb.x - aa.x) + aa.z || abs(bb.x - aa.x)
		 <= TINY_VALUE && position.x < aa.x && bb.z > aa.z) {
            // x, z lies within the triangle aa-ab-bb
            //
            // determine interpolation factors with respect to ab
            dx = (position.x - ab.x) / (bb.x - ab.x);
            dz = (position.z - ab.z) / (aa.z - ab.z);
            // interpolate to find the elevation of the updated position
            dely = ab.y + dx * (bb.y - ab.y) + dz * (aa.y - ab.y) +
			 CLEARANCE - position.y;
            //
            // determine normal to the plane of the host triangle
            n = - normal(cross(bb - ab, ab - aa));
            // find inclination axis in the plane and angle of inclination
            if (abs(delx) > TINY_VALUE || abs(delz) > TINY_VALUE) {
                // find point xp, yp, zp along path of traversing object
                if (abs(delx) > TINY_VALUE && abs(delz) > TINY_VALUE) {
                    // find intersection of the path with ab-bb
                    mPath = delz / delx;
                    mTrrn = (bb.z - ab.z) / (bb.x - ab.x);
                    denom = mPath - mTrrn;
                    slope = (bb.y - ab.y) / (bb.x - ab.x);
                    heading.x = (ab.z - aa.z - mTrrn * (ab.x - aa.x)) / denom;
                    heading.y = slope * (heading.x - ab.x + aa.x) + ab.y - aa.y;
                    heading.z = mPath * heading.x;
                }
                else if (abs(delx) < TINY_VALUE) {
                    // path is perpendicular to ab-bb
                    heading.x = ab.x - aa.x;
                    heading.y = ab.y - aa.y;
                    heading.z = ab.z - aa.z;
                }
                else {
                    // path is parallel to ab-bb
                    heading.x = bb.x - ab.x;
                    heading.y = bb.y - ab.y;
                    heading.z = bb.z - ab.z;
                }
            }
        }
        else {
            // x,z lies within the triangle aa-bb-ba
            //
            // determine interpolation factors with respect to ba
            dx = (position.x - ba.x) / (aa.x - ba.x);
            dz = (position.z - ba.z) / (bb.z - ba.z);
            // interpolate to find new elevation of the updated position
            dely = ba.y + dx * (aa.y - ba.y) + dz * (bb.y - ba.y) + 
			 CLEARANCE - position.y;
            //
            // determine normal to the plane of the host triangle
            n = - normal(cross(ba - aa, bb - ba));
            // find inclination axis in the plane and angle of inclination
            if (abs(delx) > TINY_VALUE || abs(delz) > TINY_VALUE) {
                // find point xp, zp along path of traversing object
                if (abs(delx) > TINY_VALUE && abs(delz) > TINY_VALUE) {
                    // find intersection of path with aa-ba
                    mPath = delz / delx;
                    mTrrn = (ba.z - aa.z) / (ba.x - aa.x);
                    denom = mPath - mTrrn;
                    slope = (ba.y - aa.y) / (ba.x - aa.x);
                    heading.x = (bb.z - ba.z - mTrrn * (bb.x - ba.x)) / denom;
                    heading.y = slope * (heading.x - bb.x + ba.x) + bb.y - ba.y;
                    heading.z = mPath * heading.x;
               }
                else if (abs(delx) < TINY_VALUE) {
                    // path is perpendicular to aa-ba
                    heading.x = bb.x - ba.x;
                    heading.y = bb.y - ba.y;
                    heading.z = bb.z - ba.z;
                }
                else {
                    // path is parallel to aa-ba
                    heading.x = ba.x - aa.x;
                    heading.y = ba.y - aa.y;
                    heading.z = ba.z - aa.z;
				}
            }
		}

        // move the traversing object to its new position
		
	//	Vector carHead = normal(camera->heading());

	//	delx *= carHead.z;
//		////dely *= carHead.y;
	//	delz *= carHead.z;
		
		//Vector head, up, right;

		//	
		//	head = normal(camera->heading()); 
		//	up = normal(camera->top());     
		//	right = cross(up, head);

		//	delz *= camera->heading().z;
		//	delx = right.x

		

//		object->move(delx, dely, delz);


		// restore previous orientation
		//Matrix before = frame->restore();
		//// determine instantaneous orientation of traversing
		//// object with resepct to its supporting triangle and
		//// average previous and current to suppress choppiness
		//n = normal(Vector(before.m21, before.m22, before.m23) + n);
		//Vector z = normal(Vector(before.m31, before.m32, before.m33) + heading);
		//Vector x = cross(n, z);

		//

		//Matrix orientation(x.x, x.y, x.z, 0,
		//	               n.x, n.y, n.z, 0,
		//				   z.x, z.y, z.z, 0,
		//				     0,   0,   0, 1);
		//// orient the traversing object with 
		//// respect to its supporting triangle
		//frame->orient(orientation);
		//frame->save(orientation);
		

		Vector h, u, r;

			
			h = -normal(camera->heading()); //to make look at opposite of my camera //basically this object looks at me, so I reverse it

			
			u = normal(camera->top());     
			r = cross(u, h);

		//Matrix rot(r.x, r.y, r.z, 0, 
		//       u.x, u.y, u.z, 0, 
		//	   h.x, h.y, h.z, 0, 
		//	     0,   0,   0, 1);
	
	/*		Matrix rot(0, 0, 0, 0, 
		       0, 0, 0, 0, 
			   0, 0, 0, 0, 
			     0,   0,   0, 1);*/

//   vecHead.x,  dely,  vecHead.z, 1);
//			Vector vecHead = camera->heading();		
//			vecHead *= 3;

			Vector vecHeads = normal(camera->heading());
			Vector vecTop = normal(camera->top());
			Vector vecRight = cross(vecTop, vecHeads);
			
			vecHeads *= tempz; //delx, delz로 하면 엄청 느림
			vecRight *= tempx;
		/*
		Matrix rot(r.x, r.y, r.z, 0, 
		       u.x, u.y, u.z, 0, 
		   h.x, h.y, h.z, 0, 
		   vecHeads.x + vecRight.x,  0,  vecHeads.z + vecRight.z, 1);
	*/

	object->move(0, dely, 0);

	Matrix rot(r.x, r.y, r.z, 0, 
		       u.x, u.y, u.z, 0, 
		   h.x, h.y, h.z, 0, 
		   vecHeads.x + vecRight.x,  0,  vecHeads.z + vecRight.z, 1);



		frame->orient(rot); 
		frame->save(rot);
		
	}
}

//-------------------------------- Billboard ------------------------------
//
// Billboard is a two dimensional rectangle that always faces the current
// camera 
//
IObject* CreateBillboard(Orientation orient, ICameras* cs, float minx, 
 float miny, float maxx, float maxy, Colour c, ITexture* texture) {
	
	return new Billboard(orient, cs, minx, miny, maxx, maxy, c, texture);
}

Billboard::Billboard(Orientation orient, ICameras* cs, float minx, 
 float miny, float maxx, float maxy, Colour c, ITexture* texture) : 
 cameras(cs), type(orient),
 Object(TRIANGLE_LIST, 2, 4, c, texture, false) {

//Object::Object(Shape shape, int noPrimitives, int noVertices, Colour clr, 
// ITexture* texture, bool antiAlias) : 

    Vector p1 = Vector(minx, miny, 0),
           p2 = Vector(minx, maxy, 0),
           p3 = Vector(maxx, maxy, 0),
           p4 = Vector(maxx, miny, 0);

    add(p1, p2, p3, p4, Vector(0, 0, ZAXIS_DIRECTION * -1)); 
}

// orient orients the billboard so that its normal is parallel to the
// direction from the billboard to the camera
//
void Billboard::orient() {

	Vector h, u, r, p = position();
	switch (type) {
		case SCREEN:
			h = normal(cameras->heading()); 
			u = normal(cameras->top());     
			r = cross(u, h);
			break;
		case VIEW_PLANE:
			h = normal(cameras->heading()); // fixed
			u = Vector(0, 1, 0);
			r = cross(u, h);
			u = cross(h, r);
			break;
		case VIEWPOINT:
			h = normal(p - cameras->position()); // fixed
			u = Vector(0, -1, 0);
			r = cross(u, h);
			u = cross(h, r);
			break;
		case AXIAL:
			h = normal(p - cameras->position());
			u = Vector(0, 1, 0); // fixed
			r = cross(u, h);
			h = cross(h, r);
			break;
	}
	Matrix rot(r.x, r.y, r.z, 0, 
		       u.x, u.y, u.z, 0, 
			   h.x, h.y, h.z, 0, 
			     0,   0,   0, 1);
	Frame::orient(rot); 
}

//-------------------------------- Vertex ---------------------------------
//
// Vertex holds all of the data that describes a vertex
//
Vertex::Vertex(float xx, float yy, float zz, float nxx, float nyy,
 float nzz, float ttu, float ttv) {

    x = xx;
    y = yy;
    z = zz;
    nx = nxx;
    ny = nyy;
    nz = nzz;
    tu = ttu;
    tv = ttv;
}

// position returns the vector that defines the position of the vertex
//
Vector Vertex::position() const {

	return Vector(x, y, z);
}

//-------------------------------- Texture -------------------------------
//
// Texture manages a texture for a set of objects
//
ITexture* CreateTexture(const char* file, unsigned flags) { 

	return new Texture(file, flags, Colour());
}

ITexture* CreateTexture(const char* file, unsigned flags, Colour brdrClr) { 

	return new Texture(file, flags, brdrClr);
}

IScene* Texture::scene = NULL;

// constructor adds a pointer to the texture to the scene and
// creates the graphics card representation of the texture
//
Texture::Texture(const char* file, unsigned flags, Colour brdrClr) {

	if (scene)
		scene->add(this);
	else
		error("Texture 00: Couldn\'t access the scene object");

	deviceTexture_ = CreateDeviceTexture(file, flags, brdrClr);
}

// destructor deletes the graphics card representation and removes the pointer to the
// texture from the scene
//
Texture::~Texture() {

	deviceTexture_->Delete();
	if (scene)
		scene->remove(this);
	else
		error("Texture 90:: Couldn\'t access the scene object");
}



