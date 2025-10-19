#ifndef _DEVICE_SETTINGS_H_
#define _DEVICE_SETTINGS_H_

/* Header for Device Component Configuration
 *
 * DeviceSettings.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "Settings.h" // for keypress codes

// available api's
#define DIRECT3D   1
#define OPENGL     2

// select the api here and change the linker input accordingly
//    Direct3D - d3d9.lib d3dx9.lib 
//    OpenGL   - opengl32.lib glu32.lib
//
#define GRAPHICS_API DIRECT3D
//#define GRAPHICS_API OPENGL

// windowing parameters
#if GRAPHICS_API == DIRECT3D
#define WND_EXSTYLE   0
#define WND_STYLE     WS_OVERLAPPEDWINDOW
#define D3D_NO_DOC_FORMATS 6
#define D3D_DOC_FORMATS { D3DFMT_X8R8G8B8, D3DFMT_A8R8G8B8,\
     D3DFMT_A2R10G10B10, D3DFMT_X1R5G5B5, D3DFMT_A1R5G5B5,\
     D3DFMT_R5G6B5 }
#define D3D_FORMAT_WINDOW 5
// short descriptions for each format. In practice, you
// just might want to call them either "16" or "32" (so
// the user is deciding between 16 and 32 bit colour)
// but we are making them all different here so that we
// can learn what modes the hardware we have makes available.
#define D3D_FORMAT_DESC { "X32", "A32", "A30+2", "X15", "A16", "16" }
#elif GRAPHICS_API == OPENGL
#define WND_EXSTYLE WS_EX_APPWINDOW | WS_EX_WINDOWEDGE
#define WND_STYLE   WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
#endif
// background colour
#if GRAPHICS_API == DIRECT3D
#define BGROUND_R 200
#define BGROUND_G 200
#define BGROUND_B 200
#elif GRAPHICS_API == OPENGL
#define BGROUND_R .4f
#define BGROUND_G .4f
#define BGROUND_B .4f
#endif

// hud background colour
//
#define SPRITE_R 255
#define SPRITE_G 255
#define SPRITE_B 255
#define SPRITE_A  32

// projection transformation
//
// frustum properties
#define NEAR_CLIPPING 1.0f
#define FAR_CLIPPING  2000.f
#if GRAPHICS_API == DIRECT3D
#define FIELD_OF_VIEW 0.9f // radians for Direct3D
#elif GRAPHICS_API == OPENGL
#define FIELD_OF_VIEW 51.57f // degrees for OpenGL
#endif

// text display colour
//
#if GRAPHICS_API == DIRECT3D
#define TEXT_R 255
#define TEXT_G 255
#define TEXT_B 255
#define TEXT_A 255
#elif GRAPHICS_API == OPENGL
#define TEXT_R 1.f
#define TEXT_G 1.f
#define TEXT_B 1.f
#define TEXT_A 1.f
#endif

// font reference
//
#if GRAPHICS_API == DIRECT3D
#elif GRAPHICS_API == OPENGL
#define FONT_REF 1000 // for openGL text display
#endif

// input device parameters
//
#define SAMPLE_BUFFER_SIZE    30

// sound parameters
//
#define INTERIOR_CONE 0.67f
#define MAX_SOUND_BUFFER_SIZE 500000
#define SOUND_DISTANCE_FACTOR 0.5f     // metres per unit distance
#define ATTENUATION_DISTANCE_FACTOR 30

#endif
