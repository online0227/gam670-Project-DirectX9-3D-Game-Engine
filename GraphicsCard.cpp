/* GraphicsCard Module Implementation
 *
 * GraphicsCard.cpp
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "IConfiguration.h" // for Selector and Window interfaces
#include "IScene.h"         // for Scene Object and Texture interfaces
#include "ILighting.h"      // for Lighting interface
#include "IHUD.h"           // for HUD interface
#include "math.h"           // for Matrix, Vector and Colour
#include "DeviceSettings.h" // for WND_WIDTH, WND_HEIGHT, RUN_IN_WINDOW
#include "Utilities.h"      // for error()
#include "GraphicsCard.h"   // for Host, Display, DeviceLight, Graphic,
                            // DeviceTexture and Font class declarations
  
#if GRAPHICS_API == DIRECT3D
#elif GRAPHICS_API == OPENGL
#include <gl/gl.h>
#include <gl/glu.h>
#include <cstdio>
#include <new>
using namespace std;
#endif

//-------------------------------- Host ---------------------------------
//
// Host provides access to the graphics card for capability interrogation
//
IHost* CreateHost(void* hDbWnd) {

	return new Host(hDbWnd);
}

// constructor retrieves the interface to the reference object that 
// provides access to the host system
//
Host::Host(void* hDbWnd) {

	hwnd = (HWND)hDbWnd;

    #if GRAPHICS_API == DIRECT3D
	// obtain an interface to Direct3D object
    if (!(d3d = Direct3DCreate9(D3D_SDK_VERSION)))
        error("Host::10 Unable to make Direct3D object", hwnd);

    #elif GRAPHICS_API == OPENGL
	// create a device context
    if (!(hdc = GetDC(hwnd)))
        error("Host::10 Failed to create a device context", hwnd);
    else
        // set the default pixel format to that for the device context
        format = GetPixelFormat(hdc);
    #endif
}

// adapterCount retrieves the number of installed display adapters
//
int Host::adapterCount() const {

    unsigned iDevNum;

    #if GRAPHICS_API == DIRECT3D
    iDevNum = d3d->GetAdapterCount();

    #elif GRAPHICS_API == OPENGL
    DISPLAY_DEVICE dsdv;
    dsdv.cb = sizeof dsdv;
    iDevNum = 0;
    while (EnumDisplayDevices(NULL, iDevNum, &dsdv, 0x00000001))
        iDevNum++;
    #endif

    return iDevNum;
}

// displayDescription fills desc[maxDesc+1] with the user friendly 
// description of display adapter id
//
int Host::displayDescription(int id, char* desc, int maxDesc) const {

    int rc = 0;

    #if GRAPHICS_API == DIRECT3D
    D3DADAPTER_IDENTIFIER9 d3di;

    if (SUCCEEDED(d3d->GetAdapterIdentifier(id, 0, &d3di))) {
        strcopy(desc, d3di.Description, maxDesc);
        rc = 1;
    }

    #elif GRAPHICS_API == OPENGL
    DISPLAY_DEVICE dsdv;
    dsdv.cb = sizeof dsdv;

    EnumDisplayDevices(NULL, id, &dsdv, 0x00000001);
    strcopy(desc, dsdv.DeviceString, maxDesc);

    // check that this display has at least one mode
    // (EnumDisplaySettings returns the number of modes)
    //
    DWORD iModeNum = 0;
    DEVMODE mode;
    mode.dmSize = sizeof mode;
    mode.dmDriverExtra = 0;
    rc = EnumDisplaySettings(dsdv.DeviceName, iModeNum, &mode);
    #endif

    return rc;
}

// formatCount returns the number of supported pixel formats and sets 
// the default pixel format
//
int Host::formatCount() {

    int count = 0;

    #if GRAPHICS_API == DIRECT3D
    // number of formats described in documentation
    count = D3D_NO_DOC_FORMATS;
    // set the default format
    format = D3DFMT_R5G6B5;

    #elif GRAPHICS_API == OPENGL
    PIXELFORMATDESCRIPTOR pfd;
    int size = sizeof pfd;

    // maximum number of pixel formats on the system
    int max = DescribePixelFormat(hdc, 1, size, NULL);

    // set the default format to the pixel format for
    // the current device context
    format = GetPixelFormat(hdc);

    // enumerate the pixel formats that meet OpenGL requirements
    // note that indexing for available pixel formats is 1-based
    //
    for (int i = 1; i <= max && count < MFORMAT; i++) {
        if (DescribePixelFormat(hdc, i, size, &pfd) &&
         pfd.dwFlags & PFD_DRAW_TO_WINDOW &&
         pfd.dwFlags & PFD_SUPPORT_OPENGL &&
         pfd.dwFlags & PFD_DOUBLEBUFFER &&
         (pfd.cDepthBits == 16 || pfd.cDepthBits == 32)) {
            pixelFmt[count++]  = i;
        }
    }
    #endif

    return count;
}

// modeCount returns the number of modes available on display id
// at pixel format[ip]
//
int Host::modeCount(int id, int ip) const {

    int iModeNum;

    #if GRAPHICS_API == DIRECT3D
    // The Direct3D formats are descibed in the Direct3D documentation.
    D3DFORMAT Format[] = D3D_DOC_FORMATS;
    iModeNum = d3d->GetAdapterModeCount(id, Format[ip]);

    #elif GRAPHICS_API == OPENGL
    DISPLAY_DEVICE dsdv;
    dsdv.cb = sizeof dsdv;
    EnumDisplayDevices(NULL, id, &dsdv, 0x00000001);

    DEVMODE mode;
    mode.dmSize = sizeof mode;
    mode.dmDriverExtra = 0;

    iModeNum = 0;
    while (EnumDisplaySettings(dsdv.DeviceName, iModeNum, &mode))
        iModeNum++;

    #endif

    return iModeNum;
}

// getMode retrieves the properties of display id in mode ir at pixel format
// ip.  getMode returns in fmMd the packed format|mode and in line the 
// descriptive string for display in the dialog combo box
//
bool Host::getMode(int id, int ir, int ip, unsigned& fmMd, char* line) {

    bool rc = false;

    #if GRAPHICS_API == DIRECT3D
    D3DDISPLAYMODE mode;

    // The Direct3D formats are descibed in the D3D documentation.
    D3DFORMAT Format[] = D3D_DOC_FORMATS;

    // short descriptions for each format
    char *fmtdesc[] = D3D_FORMAT_DESC;

    if (SUCCEEDED(d3d->EnumAdapterModes(id, Format[ip], ir, &mode))
     && mode.Width >= WND_WIDTH && mode.Height >= WND_HEIGHT &&
     (D3D_OK == d3d->CheckDeviceFormat(id,
     D3DDEVTYPE_HAL, mode.Format, D3DUSAGE_DEPTHSTENCIL,
     D3DRTYPE_SURFACE, D3DFMT_D16) ||
     D3D_OK == d3d->CheckDeviceFormat(id,
     D3DDEVTYPE_HAL, mode.Format, D3DUSAGE_DEPTHSTENCIL,
     D3DRTYPE_SURFACE, D3DFMT_D32) ||
     D3D_OK == d3d->CheckDeviceFormat(id,
     D3DDEVTYPE_REF, mode.Format, D3DUSAGE_DEPTHSTENCIL,
     D3DRTYPE_SURFACE, D3DFMT_D16) ||
     D3D_OK == d3d->CheckDeviceFormat(id,
     D3DDEVTYPE_REF, mode.Format, D3DUSAGE_DEPTHSTENCIL,
     D3DRTYPE_SURFACE, D3DFMT_D32))) {
        char hz[20] = "";
        if (mode.RefreshRate)
            wsprintf(hz,"(%d Hz)", mode.RefreshRate);
        wsprintf(line, "%dx%d %s %s bits", mode.Width,
         mode.Height, hz, fmtdesc[ip]);
        fmMd = (ip << 16) | ir;
        rc = true;
    }

    #elif GRAPHICS_API == OPENGL
    DISPLAY_DEVICE dsdv;
    dsdv.cb = sizeof dsdv;
    EnumDisplayDevices(NULL, id, &dsdv, 0x00000001);

    DEVMODE mode;
    mode.dmSize = sizeof mode;
    mode.dmDriverExtra = 0;
    PIXELFORMATDESCRIPTOR pfd;

    if (EnumDisplaySettings(dsdv.DeviceName, ir, &mode) &&
     mode.dmPelsWidth >= WND_WIDTH && mode.dmPelsHeight >= WND_HEIGHT &&
     DescribePixelFormat(hdc, pixelFmt[ip], sizeof pfd, &pfd) &&
     mode.dmBitsPerPel >= (DWORD)(pfd.cColorBits + pfd.cAlphaBits) &&
     ChangeDisplaySettings(&mode, CDS_TEST) == DISP_CHANGE_SUCCESSFUL) {
        char hz[20] = "";
        if (mode.dmDisplayFrequency)
            wsprintf(hz,"(%d Hz)", mode.dmDisplayFrequency);
        wsprintf(line, "%dx%d %s R%dG%dB%dA%d bits",
         mode.dmPelsWidth, mode.dmPelsHeight, hz, pfd.cRedBits,
         pfd.cGreenBits, pfd.cBlueBits, pfd.cAlphaBits);
        fmMd = (ip << 16) | ir;
        rc = true;
    }
    #endif

    return rc;
}

// configure returns the properties for setting up the display device
// that are associated with display adapter id and format/mode fmMd
//
void Host::configure(unsigned fmMd, int& id, int& ir, int& ip, 
 int& width, int& height, int& wndStyle, int& wndExStyle) {

    if (id == RUN_IN_WINDOW) {
        width      = WND_WIDTH;
        height     = WND_HEIGHT;
		ir         = 0;
        #if GRAPHICS_API == DIRECT3D
		ip         = D3D_FORMAT_WINDOW;
        #elif GRAPHICS_API == OPENGL
		ip         = format;
        #endif
        wndExStyle = WND_EXSTYLE;
        wndStyle   = WND_STYLE;
    }
    else {
        #if GRAPHICS_API == DIRECT3D
        D3DFORMAT Format[] = D3D_DOC_FORMATS;
        // unpack the resolution and format ids
        ir = fmMd & 0xFFFF;
        ip = (fmMd >> 16);
        D3DDISPLAYMODE d3ddm;    // holds mode info
        // extract the mode data
        if (FAILED(d3d->EnumAdapterModes(id, Format[ip], ir, 
         &d3ddm))) {
			// failed: so revert to windowed mode
            error("Host::52 Unable to get display mode", hwnd);
            error("Host::53 Defaulting to run in windowed mode", hwnd);
            width      = WND_WIDTH;
            height     = WND_HEIGHT;
            id         = RUN_IN_WINDOW;
			ir         = 0;
			ip         = D3D_FORMAT_WINDOW;
			wndExStyle = WND_EXSTYLE;
			wndStyle   = WND_STYLE;
        }
        else {
            width      = d3ddm.Width;
            height     = d3ddm.Height;
			wndExStyle = WS_EX_TOPMOST;
			wndStyle   = WS_POPUP;
        }

        #elif GRAPHICS_API == OPENGL
        DISPLAY_DEVICE dsdv;
        dsdv.cb = sizeof dsdv;
        EnumDisplayDevices(NULL, id, &dsdv, 0x00000001);
        // retrieve mode data
        DEVMODE mode;
        mode.dmSize = sizeof mode;
        ir = fmMd & 0xFFFF;
        ip = (fmMd >> 16);
        EnumDisplaySettings(dsdv.DeviceName, ir, &mode);
        if (ChangeDisplaySettings(&mode, CDS_FULLSCREEN) !=
         DISP_CHANGE_SUCCESSFUL) {
            error("Host::52 Failed to change to fullscreen at "
             "selected mode", hwnd);
            error("Host::53 Reverting to window mode", hwnd);
            width      = WND_WIDTH;
            height     = WND_HEIGHT;
            id         = RUN_IN_WINDOW;
			ir         = 0;
			ip         = format;
			wndExStyle = WND_EXSTYLE;
			wndStyle   = WND_STYLE;
        }
        else {
            width      = mode.dmPelsWidth;
            height     = mode.dmPelsHeight;
            ip         = pixelFmt[ip];
			wndExStyle = WS_EX_APPWINDOW;
			wndStyle   = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
        }
        #endif
    }
}

// destructor releases the reference object
//
Host::~Host() {

    // release the handle to the reference object
    #if GRAPHICS_API == DIRECT3D
    if (d3d) {
        d3d->Release();
        d3d = NULL;
    }

    #elif GRAPHICS_API == OPENGL
    // release the device context
    if (hdc) {
        ReleaseDC(hwnd, hdc);
        hdc = NULL;
    }
    #endif
}

//-------------------------------- Display ------------------------------
//
// Display manages the display device on the graphics card
//
IDisplay* CreateDisplay(IScene* s, ILighting* l, IHUD* h, ICameras* ca) {

	return new Display(s, l, h, ca);
}

// constructor initializes the instance variables
//
Display::Display(IScene* s, ILighting* l, IHUD* h, ICameras* ca) : scene(s),
 lighting(l), hud(h), cameras(ca) { //particle implementation - 카메라 집어처넣기

    #if GRAPHICS_API == DIRECT3D
    // obtain interface to Direct3D COM object
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d)
        error("Display::02 Unable to make Direct3D object");
    d3dd        = NULL;
    sprite = NULL;
    hud_tex     = NULL;
	fvf         = (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 |\
                        D3DFVF_TEXCOORDSIZE2(0));

    #elif GRAPHICS_API == OPENGL
    hdc = NULL;
    hrc = NULL;
    #endif

    // setup parameters
	display  = 0;
    mode     = 0;
    pixel    = 0;
    width    = 0;
    height   = 0;
    titleBar = 0;
	

}

// setup retrieves the interface to the display device and associates it
// with the application window
//
bool Display::setup() {

    bool rc = false;
	
	//particle implementation
	/* it should be declared here because parameters imply that scene and cameras were already setup. look at Game::setup() and see the order */
	psGun = new ParticleGun(cameras->returnCurCar(), scene->returnCurrentHero()); //particle implementation

	terrain = scene->returnCurrentTerrain();
	Vector tempMin = terrain->returnBoundingMin();
	Vector tempMax = terrain->returnBoungingMax();
	tempMax.y += 100;

	boundingBox._min = D3DXVECTOR3(tempMin.x, tempMin.y, tempMin.z);
	boundingBox._max = D3DXVECTOR3(tempMax.x, tempMax.y, tempMax.z);

	psSnow = new Snow(&boundingBox, 1000);
    // see the end of this class, there is another implementation for particle system.

	SelectorAddress()->configureD(width, height, display, mode, pixel, 
	 titleBar);
	hwnd = (HWND)WindowAddress()->window();

    // release the interface if one is currently set
    release();

    #if GRAPHICS_API == DIRECT3D
    UINT adapter;
	D3DFORMAT Format[] = D3D_DOC_FORMATS;
    D3DFORMAT d3dFormat = Format[pixel];

    // set the D3D presentation parameters
    ZeroMemory(&d3dpp, sizeof d3dpp);
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferCount = 1;
    d3dpp.EnableAutoDepthStencil = TRUE;
    if (display == RUN_IN_WINDOW) {
        d3dpp.Windowed = TRUE;
		d3dpp.BackBufferWidth = width;
		d3dpp.BackBufferHeight = height;
        adapter = D3DADAPTER_DEFAULT;
    }
    else {
        D3DDISPLAYMODE d3ddm;
        if (FAILED(d3d->EnumAdapterModes(display, d3dFormat, mode, &d3ddm)))
            error("Display::10 Unable to get selected display mode");
        else {
            d3dpp.BackBufferWidth  = d3ddm.Width;
            d3dpp.BackBufferHeight = d3ddm.Height;
            d3dpp.BackBufferFormat = d3ddm.Format;
			d3dpp.FullScreen_RefreshRateInHz = d3ddm.RefreshRate;
            width   = d3ddm.Width;
            height  = d3ddm.Height;
            adapter = display;
        }
    }

    // find the best format for depth buffering and stenciling
    //
    D3DDEVTYPE devtype;
    if (D3D_OK == d3d->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL,
     d3dFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24S8)) {
        d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8; // depth & stencil
        devtype = D3DDEVTYPE_HAL;                    // REF or HAL
    }
    else if (D3D_OK == d3d->CheckDeviceFormat(adapter, D3DDEVTYPE_HAL, 
     d3dFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, D3DFMT_D24X4S4)) {
        d3dpp.AutoDepthStencilFormat = D3DFMT_D24X4S4;
        devtype = D3DDEVTYPE_HAL;
    }
    else if (D3D_OK == d3d->CheckDeviceFormat(adapter,
     D3DDEVTYPE_HAL, d3dFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE,
     D3DFMT_D15S1)) {
        d3dpp.AutoDepthStencilFormat = D3DFMT_D15S1;
        devtype = D3DDEVTYPE_HAL;
    }
    // if the above attempts fail, use the REF (software emulation) device
    // with a 32-bit depth buffer rather than the HAL (hardware accelerated) 
	// device
    else if (D3D_OK == d3d->CheckDeviceFormat(adapter,
     D3DDEVTYPE_REF, d3dFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE,
     D3DFMT_D32)) {
        d3dpp.AutoDepthStencilFormat = D3DFMT_D32;
        devtype = D3DDEVTYPE_REF;
    }
    // if all else fails, use the REF (software emulation) with a 16-bit
    // depth buffer, hoping that it will work. (If it doesn't, we are out
    // of luck anyway.)
    else {
        d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
        devtype = D3DDEVTYPE_REF;
    }

	// check if full-scene antialiasing is supported, if so, set 
	// presentation parameter
	if(SUCCEEDED(d3d->CheckDeviceMultiSampleType(adapter, devtype,
	 d3dpp.AutoDepthStencilFormat, FALSE, 
	 D3DMULTISAMPLE_2_SAMPLES, NULL ) ) )
	    d3dpp.MultiSampleType  = D3DMULTISAMPLE_2_SAMPLES;

    // retrieve the interface to the D3D display device
    if (FAILED(d3d->CreateDevice(adapter, devtype, hwnd,
     D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3dd)))
        error("Display::12 Unable to create D3D device");
    else {
        // extract the hardware capabilities and configure the limits
        D3DCAPS9 caps;
        d3d->GetDeviceCaps(adapter, devtype, &caps);
        // maximum number of lights supported by the display device
        maxLights = caps.MaxActiveLights ? caps.MaxActiveLights : MAX_LIGHTS;
        // set anisotropic filtering to the maximum available on the device
        if (FAILED(d3dd->SetSamplerState(0, D3DSAMP_MAXANISOTROPY,
         caps.MaxAnisotropy - 1)))
            error("Display::13 Unable to set up anisotropic filtering");
		// maximum number of texture stages supported by the display device
		int maxStages = min(caps.MaxSimultaneousTextures, 
		 caps.MaxTextureBlendStages);

		// create a sprite COM object to draw the hud texture and the 
		// text item fonts as well as the background texture
		if (!sprite && FAILED(D3DXCreateSprite(d3dd, &sprite)))
			error("Display::14 Failed to create the font manager");
		// load the hud background file image into a texture COM object
		if (hud->file() && FAILED(D3DXCreateTextureFromFileEx(d3dd, hud->file(), 
		  unsigned(width * hud->width()), unsigned(height * hud->height()), 
		  D3DX_DEFAULT, NULL, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT, 
		  D3DX_DEFAULT, D3DCOLOR_XRGB(0, 0, 0), NULL, NULL, &hud_tex))) {
			error("Display::15 Unable to load hud image from file");
		}

		// set the vertex format
        d3dd->SetFVF(fvf);

		// set class variables for related classes
		DeviceLight::d3dd        = d3dd;
		Graphic::d3dd            = d3dd;
		Graphic::fvf             = fvf;
		Mesh::d3dd               = d3dd;
		Mesh::fvf                = fvf;
		DeviceTexture::width     = width;
		DeviceTexture::height    = height - titleBar;
		DeviceTexture::d3dd      = d3dd;
		DeviceTexture::sprite    = sprite;
		DeviceTexture::maxStages = maxStages;
		Font::d3dd               = d3dd;
		Font::width              = width;
		Font::height             = height - titleBar;
		Font::sprite             = sprite;

		//particle implementation
		psGun->init("flare_alpha.dds", d3dd);
		psSnow->init("snowflake.dds", d3dd);

		// setup successful
        rc = true;
    }

    #elif GRAPHICS_API == OPENGL
    PIXELFORMATDESCRIPTOR pfd;

    // Get a device context (Windows drawing destination) so that we
    // can make a rendering context (OpenGL drawing destination)
    // This device context is released in the destructor
    //
    hdc = GetDC(hwnd);

    // retrieve the color depth of the requested pixel format
    DescribePixelFormat(hdc, pixel, sizeof pfd, &pfd);
    int colourBits = pfd.cColorBits;
    int zDepthBits = pfd.cDepthBits;

    // find the most suitable pixel format
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof pfd;
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
     PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = colourBits;
    pfd.cDepthBits = zDepthBits;
    pfd.iLayerType = PFD_MAIN_PLANE;
    int iPixelFormat = ChoosePixelFormat(hdc, &pfd);

    // set the pixel format for the device context
    // can only be done once for a window
    //
    SetPixelFormat(hdc, iPixelFormat, &pfd);

    // create an OpenGL rendering context
    if (!(hrc = wglCreateContext(hdc)))
        error("Display::10 Failed to create an OpenGL rendering context");
    // Make the rendering context the current drawing destination
    else if (!wglMakeCurrent(hdc, hrc))
        error("Display::11 Failed to make rendering context current");
    else {
        // Turn on backface culling (default is to render both sides at extra cost)
        glEnable(GL_CULL_FACE);
		// set maximum number of lights 
		maxLights = 8;

		// set class variables for related classes
		Font::hrc    = hrc;
		Font::hdc    = hdc;
		Font::width  = width;
		Font::height = height - titleBar;

		// setup successful
        rc = true;
    }
    #endif

	// complete the setup
	if (rc) {
        setupProjection();
        setupLighting();
		setGlobalState();
	}

    return rc;
}

