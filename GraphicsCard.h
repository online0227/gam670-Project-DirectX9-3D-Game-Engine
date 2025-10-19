#ifndef _GRAPHICSCARD_H_
#define _GRAPHICSCARD_H_

/* Header for the GraphicsCard Module
 *
 * consists of Host declaration
 *             Display declaration
 *             DeviceLight declaration
 *             Graphic declaration
 *             DeviceTexture declaration
 *             Font declaration
 *
 * GraphicsCard.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "DeviceSettings.h" // for GRAPHICS_API
#include "Particle.h"
//#include "Utilities.h"

#if GRAPHICS_API == DIRECT3D
#include <d3d9.h>      // for basic D3D
#include <d3dx9.h>     // for D3D utilities
#include <d3dx9core.h> // for LPD3DXFONT
#elif GRAPHICS_API == OPENGL
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <gl/gl.h>  // for basic OpenGL   
#include <gl/glu.h> // for OpenGL utilities
#define MFORMAT 300 // maximum number of pixel formats
#endif

#include "IGraphicsCard.h"
#include "IScene.h"

//-------------------------------- Host ---------------------------------
//
// Host provides access to the graphics card for capability interrogation
//
class Host : public IHost {

	HWND hwnd;

    #if GRAPHICS_API == DIRECT3D
    LPDIRECT3D9 d3d;            // interface to the Direct3D object
    D3DFORMAT format;           // pixel format identifier

    #elif GRAPHICS_API == OPENGL
    HDC  hdc;                   // Windows device context
    int  format;                // pixel format identifier
    int  pixelFmt[MFORMAT];     // list of suitable pixel formats
    #endif

	Host(void* hDbWnd);
	Host(const Host&);
	Host& operator=(const Host&);
    virtual ~Host();

  public:
	friend IHost* CreateHost(void* hDbWnd);
    int    adapterCount() const;
    int    displayDescription(int id, char* desc, int maxDesc) const;
    int    formatCount();
    int    modeCount(int id, int k) const;
    bool   getMode(int id, int ir, int k, unsigned& fmMd, char* line);
	void   configure(unsigned fmMd, int& id, int& ir, int& format, 
	 int& width, int& height, int& wndStyle, int& wndExStyle);
	void   Delete() { delete this; }
};

//-------------------------------- Display ------------------------------
//
// Display manages the display device on the graphics card
//
class Display : public IDisplay {

	static const int MAX_LIGHTS = 8;

    HWND       hwnd;     // points to the main application window
    IScene*    scene;    // points to the scene interface
    ILighting* lighting; // points to the lighting interface
    IHUD*      hud;      // points to the heads up display interface
	ICameras* cameras; //particle implementation
	ParticleSystem* psGun; //particle implementation // for laser pointer
	ParticleSystem* psSnow;
	BoundingBox boundingBox;
	IObject* terrain;



    int display;          // display adapter identifier
    int mode;             // resolution mode identifier
    int pixel;            // pixel format identifier
    int width;            // width of the client area
    int height;           // height of the client area
    int titleBar;         // approximate height of the title bar
    int maxLights;        // max no of lights supported by graphics card

    #if GRAPHICS_API == DIRECT3D
    LPDIRECT3D9 d3d;             // interface to the Direct3D object
    LPDIRECT3DDEVICE9 d3dd;      // interface to Direct3D display device
    D3DPRESENT_PARAMETERS d3dpp; // parameters for creating/restoring D3D
                                 // display device
    LPD3DXSPRITE sprite;         // points to the sprite COM object
    LPDIRECT3DTEXTURE9 hud_tex;  // points to the hud texture
	unsigned fvf;                // holds the flexible vertex format

    #elif GRAPHICS_API == OPENGL
    HDC hdc;                     // Windows device context
    HGLRC hrc;                   // OpenGL rendering context
    #endif

    void setupProjection();      // sets up the projection matrix
    void setupLighting();        // sets up the lighting
	void setGlobalState();        // sets up the alpha blending

    Display(IScene* s, ILighting* l, IHUD* h, ICameras* ca);
	Display(const Display& d);            // prevents copying
	Display& operator=(const Display& d); // prevents assignment
    virtual ~Display();

  public:
	friend IDisplay* CreateDisplay(IScene* s, ILighting* l, IHUD* h, ICameras* ca);
    bool   setup();
    void   wndResize();
    void   draw(const Vector& p, const Vector& h, const Vector& u);
    void   suspend();
    bool   restore();
    void   release();
	void   Delete() { delete this; }
    friend class Font;
	friend class Graphic;
};

//-------------------------------- DeviceLight ---------------------------
//
// DeviceLight manages a light source as stored on the graphics card
//
class ILight;

class DeviceLight : public IDeviceLight {

    #if GRAPHICS_API == DIRECT3D
    static LPDIRECT3DDEVICE9 d3dd; // Direct3D display device interface
    #elif GRAPHICS_API == OPENGL
    #endif
	
	ILight* light;                 // points to light source interface
	int index;                     // identifier on graphics card

	DeviceLight(ILight* light);

  public:
	friend IDeviceLight* CreateDeviceLight(ILight* light);
	void   setup(int index);
	void   update();
	friend class Display;
};

//-------------------------------- Graphic -------------------------------
//
// Graphic manages the graphics primitives that represent an object on the 
// graphics card
//
class IObject;

class Graphic : public IGraphic {

    bool isOpaque;                  // is perfectly opaque?
	bool antiAliasingOn;            // anti-aliasing is on?
	int  nVertices;                 // number of vertices
	int  nTextures;                 // number of texture levels
	int  vertexSize;                // size of a single vertex
	int  vertexBufferSize;          // size of the vertex buffer
	int  indexBufferSize;           // size of the index buffer
	IDeviceTexture** deviceTexture; // points to the deviceTexture object

    #if GRAPHICS_API == DIRECT3D
    static LPDIRECT3DDEVICE9 d3dd; // Direct3D display device
	static unsigned fvf;           // flexible vertex format
    int nPrimitives;               // number of primitives
    D3DPRIMITIVETYPE type;         // primitive type
    D3DMATERIAL9 mat;              // material reflectivity
    LPDIRECT3DVERTEXBUFFER9 vb;    // vertex buffer (holds the vertices)
    LPDIRECT3DINDEXBUFFER9 ib;     // index buffer (points to vertices)

    #elif GRAPHICS_API == OPENGL
    int nVertices;        // number of vertices
	GLfloat (*vb)[3];     // vertex array
    GLfloat (*nv)[3];     // normal array
    GLfloat (*tc)[2];     // texture array
    GLenum type;          // primitive type
    unsigned int tex;     // texture spread over object's surface
    GLfloat red;          // colour ...
    GLfloat green;        //  ...
    GLfloat blue;         //   ...
    GLfloat alpha;        //    ...
    int power;            // shininess
    #endif

    Graphic(Shape pType, int noPrimitives, int noVertices, 
	 int vSize, int noIndices, Colour clr, IDeviceTexture* devTex, 
	 bool antiAlias);
    Graphic(const Graphic& v);            // prevents copying
    Graphic& operator=(const Graphic& v); // prevents assignment
    virtual ~Graphic();

	void setup(const IObject* object);

  public:
	friend IGraphic* CreateGraphic(Shape pType, int noPrimitives, 
	 int noVertices, int vSize, int noIndices, Colour clr, 
	 IDeviceTexture* devTex, bool antiAlias);
    bool   opaque() const { return isOpaque; }
	void   add(IDeviceTexture* deviceTexture);
    void   draw(const IObject* object);
    void   suspend();
	void   Delete() { delete this; }
    friend class Display;
};

//-------------------------------- Mesh ----------------------------------
//
// Mesh manages the graphics primitives that represent an object on the 
// graphics card in the form of a mesh
//
class IObject;

class Mesh : public IGraphic {

    bool isOpaque;                 // is perfectly opaque?
	bool antiAliasingOn;           // anti-aliasing is on?
    int  nSubsets;                 // number of subsets in the mesh
	int  nTextures;                // number of texture levels
    int  nPrimitives;              // number of primitives in the mesh
	int  nVertices;                // number of vertices in the mesh
	IDeviceTexture** tex;          // points to deviceTexture addresses
	Shape shape;                   // the type of shape being represented
	float dimension[3];            // dimensions of stock shapes
	int partition[2];              // tesselation parameters for stock shapes
	const char* file;              // file containing mesh data

    static LPDIRECT3DDEVICE9 d3dd; // Direct3D display device
	static unsigned fvf;           // flexible vertex format
    LPD3DXMESH mesh;               // set of vertices, indices, attributes
    D3DMATERIAL9* mat;             // material reflectivity for each subset

    Mesh(int noSubsets, int noPrimitives, int noVertices, Colour* clr, 
	 IDeviceTexture** devTex, bool antiAlias);
	Mesh(Shape shape, float* d, int* p, Colour clr, bool antiAlias);
	Mesh(Shape shape, const char* filename, bool antiAlias);
    Mesh(const Mesh& v);            // prevents copying
    Mesh& operator=(const Mesh& v); // prevents assignment
    virtual ~Mesh();

	void setup(const IObject* object);

  public:
	friend IGraphic* CreateMesh(int noSubsets, int noPrimitives, 
	 int noVertices, Colour* clr, IDeviceTexture** devTex, 
	 bool antiAlias);
	friend IGraphic* CreateMesh(Shape shape, float* d, int* p, 
	 Colour clr, bool antiAlias);
	friend IGraphic* CreateMesh(Shape shape, const char* filename, 
	 bool antiAlias);
    bool   opaque() const { return isOpaque; }
	void   add(IDeviceTexture* deviceTexture);
    void   draw(const IObject* object);
    void   suspend();
	void   Delete() { delete this; }
    friend class Display;
};

//-------------------------------- DeviceTexture -------------------------
//
// DeviceTexture manages a texture as stored on the graphics card
//
class DeviceTexture : public IDeviceTexture {

	static int width;     // width of the client area
	static int height;    // height of the client area
	static int maxStages; // maximum number of texture stages

	unsigned filter;      // sample filtering flags
	unsigned borderColor; // border colour
	char* filename;       // points to the background image file

	#if GRAPHICS_API == DIRECT3D
    static LPDIRECT3DDEVICE9 d3dd; // Direct3D display device
	static LPD3DXSPRITE sprite;    // point to the drawing manager to use
    LPDIRECT3DTEXTURE9 tex;        // texture spread over object's surface 

    #elif GRAPHICS_API == OPENGL
    unsigned int tex;              // texture spread over object's surface
    #endif

	DeviceTexture(const char* file, unsigned flags, Colour brdrClr);
	virtual ~DeviceTexture();

	void setSamplerState(int i);
	void setup();

  public:
	friend IDeviceTexture* CreateDeviceTexture(const char* file, 
	 unsigned flags, Colour brdrClr);
	void   attach(int i);
	void   detach(int i);
	void   draw(int topLeftX, int topLeftY, int bottomRightX,
     int bottomRightY);
	void   suspend();
	void   Delete() { delete this; }
	friend class Display;
};

//-------------------------------- Font ----------------------------------
//
// Font manages a single Text object as stored on the graphics card
//
class IText;

class Font : public IFont {

	static int width;     // width of the client area
	static int height;    // height of the client area

    const    IText* text; // points to the parent Text object 
	unsigned flags;       // font display flags

    #if GRAPHICS_API == DIRECT3D
    static LPDIRECT3DDEVICE9 d3dd; // Direct3D display device interface
    static LPD3DXSPRITE sprite;    // points to font manager COM object
    LPD3DXFONT d3dfont;            // points to the Direct3D font

    #elif GRAPHICS_API == OPENGL
    static HGLRC hrc;              // OpenGL rendering context
	static HDC hdc;                // Windows device context
	bool  isSetup;                 // the font object is setup?
    #endif

    Font(const IText* t, unsigned f);
    Font(const Font& v);
    Font& operator=(const Font& v);
    virtual ~Font();
    void    setup();

  public:
	friend IFont* CreateFont_(IText* text, unsigned int flags);
    void   draw(const char* text);
    void   suspend();
    bool   restore();
	void   release();
	void   Delete() { delete this; }
    friend class Display;
};

#if GRAPHICS_API == DIRECT3D
#elif GRAPHICS_API == OPENGL
unsigned char *createTexData(const char *file, int &width, int &height);
#endif

#endif