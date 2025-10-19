#ifndef _SETTINGS_H_
#define _SETTINGS_H_

/* Header for Common Game Component Settings
 *
 * Settings.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

// direction of the z axis
#define INTO_SCREEN 1
#define OUT_OF_SCREEN -1

// select direction here
#define ZAXIS_DIRECTION INTO_SCREEN
//#define ZAXIS_DIRECTION OUT_OF_SCREEN

// keypress latency - time lag between recognized key presses
#define LATENCY 500
#define LATENCY_SHORT 50

// keypress codes
#define UP    1001
#define DOWN  1002
#define PGUP  1003
#define PGDN  1004
#define LEFT  1005
#define RIGHT 1006
#define NUM1  1011
#define NUM2  1012
#define NUM3  1013
#define NUM4  1014
#define NUM5  1015
#define NUM6  1016
#define NUM7  1017
#define NUM8  1018
#define NUM9  1019
#define F1    1021
#define F2    1022
#define F3    1023
#define F4    1024
#define F5    1025
#define F6    1026
#define F7    1027
#define F8    1028
#define F9    1029
#define F10   1030
#define F11   1031
#define F12   1032
#define SPACE 1040
#define ENTER 1041
#define ESCAPE 1042

// window caption
#define WND_NAME "My Game"

// window parameters
//
#define RUN_IN_WINDOW -1
// minimum resolution parameters
#define WND_WIDTH    640      // minimum window width
#define WND_HEIGHT   480      // minimum window height
#define WND_BITS      16      // minimum color depth

// text alignment flags
//
// text alignment flags
#define TEXT_LEFT    1
#define TEXT_RIGHT   2
#define TEXT_CENTER  4
#define TEXT_TOP     8
#define TEXT_BOTTOM 16
#define TEXT_MIDDLE 32
#define TEXT_FLAGS_DEFAULT TEXT_LEFT | TEXT_TOP

// texture filtering flags
//
#define TEX_MIN_POINT        1
#define TEX_MIN_LINEAR       2
#define TEX_MIN_ANISOTROPIC  4
#define TEX_MAG_POINT        8
#define TEX_MAG_LINEAR      16
#define TEX_MAG_ANISOTROPIC 32
#define TEX_TILE_BORDER_U   64
#define TEX_TILE_BORDER_V  128
#define TEX_TILE_CLAMP_U   256
#define TEX_TILE_CLAMP_V   512
#define TEX_TILE_MIRROR_U 1024
#define TEX_TILE_MIRROR_V 2048
#define TEX_MIP_MAPPING   4096
#define OB_FLAGS TEX_MIN_LINEAR | TEX_MAG_LINEAR | TEX_MIP_MAPPING

// collision control
const float NEAR_ZERO = 1E-10f;
const float dtmin = 0.001f;

#endif