// setupProjection calculates the transformation from camera space to
// homogeneous clip space and applies it to the display device
//
void Display::setupProjection() {

    #if GRAPHICS_API == DIRECT3D
	Matrix projection;
    projectionFov(projection, FIELD_OF_VIEW, width/float(height), 
		NEAR_CLIPPING, FAR_CLIPPING);
    d3dd->SetTransform(D3DTS_PROJECTION, (D3DXMATRIX*)&projection);

    #elif GRAPHICS_API == OPENGL
    // reset the viewport
    glViewport(0, 0, width, height);

    // set up the projection transformation, and leave things
    // in Game/View form, which everything else is expecting.
    //
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FIELD_OF_VIEW, width/(float)height, NEAR_CLIPPING,
     FAR_CLIPPING);
    glMatrixMode(GL_MODELVIEW);
    #endif
}

// setupLighting sets up the lighting on the display device
//
void Display::setupLighting() {

    #if GRAPHICS_API == DIRECT3D
    // setup the global ambient light
    Colour c = *lighting->ambient();
    d3dd->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_COLORVALUE(c.r, c.g, c.b, c.a));

    // allow specular highlights (can be slow on some machines)
    d3dd->SetRenderState(D3DRS_SPECULARENABLE, TRUE);

    #elif GRAPHICS_API == OPENGL
    glEnable(GL_LIGHTING);
    GLfloat black[] = {0, 0, 0, 1};
    glMaterialfv(GL_FRONT, GL_EMISSION, black);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    #endif

    lighting->setupDeviceLights(maxLights);
}

