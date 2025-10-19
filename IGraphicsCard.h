#ifndef _I_GRAPHICS_CARD_H_
#define _I_GRAPHICS_CARD_H_

/* Interface to the GraphicsCard Module
 *
 * consists of IHost interface
 *             IDisplay interface
 *             IDeviceLight interface
 *             IGraphic interface
 *             IDeviceTexture interface
 *             IFont interface
 *
 * IGraphicsCard.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

//-------------------------------- IHost ---------------------------------
//
// IHost is the interface to the graphics card for initial interrogation
//
class IHost {
  public:
    virtual int  adapterCount() const                                 = 0;
    virtual int  displayDescription(int id, char* desc, int maxDesc) 
	 const                                                            = 0;
    virtual int  formatCount()                                        = 0;
    virtual int  modeCount(int id, int k) const                       = 0;
    virtual bool getMode(int id, int ir, int k, unsigned& fmMd, 
	 char* line)                                                      = 0;
	virtual void configure(unsigned fmMd, int& id, int& ir, int& format, 
	 int& width, int& height, int& wndStyle, int& wndExStyle)         = 0;
	virtual void Delete()                                             = 0;
};

extern "C"
IHost* CreateHost(void* hDbWnd);

//-------------------------------- IDisplay ------------------------------
//
// IDisplay is the interface to the representation of the display device
// on the graphics card
//
class IScene;
class ILighting;
class IHUD;
class ICameras;
struct Vector;

class IDisplay {
  public:
    virtual bool setup()                                           = 0;
    virtual void wndResize()                                       = 0;
    virtual void draw(const Vector&, const Vector&, const Vector&) = 0;
    virtual void suspend()                                         = 0;
    virtual bool restore()                                         = 0;
    virtual void release()                                         = 0;
	virtual void Delete()                                          = 0;
};

extern "C"
IDisplay* CreateDisplay(IScene* s, ILighting* l, IHUD* h, ICameras* ca);

//-------------------------------- IDeviceLight --------------------------
//
// IDeviceLight is the interface to the representation of a single light
// source on the graphics card
//
class ILight;

class IDeviceLight {
  public:
    virtual void setup(int index)  = 0;
    virtual void update()          = 0;
};

extern "C"
IDeviceLight* CreateDeviceLight(ILight* light);

//-------------------------------- IGraphic ------------------------------
//
// IGraphic is the interface to the representation of a single object on
// the graphics card
//
class IObject;
struct Colour;
class IDeviceTexture;

typedef enum Shape {
    POINT_LIST     = 1,
    LINE_LIST      = 2,
    LINE_STRIP     = 3,
    TRIANGLE_LIST  = 4,
    TRIANGLE_STRIP = 5,
    TRIANGLE_FAN   = 6,
	SPHERE,
	CYLINDER,
	TORUS,
	TEAPOT,
	X_FILE,
	CUSTOM
} Shape;

class IGraphic {
  public:
    virtual void setup(const IObject* object)       = 0;
    virtual bool opaque() const                     = 0;
	virtual void add(IDeviceTexture* deviceTexture) = 0;
    virtual void draw(const IObject* object)        = 0;
    virtual void suspend()                          = 0;
	virtual void Delete()                           = 0;
};

extern "C"
IGraphic* CreateGraphic(Shape pType, int noPrimitives, 
 int noVertices, int vSize, int noIndices, Colour clr, 
 IDeviceTexture* devTex, bool antiAlias);

IGraphic* CreateMesh(int noSubsets, int noPrimitives, int noVertices, 
 Colour* clr, IDeviceTexture** devTex, bool antiAlias);

IGraphic* CreateMesh(Shape shape, float* d, int* p, Colour clr, 
 bool antiAlias);

IGraphic* CreateMesh(Shape shape, const char* filename, bool antiAlias);

//-------------------------------- IDeviceTexture ------------------------
//
// IDeviceTexture is the interface to the representation of a texture on 
// the graphics card
//
class IDeviceTexture {
public:
	virtual void setup()                                            = 0;
	virtual void attach(int i)                                      = 0;
	virtual void detach(int i)                                      = 0;
	virtual void draw(int topLeftX, int topLeftY, int visibleWidth,
	 int visibleHeight)                                             = 0;
	virtual void suspend()                                          = 0;
	virtual void Delete()                                           = 0;
};

extern "C"
IDeviceTexture* CreateDeviceTexture(const char* file, unsigned flags,
 Colour brdrClr);

//-------------------------------- IFont ---------------------------------
//
// IFont is the interface to the representation of a text item on the
// graphics card
//
class IText;

class IFont {
  public:
    virtual void draw(const char* text) = 0;
    virtual void suspend()              = 0;
    virtual bool restore()              = 0;
	virtual void release()              = 0;
	virtual void Delete()               = 0;
};

extern "C"
IFont* CreateFont_(IText* text, unsigned int flags);

#endif