// setGlobalState sets up colour dithering and sets the formula for
// alpha blending
//
void Display::setGlobalState() {

    #if GRAPHICS_API == DIRECT3D
    // allow colour dithering (much smoother looking when using lights)
    d3dd->SetRenderState(D3DRS_DITHERENABLE, TRUE);

    // how alpha-blending is done (when drawing transparent things)
    d3dd->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    d3dd->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	#elif GRAPHICS_API == OPENGL
    // how alpha-blending should be done, when we draw transparent things
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    #endif
}

// draw draws a frame of the Scene and then superimposes the hud
//
void Display::draw(const Vector& p, const Vector& h, const Vector& u) {

    // construct the view matrix
    //
    Vector a = p + h;



    #if GRAPHICS_API == DIRECT3D
    Matrix v;
    view(v, p, a, u);
    d3dd->SetTransform(D3DTS_VIEW, (D3DXMATRIX*)&v);
	//ps->update(0.001f); //particle implementation

    #elif GRAPHICS_API == OPENGL
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    gluLookAt(p.x, p.y, p.z, a.x, a.y, a.z, u.x, u.y, u.z);
    glMatrixMode(GL_MODELVIEW);
    #endif

    // clear the backbuffer
    //
    #if GRAPHICS_API == DIRECT3D
    d3dd->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
     D3DCOLOR_XRGB(BGROUND_R, BGROUND_G, BGROUND_B), 1.0, 0);
    d3dd->BeginScene();
	if (sprite) {
		sprite->Begin(0);
		// draw the background image for the scene
		scene->drawBackground();
		sprite->End();
	}

		//particle implementation
	#if GRAPHICS_API == DIRECT3D
	//ps->render();
    #elif GRAPHICS_API == OPENGL
    
    #endif

    #elif GRAPHICS_API == OPENGL
    glEnable(GL_DEPTH_TEST);
    glClearColor(BGROUND_R, BGROUND_G, BGROUND_B, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    #endif

    // draw the opaque objects in the scene first
    scene->drawOpaque();
    // turn on the alpha blending
    #if GRAPHICS_API == DIRECT3D
    d3dd->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    #elif GRAPHICS_API == OPENGL
    glEnable(GL_BLEND);
    #endif
    // draw the translucent and transparent objects next
    scene->drawTranslucent();
    // turn off the alpha blending
    #if GRAPHICS_API == DIRECT3D
    d3dd->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	d3dd->SetRenderState(D3DRS_ZENABLE, false);
	psGun->render();
	//psSnow->update(0.1f);
	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	d3dd->SetTransform(D3DTS_WORLD, &I);
	psSnow->render();
	d3dd->SetRenderState(D3DRS_ZENABLE, true);

    #elif GRAPHICS_API == OPENGL
    glDisable(GL_BLEND);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    #endif




    // draw the hud
	if (hud->isOn()) {
		#if GRAPHICS_API == DIRECT3D
		if (sprite) {
			sprite->Begin(D3DXSPRITE_ALPHABLEND);
			//float sx = 1;
			//float sy = 1;
			//D3DXMATRIX m(sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
			//sprite->SetTransform(&m);
			// draw the background image for the hud
			if (hud_tex) {
				Vector position = hud->position(width, height - titleBar);
				sprite->Draw(hud_tex, NULL, NULL, 
				 (D3DXVECTOR3*)&position,
				 D3DCOLOR_RGBA(SPRITE_R, SPRITE_G, SPRITE_B, SPRITE_A));
			}
		}
		#elif GRAPHICS_API == OPENGL
		glMatrixMode(GL_PROJECTION);
		// save the perspective projection
		glPushMatrix();
		// make an orthographic projection
		glLoadIdentity();
		gluOrtho2D(0, width, 0, height);
		glMatrixMode(GL_MODELVIEW);
		// save the gameview
		glPushMatrix();
		// empty the gameview matrix
		glLoadIdentity();
		// disable lighting
		glDisable(GL_LIGHTING);
		// disable texturing before writing text
		glDisable(GL_TEXTURE_2D);
		// set text color
		glColor4f(TEXT_R, TEXT_G, TEXT_B, TEXT_A);
		#endif
		// draw the fonts
		hud->draw();
		// conclude the drawing of the hud
		#if GRAPHICS_API == DIRECT3D
		if (sprite)
			sprite->End();
		#elif GRAPHICS_API == OPENGL
		// re-enable lighting
		glEnable(GL_LIGHTING);
		// re-enable textureing after writing
		glEnable(GL_TEXTURE_2D);
		// restore perspective projection and view matrices
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		#endif
	}

    // present the backbuffer to the primary buffer
    //
    #if GRAPHICS_API == DIRECT3D
    d3dd->EndScene();
    if (FAILED(d3dd->Present(NULL, NULL, NULL, NULL)))
        error("Display::40 Failed to flip backbuffer");

    #elif GRAPHICS_API == OPENGL
    if (!SwapBuffers(hdc))
        error("Display::40 Failed to swap buffers");
    #endif
}

// wndResize resizes the main application window and recalculates
// the projection matrix
//
void Display::wndResize() {

	if (display == RUN_IN_WINDOW) {
		// find the new size of the client area
		RECT rect;
		GetClientRect(hwnd, &rect);
		AdjustWindowRectEx(&rect, WND_STYLE, FALSE, WND_EXSTYLE);
		width  = rect.right - rect.left;
		height = rect.bottom - rect.top;
		// check constraints
		if (width == 0) width = 1;
		if (height == 0) height = 1;

		// Set up the projection matrix
		#if GRAPHICS_API == DIRECT3D
		if (d3dd) 
		#elif GRAPHICS_API == OPENGL
		if (hrc) 
		#endif
			setupProjection();
	}
}

// suspend prepares the display device for de-activation
//
void Display::suspend() {

	// particle implementation
	//::Delete<ParticleSystem*>( ps );
	//ps = NULL;

	// detach the fonts from video memory
	hud->suspend();

    #if GRAPHICS_API == DIRECT3D
    // release the hud texture, if any
    if (hud_tex) {
        hud_tex->Release();
        hud_tex = NULL;
    }
    // detach the sprite manager from video memory
    if (sprite)
        sprite->OnLostDevice();

    #elif GRAPHICS_API == OPENGL
	#endif
}

// restore re-activates the display device
//
bool Display::restore() {

    bool rc = false;

    #if GRAPHICS_API == DIRECT3D
    if (d3dd) {
        HRESULT hr;
		hr = d3dd->TestCooperativeLevel();
		if (hr == D3DERR_DEVICENOTRESET) {
			// reset the display device
			hr = d3dd->Reset(&d3dpp);
			if (hr == D3DERR_DEVICELOST)
				error("Display::70 Device Lost");
			else if (hr == D3DERR_DRIVERINTERNALERROR)
				error("Display::71 Driver Internal Error");
			else if (hr == D3DERR_OUTOFVIDEOMEMORY)
				error("Display::72 Out of Video Memory");
			else if (hr != D3D_OK)
				error("Display::73 Unknown error");
			else {
				rc = true;
			}
		} 
		else if (hr == S_OK) {
			rc = true;
		}
	}
	if (rc) {
		// reset the vertex format
		d3dd->SetFVF(fvf);

		// reload the hud file image into the hud texture object
		if (hud->file() && !hud_tex && 
		 FAILED(D3DXCreateTextureFromFileEx(d3dd, hud->file(), 
		 unsigned(width * hud->width()), unsigned(height * hud->height()), 
		 D3DX_DEFAULT, NULL, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT,
		 D3DX_DEFAULT, D3DCOLOR_XRGB(0, 0, 0), NULL, NULL, &hud_tex))) {
			error("Display::75 Unable to load hud image from file");
		}
		// reacquire sprite manager references to video memory
		if (sprite) sprite->OnResetDevice();
    }

    #elif GRAPHICS_API == OPENGL
    if (hrc)
        rc = true;
    #endif

	// complete the restoration
	if (rc) {
        setupProjection();
        setupLighting();
		setGlobalState();
	}

    return rc;
}

// release releases the interfaces to the display device
//
void Display::release() {

    suspend();

	hud->release();

	#if GRAPHICS_API == DIRECT3D
    // release the font manager
    if (sprite) {
        sprite->Release();
        sprite = NULL;
		Font::sprite = NULL;
    }
	// release the display device
    if (d3dd) {
        d3dd->Release();
        d3dd = NULL;
		DeviceLight::d3dd   = NULL;
		Graphic::d3dd       = NULL;
		DeviceTexture::d3dd = NULL;
		Font::d3dd          = NULL;
    }
    #endif
}

// destructor release the display object along with the interface to the
// reference object
//
Display::~Display() {

    release();

    #if GRAPHICS_API == DIRECT3D
    if (d3d) {
        d3d->Release();
        d3d = NULL;
    }



    #elif GRAPHICS_API == OPENGL
    if (hdc) {
        if (hrc) {
            // uncouple rendering context from device context
            wglMakeCurrent(NULL, NULL);
            // release rendering context created in setup()
            wglDeleteContext(hrc);
        }
        // release DC created in setup()
        ReleaseDC(hwnd, hdc);
    }

    // revert to original resolution if fullscreen application
    //
    if (display != RUN_IN_WINDOW) {
        ChangeDisplaySettings(NULL, 0);
    }
    #endif
}

//-------------------------------- DeviceLight -----------------------------
//
// DeviceLight manages a single light source on the graphics card
//
IDeviceLight* CreateDeviceLight(ILight* light) {

	return new DeviceLight(light);
}

#if GRAPHICS_API == DIRECT3D
LPDIRECT3DDEVICE9 DeviceLight::d3dd = NULL;  // Direct3D display device
#elif GRAPHICS_API == OPENGL
#endif

// constructor stores the address of the light source
//
DeviceLight::DeviceLight(ILight* l) : light(l) {}

// setup sets up the device light on the graphics card
//
void DeviceLight::setup(int i) {

	Colour c;
    Vector v;
	index = i;

    #if GRAPHICS_API == DIRECT3D
    D3DLIGHT9 d3dLight;
    ZeroMemory(&d3dLight, sizeof d3dLight);
    switch (light->isType()) {
      case POINT_LIGHT:
        d3dLight.Type = D3DLIGHT_POINT;
        break;
      case SPOT_LIGHT:
        d3dLight.Type = D3DLIGHT_SPOT;
        break;
      case DIRECTIONAL_LIGHT:
        d3dLight.Type = D3DLIGHT_DIRECTIONAL;
        break;
    }
    c = light->diffuse();
    d3dLight.Diffuse      = D3DXCOLOR(c.r, c.g, c.b, c.a);
    c = light->ambient();
    d3dLight.Ambient      = D3DXCOLOR(c.r, c.g, c.b, c.a);
    c = light->specular();
    d3dLight.Specular     = D3DXCOLOR(c.r, c.g, c.b, c.a);
    v = light->position();
    v.z = v.z * ZAXIS_DIRECTION;
    d3dLight.Position     = D3DXVECTOR3(v.x, v.y, v.z);
    v = light->direction();
    v.z = v.z * ZAXIS_DIRECTION;
    d3dLight.Direction    = D3DXVECTOR3(v.x, v.y, v.z);
    d3dLight.Attenuation0 = light->attenuation0();
    d3dLight.Attenuation1 = light->attenuation1();
    d3dLight.Attenuation2 = light->attenuation2();
    d3dLight.Falloff      = light->falloff();
    d3dLight.Range        = light->range();
    d3dLight.Theta        = light->theta();
    d3dLight.Phi          = light->phi();

    if (FAILED(d3dd->SetLight(index, &d3dLight)) ||
     FAILED(d3dd->LightEnable(index, light->isOn())))
        error("Display::20 Unable to create light");

    #elif GRAPHICS_API == OPENGL
    float lightType;
    float spread;
    switch (light->isType()) {
      case POINT_LIGHT:
        lightType = 1;
        spread = 180;  // degrees of spread
        break;
      case SPOT_LIGHT:
        lightType = 1;
        spread = light->phi() * 57.3f;  // degrees of spread
        break;
      case DIRECTIONAL_LIGHT:
        lightType = 0;
        spread = 180;  // degrees of spread
        break;
    }
    int j = GL_LIGHT0 + index;
    c = light->diffuse();
    GLfloat diffuse[] = {c.r, c.g, c.b, c.a};
    glLightfv(j, GL_DIFFUSE,  diffuse);
    c = light->ambient();
    GLfloat ambient[] = {c.r, c.g, c.b, c.a};
    glLightfv(j, GL_AMBIENT,  ambient);
    c = light->specular();
    GLfloat specular[] = {c.r, c.g, c.b, c.a};
    glLightfv(j, GL_SPECULAR,  specular);
    v = light->position();
    v.z = v.z * ZAXIS_DIRECTION;
    GLfloat lightpos[] = {v.x, v.y, v.z, lightType};
    glLightfv(j, GL_POSITION, lightpos);
    v = light->direction();
    v.z = v.z * ZAXIS_DIRECTION;
    GLfloat lightdir[] = {v.x, v.y, v.z};
    glLightfv(j, GL_SPOT_DIRECTION, lightdir);
    glLightf (j, GL_SPOT_CUTOFF, spread);
    glLightf (j, GL_SPOT_EXPONENT, light->falloff());
    glLightf (j, GL_CONSTANT_ATTENUATION, light->attenuation0());
    glLightf (j, GL_LINEAR_ATTENUATION, light->attenuation1());
    glLightf (j, GL_QUADRATIC_ATTENUATION, light->attenuation2());
    glEnable(j);
    #endif
}

// update updates the device light on the graphics card 
//
void DeviceLight::update() {

    #if GRAPHICS_API == DIRECT3D
    d3dd->LightEnable(index, light->isOn());
    #elif GRAPHICS_API == OPENGL
    light->isOn() ? glEnable(GL_LIGHT0 + index) : glDisable(GL_LIGHT0 + 
	 index);
    #endif
}

//-------------------------------- Graphic ---------------------------------
//
// Graphic manages the graphics primitives that represent an object on the
// graphics card
//
IGraphic* CreateGraphic(Shape pType, int noPrimitives, int noVertices, 
 int vSize, int noIndices, Colour clr, IDeviceTexture* devTex, 
 bool antiAlias) {

	return new Graphic(pType, noPrimitives, noVertices, vSize, noIndices, 
	 clr, devTex, antiAlias);
}

#if GRAPHICS_API == DIRECT3D
LPDIRECT3DDEVICE9 Graphic::d3dd = NULL;
unsigned Graphic::fvf = NULL;
#elif GRAPHICS_API == OPENGL
#endif

// constructor identifies the graphics api primitive type and stores the
// reflective colour
//
Graphic::Graphic(Shape pType, int noPrimitives, int noVertices, 
 int vSize, int noIndices, Colour clr, IDeviceTexture* devTex, 
 bool antiAlias) : nTextures(0), deviceTexture(NULL), vb(NULL),
 ib(NULL), nVertices(noVertices), vertexBufferSize(noVertices * vSize), 
 vertexSize(vSize), indexBufferSize(noIndices * sizeof(short)) {

    // store the primitive type
	switch (pType) {
        #if GRAPHICS_API == DIRECT3D
        case POINT_LIST    : type = D3DPT_POINTLIST;     break;
        case LINE_LIST     : type = D3DPT_LINELIST;      break;
        case LINE_STRIP    : type = D3DPT_LINESTRIP;     break;
        case TRIANGLE_LIST : type = D3DPT_TRIANGLELIST;  break;
        case TRIANGLE_STRIP: type = D3DPT_TRIANGLESTRIP; break;
        case TRIANGLE_FAN  : type = D3DPT_TRIANGLEFAN;   break;
        default            : type = D3DPT_POINTLIST;

        #elif GRAPHICS_API == OPENGL
        case POINT_LIST    : type = GL_POINTS;         break;
        case LINE_LIST     : type = GL_LINES;          break;
        case LINE_STRIP    : type = GL_LINE_STRIP;     break;
        case TRIANGLE_LIST : type = GL_TRIANGLES;      break;
        case TRIANGLE_STRIP: type = GL_TRIANGLE_STRIP; break;
        case TRIANGLE_FAN  : type = GL_TRIANGLE_FAN;   break;
        default            : type = GL_POINTS;
        #endif
    }

    #if GRAPHICS_API == DIRECT3D
    nPrimitives = noPrimitives > 0 ? noPrimitives : 1;
    // make a shiny material of the specified color
    ZeroMemory(&mat, sizeof(mat));
    mat.Ambient  = D3DXCOLOR(clr.r*0.7f, clr.g*0.7f, clr.b*0.7f, clr.a);
    mat.Diffuse  = D3DXCOLOR(clr.r, clr.g, clr.b, clr.a); // reflected from lights
    mat.Specular = D3DXCOLOR(1, 1, 1, clr.a);       // shine from lights
    mat.Power    = 100; // 0 if it shouldn't be shiny

    #elif GRAPHICS_API == OPENGL
    nVertices  = vertexBufferSize/vertexSize;
    nv         = NULL;
    tc         = NULL;
    tex        = 0u;
    // make a shiny material of the specified color
    red   = clr.r;
    green = clr.g;
    blue  = clr.b;
    alpha = clr.a;
    power = 100;  // 0 if it shouldn't be shiny
    #endif

    isOpaque = clr.a == 1.f;
	add(devTex);
}

// setup creates and populates the vertex buffer
//
void Graphic::setup(const IObject* object) {

    #if GRAPHICS_API == DIRECT3D
    // create the vertex buffer
    if (FAILED(d3dd->CreateVertexBuffer(vertexBufferSize, 0, fvf, 
	 D3DPOOL_DEFAULT, &vb, NULL))) {
         error("Graphic::10 Couldn\'t create the vertex buffer");
        vb = NULL;
    }
	// create the index buffer
    else if (FAILED(d3dd->CreateIndexBuffer(indexBufferSize, 0,
	 D3DFMT_INDEX16, D3DPOOL_DEFAULT, &ib, NULL))) {
        error("Graphic::11 Couldn't create the index buffer");
        ib = NULL;
    }

    // populate the newly created Vertex Buffer
    if (vb) {
        void* pv;
        if (SUCCEEDED(vb->Lock(0, vertexBufferSize, (void**)&pv, 0)))
            object->populateVB(pv);
        vb->Unlock();
    }

    // populate the newly created Index Buffer
    if (ib) {
        void* iv;
        if (SUCCEEDED(ib->Lock(0, indexBufferSize, (void**)&iv, 0)))
            object->populateIB(iv);
        ib->Unlock();
    }

    #elif GRAPHICS_API == OPENGL
    // create new vertex normal and texture arrays
    //
    if (!(vb = new (std::nothrow) GLfloat[nVertices][3]) ||
     !(nv = new (std::nothrow) GLfloat[nVertices][3]) ||
	 !(tc = new (std::nothrow) GLfloat[nVertices][2])) {
        error("Couldn't create the vertex, normal or texture arrays");
    }

    // populate the newly created vertex, normal and texture buffers
    //
    if (vb && nv && tc) {
		float (*vertex)[8];
		vertex = new float[nVertices][8];
		object->populateVB(vertex);
        for (int i = 0; i < nVertices; i++) {
            vb[i][0] = vertex[i][0];
            vb[i][1] = vertex[i][1];
            vb[i][2] = vertex[i][2];
            nv[i][0] = vertex[i][3];
            nv[i][1] = vertex[i][4];
            nv[i][2] = vertex[i][5];
            tc[i][0] = vertex[i][6];
            tc[i][1] = vertex[i][7];
        }
		delete [] vertex;
    }
    #endif
}

// add adds a texture level to the graphic representation
//
void Graphic::add(IDeviceTexture* devTex) {

	if (devTex) {
		IDeviceTexture** dTexture = new IDeviceTexture*[nTextures + 1];
		for (int i = 0; i < nTextures; i++)
			dTexture[i] = deviceTexture[i];
		dTexture[nTextures++] = devTex;
		if (deviceTexture) delete [] deviceTexture;
		deviceTexture = dTexture;
	}
}

// draw draws the set of graphics primitives using the object's world
// transformation and reflected colour, along with the object's texture
//
void Graphic::draw(const IObject* object) {

    // if just created, setup first
    if (!vb || !ib) setup(object);

    #if GRAPHICS_API == DIRECT3D
    if (vb) {
        d3dd->SetTransform(D3DTS_WORLD, (D3DXMATRIX*)(&object->world()));
		d3dd->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, antiAliasingOn);
        d3dd->SetStreamSource(0, vb, 0, vertexSize);
        d3dd->SetMaterial(&mat);
		for (int i = 0; i < nTextures; i++) {
			if (deviceTexture[i])
				deviceTexture[i]->attach(i);
		}
		d3dd->SetIndices(ib);
        d3dd->DrawIndexedPrimitive(type, 0, 0, nVertices, 0, nPrimitives);
		for (int i = 0; i < nTextures; i++) {
			if (deviceTexture[i])
				deviceTexture[i]->detach(i);
		}
		Matrix temp;temp.isIdentity();
		d3dd->SetTransform(D3DTS_WORLD, (D3DXMATRIX*)(&temp)); //particle implementation

		/*
		1. 오브젝트를 옮기는 과정에서 D3DTS_WORLD가 변형되었을때, 위의 항등행렬이 하는 것이, D3DTS_WORLD를 원점(0,0,0)으로 되돌리는 것인가?

		2. 발사 지점이 오브젝트의 포지션을 받아야 하는가? WORLD를 받아야 하는가?
		*/

    }

    #elif GRAPHICS_API == OPENGL
    // openGL stores matrices in column major order
    // for the elements to be correctly arranged, row major the
    // world transformation should be stored as the transpose of
    // the required matrix assuming row major ordering
    glLoadMatrixf((GLfloat*)(&object->world()));

    // set the material colour and shininess
    glColor4f(red, green, blue, alpha);
    glMateriali(GL_FRONT, GL_SHININESS, power);

    // normal and vertex arrays - order matters here
	if (deviceTexture)
		deviceTexture->attach(0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, tc);
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, nv);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vb);

    glDrawArrays(type, 0, nVertices);
	if (deviceTexture)
		deviceTexture->detach(0);
    #endif
}

// suspend detaches the pointer to the vertex buffer
//
void Graphic::suspend() {

    #if GRAPHICS_API == DIRECT3D
    // release the interface to the vertex buffer
    if (vb) {
        vb->Release();
        vb = NULL;
    }
    // release the interface to the index buffer
    if (ib) {
        ib->Release();
        ib = NULL;
    }

    #elif GRAPHICS_API == OPENGL
    if (nv) {
        delete [] nv;
        nv = NULL;
    }
    if (vb) {
        delete [] vb;
        vb = NULL;
    }
    if (tc) {
        delete [] tc;
        tc = NULL;
    }
    #endif
}

// destructor releases the graphic representation 
//
Graphic::~Graphic() {

    suspend();
	if (deviceTexture)
		delete [] deviceTexture;
}

//-------------------------------- Mesh ----------------------------------
//
// Mesh manages the graphics primitives that represent an object on the
// graphics card in the form of a mesh
//
IGraphic* CreateMesh(int noSubsets, int noPrimitives, int noVertices, 
 Colour* clr, IDeviceTexture** devTex, bool antiAlias) {

	return new Mesh(noSubsets, noPrimitives, noVertices, clr, devTex, 
	 antiAlias);
}

IGraphic* CreateMesh(Shape shape, float* d, int* p, Colour clr, 
 bool antiAlias) {

	return new Mesh(shape, d, p, clr, antiAlias);
}

IGraphic* CreateMesh(Shape shape, const char* filename, bool antiAlias) {

	return new Mesh(shape, filename, antiAlias);
}

LPDIRECT3DDEVICE9 Mesh::d3dd = NULL;
unsigned Mesh::fvf = NULL;

// constructor stores the reflective colours and addresses of the device
// textures for each subset of the mesh
//
Mesh::Mesh(Shape shape, float* d, int* p, Colour clr, bool antiAlias) : 
 nSubsets(1), antiAliasingOn(antiAlias) {

	switch(shape) {
		case SPHERE:
			dimension[0] = d[0];
			partition[0] = p[0];
			partition[1] = p[1];
			break;
		case CYLINDER:
			dimension[0] = d[0];
			dimension[1] = d[1];
			dimension[2] = d[2];
			partition[0] = p[0];
			partition[1] = p[1];
			break;
		case TORUS:
			dimension[0] = d[0];
			dimension[1] = d[1];
			partition[0] = p[0];
			partition[1] = p[1];
			break;
		case TEAPOT:
			break;
	}
	this->shape = shape;

    mesh = NULL;
    mat  = new D3DMATERIAL9[nSubsets];
    tex  = new IDeviceTexture*[nSubsets];

    // make a shiny material of the specified color
    ZeroMemory(&mat[0], sizeof D3DMATERIAL9);
    mat[0].Ambient  = D3DXCOLOR(clr.r*0.7f, clr.g*0.7f, clr.b*0.7f, 
	 clr.a);
    mat[0].Diffuse  = D3DXCOLOR(clr.r, clr.g, clr.b, clr.a); // reflected
    mat[0].Specular = D3DXCOLOR(1.f, 1.f, 1.f, clr.a);  // shine 
    mat[0].Power    = 100; // 0 if it shouldn't be shiny
    // texture objects
    tex[0] = NULL;
}

Mesh::Mesh(Shape shape, const char* filename, bool antiAlias) : 
 nSubsets(0), file(filename), antiAliasingOn(antiAlias) {

	this->shape = shape;

    mesh = NULL;
    mat  = NULL;
    tex  = NULL;
}

Mesh::Mesh(int noSubsets, int noPrimitives, int noVertices, Colour* clr, 
 IDeviceTexture** devTex, bool antiAlias) : nSubsets(noSubsets), 
 nPrimitives(noPrimitives), nVertices(noVertices), shape(CUSTOM), 
 antiAliasingOn(antiAlias) {

     mesh = NULL;
     mat  = new D3DMATERIAL9[nSubsets];
     tex  = new IDeviceTexture*[nSubsets];

     for (int i = 0; i < nSubsets; i++) {
         // make a shiny material of the specified color
         ZeroMemory(&mat[i], sizeof D3DMATERIAL9);
         mat[i].Ambient  = D3DXCOLOR(clr[i].r*0.7f, clr[i].g*0.7f,
          clr[i].b*0.7f, clr[i].a);
         mat[i].Diffuse  = D3DXCOLOR(clr[i].r, clr[i].g, clr[i].b,
          clr[i].a); // reflected
         mat[i].Specular = D3DXCOLOR(1.f, 1.f, 1.f, clr[i].a);  // shine 
         mat[i].Power    = 100; // 0 if it shouldn't be shiny
         // texture objects
         tex[i] = devTex[i];
     }
}

// setup creates and populates the mesh
//
void Mesh::setup(const IObject* object) {

    float radius, radius1, radius2, length, innerRadius, outerRadius;
    unsigned slices, stacks, sides, rings;
    // first check if the shape type is a stock type, if so, create it
    //
    switch (shape) {
      case TEAPOT:
        if (FAILED(D3DXCreateTeapot(d3dd, &mesh, 0)))
			error("Mesh 20::Failed to create the mesh for a teapot");
        break;

      case SPHERE:
        radius = dimension[0];
        slices = partition[0];
        stacks = partition[1];
        if (FAILED(D3DXCreateSphere(d3dd, radius, slices, stacks, &mesh, 
         0)))
		    error("Mesh 21::Failed to create the mesh for a sphere");
        break;

      case CYLINDER:
        radius1 = dimension[0];
        radius2 = dimension[1];
        length  = dimension[2];
        slices  = partition[0];
        stacks  = partition[1];
        if (FAILED(D3DXCreateCylinder(d3dd, radius1, radius2, length, 
         slices, stacks, &mesh, 0)))
		    error("Mesh 22::Failed to create the mesh for a cylinder");
        break;

      case TORUS:
        innerRadius = dimension[0];
        outerRadius = dimension[1];
        sides = partition[0];
        rings = partition[1];
        if (FAILED(D3DXCreateTorus(d3dd, innerRadius, outerRadius,
         sides, rings, &mesh, 0)))
			 error("Mesh 23::Failed to create the mesh for a torus");
        break;

      case X_FILE:
		LPD3DXBUFFER mtrl;
        if (FAILED(D3DXLoadMeshFromX(file, 0, d3dd, NULL, &mtrl, 0, 
		 (DWORD*)&nSubsets, &mesh)))
            error("Mesh 24::Failed to create the mesh for the .x file");
        else {
            if (!tex) {
                D3DXMATERIAL* matl;
                mat  = new D3DMATERIAL9[nSubsets];
                tex  = new IDeviceTexture*[nSubsets];
                matl = (D3DXMATERIAL*)mtrl->GetBufferPointer();
                for (int i = 0; i < nSubsets; i++) {
                    ZeroMemory(&mat[i], sizeof D3DMATERIAL9);
                    mat[i].Ambient.r  = matl[i].MatD3D.Diffuse.r * 0.7f; 
                    mat[i].Ambient.g  = matl[i].MatD3D.Diffuse.g * 0.7f;
                    mat[i].Ambient.b  = matl[i].MatD3D.Diffuse.b * 0.7f;
                    mat[i].Ambient.a  = matl[i].MatD3D.Diffuse.a;
                    mat[i].Diffuse    = matl[i].MatD3D.Diffuse;
                    mat[i].Specular   = matl[i].MatD3D.Specular;
                    mat[i].Power      = matl[i].MatD3D.Power;
                    const char* tFile = matl[i].pTextureFilename;
					if (tFile)
						tex[i] = CreateTexture(tFile, 0, 0)->deviceTexture();
					else	
						tex[i] = NULL;
                }
            }
            mtrl->Release();
        }
        break;

      case CUSTOM:
		// Create an empty mesh
		//
		if (FAILED(D3DXCreateMeshFVF(nPrimitives, nVertices, 0, fvf, d3dd, 
		 &mesh))) {
			 error("Mesh 25::Couldn't create the empty mesh");
			mesh = NULL;
		}

		// populate the newly created Vertex Buffer
		if (mesh) {
			void* pv;
			if (SUCCEEDED(mesh->LockVertexBuffer(0, (void**)&pv))) {
				object->populateVB(pv);
				mesh->UnlockVertexBuffer();
			}
		}

		// populate the newly created Index Buffer
		if (mesh) {
			void* pi;
			if (SUCCEEDED(mesh->LockIndexBuffer(0, (void**)&pi))) {
				object->populateIB(pi);
				mesh->UnlockIndexBuffer();
			}
		}

		// Populate the newly created Attribute Buffer
		//
		if (mesh) {
			void *pa;
			if (SUCCEEDED(mesh->LockAttributeBuffer(0, (DWORD**)&pa))) {
				object->populateAB(pa);
				mesh->UnlockAttributeBuffer();
			}
		}
		break;
	  }
}

// add adds a texture level to the graphic representation
//
void Mesh::add(IDeviceTexture* devTex) {

	//if (devTex) {
	//	IDeviceTexture** dTexture = new IDeviceTexture*[nTextures + 1];
	//	for (int i = 0; i < nTextures; i++)
	//		dTexture[i] = deviceTexture[i];
	//	dTexture[nTextures] = devTex;
	//	if (deviceTexture) delete [] deviceTexture;
	//	deviceTexture = dTexture;
	//}
}

// draw draws the set of graphics primitives using the object's world
// transformation and reflected colour, along with the object's texture
//
void Mesh::draw(const IObject* object) {

    // if just created, setup first
    if (!mesh) setup(object);

    if (mesh) {
        d3dd->SetTransform(D3DTS_WORLD, (D3DXMATRIX*)(&object->world()));
		d3dd->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, antiAliasingOn);
		// draw each subset
		for (int i = 0; i < nSubsets; i++) {
			d3dd->SetMaterial(&mat[i]);
			if (tex[i])
				tex[i]->attach(0);
			mesh->DrawSubset(i);
			if (tex[i])
				tex[i]->detach(0);
		}
		// reset to default format
		d3dd->SetFVF(fvf);
    }
}

// suspend detaches the pointer to the vertex buffer
//
void Mesh::suspend() {

    if (mesh) {
        mesh->Release();
        mesh = NULL;
    }
}

// destructor releases the graphic representation 
//
Mesh::~Mesh() {

    suspend();
	if (mat)
		delete [] mat;
	if (tex)
		delete [] tex;
}

//-------------------------------- DeviceTexture -------------------------
//
// DeviceTexture represents a texture on the graphics card
//
IDeviceTexture* CreateDeviceTexture(const char* file, unsigned flags,
 Colour brdrClr) {

	return new DeviceTexture(file, flags, brdrClr);
}

int DeviceTexture::width  = WND_WIDTH;
int DeviceTexture::height = WND_HEIGHT;
#if GRAPHICS_API == DIRECT3D
LPDIRECT3DDEVICE9 DeviceTexture::d3dd   = NULL;
LPD3DXSPRITE DeviceTexture::sprite = NULL;
int DeviceTexture::maxStages = 0;
#elif GRAPHICS_API == OPENGL
#endif

// constructor initializes the instance variables 
//
DeviceTexture::DeviceTexture(const char* file, unsigned flags, 
 Colour brdrClr) : filter(flags) {

    if (!filter) filter = OB_FLAGS;
	borderColor = D3DXCOLOR(brdrClr.r, brdrClr.g, brdrClr.b, brdrClr.a);

	filename = new char[strlen(file) + 1];
	strcopy(filename, file, strlen(file));

    #if GRAPHICS_API == DIRECT3D
    tex = NULL;

    #elif GRAPHICS_API == OPENGL
    tex = 0u;
    #endif
}

// setup creates the device texture from the texture file
//
void DeviceTexture::setup() {

    #if GRAPHICS_API == DIRECT3D
    // create the texture COM object from the object's texture file
    //
    if (filename && FAILED(D3DXCreateTextureFromFile(d3dd,
	 filename, &tex))) {
         error("DeviceTexture::11 Couldn\'t load texture");
        tex = NULL;
    }

    #elif GRAPHICS_API == OPENGL
    if (file_) {
        unsigned char* texdata;
        int texw, texh;
        if (texdata = createTexData(filename, texw, texh)) {
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, texw, texh,
             GL_BGR_EXT, GL_UNSIGNED_BYTE, texdata);
            delete [] texdata;
        }
        else {
            char str[81];
            wsprintf(str, "DeviceTexture::11 Couldn\'t load texture",
			 file_);
            error(str);
        }
    }
    #endif
}

// attach attaches the device texture to sampling stage i
//
void DeviceTexture::attach(int i) {

	if (!tex && filename) setup();

    #if GRAPHICS_API == DIRECT3D
    if (tex && i < maxStages) {
        d3dd->SetTexture(i, tex);
		setSamplerState(i);
		if (i) {
			// use the same texture coordinates as for state 0
			d3dd->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, 0);
			// blend stage i with stage i-1 by modulation
			d3dd->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_MODULATE);
			d3dd->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			d3dd->SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CURRENT);
			d3dd->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			d3dd->SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			d3dd->SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
		}
    }

    #elif GRAPHICS_API == OPENGL
    if (tex) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
    }
    else {
        glDisable(GL_TEXTURE_2D);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    #endif
}

// setSamplerState sets the sampling state according to the filtering
// flags
//
void DeviceTexture::setSamplerState(int i) {

    #if GRAPHICS_API == DIRECT3D
	// minification
    if (filter & TEX_MIN_POINT)
        d3dd->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    else if (filter & TEX_MIN_LINEAR)
        d3dd->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    else if (filter & TEX_MIN_ANISOTROPIC)
        d3dd->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC); 

	// magnification
    if (filter & TEX_MAG_POINT)
        d3dd->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    else if (filter & TEX_MAG_LINEAR)
        d3dd->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    else if (filter & TEX_MAG_ANISOTROPIC)
        d3dd->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);

	// mipmapping
    if (filter & TEX_MIP_MAPPING)
        d3dd->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
	else
        d3dd->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

    // tiling
    //
    if (filter & TEX_TILE_BORDER_U)
        d3dd->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
    else if (filter & TEX_TILE_MIRROR_U)
        d3dd->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR); 
    else if (filter & TEX_TILE_CLAMP_U)
        d3dd->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP); 
    else
        d3dd->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    if (filter & TEX_TILE_BORDER_V)
        d3dd->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
    else if (filter & TEX_TILE_MIRROR_V)
        d3dd->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR); 
    else if (filter & TEX_TILE_CLAMP_V)
        d3dd->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP); 
    else
        d3dd->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    if (filter & TEX_TILE_BORDER_U || filter & TEX_TILE_BORDER_V)
        d3dd->SetSamplerState(0, D3DSAMP_BORDERCOLOR, borderColor);
    #endif
}

// detach detaches the device texture from sampling stage i
//
void DeviceTexture::detach(int i) {

    #if GRAPHICS_API == DIRECT3D
	if (tex && i < maxStages) {
        d3dd->SetTexture(i, NULL);
		if (i) {
			d3dd->SetTextureStageState(i, D3DTSS_COLOROP, D3DTOP_DISABLE);
			d3dd->SetTextureStageState(i, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		}
	}

    #elif GRAPHICS_API == OPENGL
    if (tex) {
        glDisable(GL_TEXTURE_2D);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    #endif
}

// draw copies the visible portion of the texture directly onto
// the background plane of the display device
//
void DeviceTexture::draw(int topLeftX, int topLeftY, int bottomRightX,
 int bottomRightY) {

	if (!tex && filename) setup();

    #if GRAPHICS_API == DIRECT3D
    if (tex) {
		RECT rect;
		SetRect(&rect, topLeftX, topLeftY, bottomRightX, bottomRightY); 
		// draw the texture on the background plane
		D3DXVECTOR3 position(0, 0, 1);
		// scale the portion of the texture to fit the entire screen
        float sx = (float) width  / (bottomRightX - topLeftX);
        float sy = (float) height / (bottomRightY - topLeftY - 100);
        D3DXMATRIX T(sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
        sprite->SetTransform(&T);
        sprite->Draw(tex, &rect, NULL, &position, 0xFFFFFFFF);
		// revert to no scaling
        T._11 = 1;
		T._22 = 1;
        sprite->SetTransform(&T);
    }

    #elif GRAPHICS_API == OPENGL
    #endif
}

// suspend detaches the pointer to the device texture on the graphics card
//
void DeviceTexture::suspend() {

    #if GRAPHICS_API == DIRECT3D
    // release the interface to the texture COM object
    if (tex) {
        tex->Release();
        tex = NULL;
    }

    #elif GRAPHICS_API == OPENGL
    if (tex) {
        glDeleteTextures(1, &tex);
        tex = 0;
    }
    #endif
}

// destructor suspends the device texture 
//
DeviceTexture::~DeviceTexture() {

    suspend();
	if (filename)
		delete [] filename;
}

#if GRAPHICS_API == DIRECT3D
#elif GRAPHICS_API == OPENGL
// Allocates, fills and returns a block of memory with pixel data
// from a 24-bit BMP file. Returns 0 if it can't. The pixel width
// and height are returned through "width" and "height".
//
// Important: Use delete [] on the returned address when done!
//
unsigned char *createTexData(const char *file, int &width, int &height) {

    unsigned char *rc = 0, c[2] = { 0, 0 };
    FILE *fp;
    unsigned int w = 0, h = 0, offs = 0, comp = 0, size = 0;
    unsigned short bpp = 0;

    if (file)
        if (fp = fopen(file, "rb")) {
            // Examine the BMP file header so we can be sure it
            // is the kind of data we are expecting.
            fread(c, 2, 1, fp);      // should be 'B' 'M'
            fseek(fp, 8, SEEK_CUR);
            fread(&offs, 4, 1, fp);  // offset of bitmap data
            fseek(fp, 4, SEEK_CUR);
            fread(&w, 4, 1, fp);     // pixel width of image
            fread(&h, 4, 1, fp);     // pixel height of image
            fseek(fp, 2, SEEK_CUR);
            fread(&bpp, 2, 1, fp);   // bits per pixel (should be 24)
            fread(&comp, 4, 1, fp);  // compression scheme (should be 0)

            // Fortunately, the order that pixel data is stored in a BMP
            // is compatible with OpenGL, so we can just read it in one
            // big block.
            //
            if (c[0] == 'B' && c[1] == 'M' && offs && w && h && bpp == 24
             && comp == 0 && (rc = new unsigned char[size = w * h * 3])) {
                fseek(fp, offs, SEEK_SET);
                fread(rc, size, 1, fp);
                width = w;
                height = h;
            }
        }
        if (!rc)
            error("Error reading texture file");
    return rc;
}
#endif

//-------------------------------- Font --------------------------------------
//
IFont* CreateFont_(IText* text, unsigned int flags) {

	return new Font(text, flags);
}

int Font::width    = WND_WIDTH;
int Font::height   = WND_HEIGHT;
#if GRAPHICS_API == DIRECT3D
LPDIRECT3DDEVICE9 Font::d3dd = NULL;
LPD3DXSPRITE Font::sprite = NULL; 
#elif GRAPHICS_API == OPENGL
HGLRC Font::hrc = NULL;
HDC Font::hdc = NULL;
#endif

// constructor selects the graphics api text alignment flags and
// initializes the instance variables
//
Font::Font(const IText* t, unsigned f) : text(t) {

	// default
	if (!f) f = TEXT_FLAGS_DEFAULT;

    #if GRAPHICS_API == DIRECT3D
    d3dfont = NULL;
    flags = 0;
    if (f & TEXT_LEFT)        flags |= DT_LEFT;
    else if (f & TEXT_RIGHT)  flags |= DT_RIGHT;
    else if (f & TEXT_CENTER) flags |= DT_CENTER;
    if (f & TEXT_TOP)         flags |= DT_TOP;
    else if (f & TEXT_BOTTOM) flags |= DT_BOTTOM;
    else if (f & TEXT_MIDDLE) flags |= DT_VCENTER;
    #elif GRAPHICS_API == OPENGL
	isSetup = false;
	// text alignment not implemented in OpenGL
    #endif
}

// setup creates the font on the graphics card 
//
void Font::setup() {

    #if GRAPHICS_API == DIRECT3D
    // create the font COM object on the display device
    if (FAILED(D3DXCreateFont(d3dd, 15, 0, FW_NORMAL, 0, 0,
     DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
     FF_DONTCARE | DEFAULT_PITCH, "ARIAL", &d3dfont)))
        error("Font::10 Unable to create the font object");

    #elif GRAPHICS_API == OPENGL
	// setup a font object for displaying text
    SelectObject(hdc, GetStockObject(SYSTEM_FONT));
    wglUseFontBitmaps(hdc, 0, 255, FONT_REF); // FONT_REF see draw
	isSetup = true;
    #endif
}

// draw defines the rectangle for the font and draws str within 
// the rectangle using color TEXT_R, G, B
//
void Font::draw(const char* str) {

    #if GRAPHICS_API == DIRECT3D
    if (!d3dfont) setup();
	if (sprite && d3dfont && str) {
		// creates the drawing rectangle in screen coordinates 
		RECT rect;
		SetRect(&rect, (int)(width * text->topLeftX()), 
		 (int)(height * text->topLeftY()),
		 (int)(width  * text->bottomRightX()), 
		 (int)(height * text->bottomRightY()));
        // draws the text string
		d3dfont->DrawText(sprite, str, -1, &rect, flags,
         D3DCOLOR_ARGB(TEXT_A, TEXT_R, TEXT_G, TEXT_B));
	}
    #elif GRAPHICS_API == OPENGL
	if (!isSetup) setup();
	if (str) {
		// position the text
		int x = (int)(width * text->topLeftX());
		int y = (int)(height * (1 - text->bottomRightY()));
		glRasterPos2i(x, y);
		glListBase(FONT_REF);
		glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
	}
    #endif
}

// suspend detaches the reference to the graphics card before loss of 
// focus
//
void Font::suspend() {

    #if GRAPHICS_API == DIRECT3D
    // release references to the display device
    if (d3dfont)
        d3dfont->OnLostDevice();
    #elif GRAPHICS_API == OPENGL
	isSetup = false;
	#endif
}

// restore re-acquires the reference to the graphics card
//
bool Font::restore() {

    bool rc = false;

    #if GRAPHICS_API == DIRECT3D
    // re-acquire the reference to the graphics card
    if (d3dfont)
        rc = d3dfont->OnResetDevice() == D3D_OK;
    #endif

    return rc;
}

// release releases the pointer to the font COM object
//
void Font::release() {

	suspend();

    #if GRAPHICS_API == DIRECT3D
    if (d3dfont) {
        d3dfont->Release();
        d3dfont = NULL;
    }
    #endif
}

// destructor releases the font object
//
Font::~Font() {

	release();
}

