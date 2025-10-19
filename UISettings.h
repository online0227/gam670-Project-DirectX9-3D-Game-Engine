#ifndef _UI_SETTINGS_H_
#define _UI_SETTINGS_H_

/* Header for User Interface Settings
 *
 * UISettings.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "Settings.h" // for window resolutions parameters

// shared with dialog resource (follows windows conventions)
//
#define IDD_DLG        101
#define IDC_DISPLAY    102
#define IDC_RESOLUTION 103
#define IDC_GO         104
#define IDC_CONTROLLER 105
#define IDC_TRG        106
#define IDC_YRV        107
#define IDC_ZON        108
#define IDC_NOFF       109
#define IDC_FF         110

// windowing parameters
//
#define CLASS_NAME         "GAM670_DPS905"
#define RUN_IN_WINDOW_DESC "Run in a window"

// hud, text parameters
//
// hud background
#define HUD_SPRITE_X     0.55f
#define HUD_SPRITE_Y     0.01f
#define HUD_SPRITE_W     0.43f
#define HUD_SPRITE_H     0.43f
#define HUD_SPRITE_IMAGE "spriteBackground.bmp"
#define HUD_MOVE         2000.f
// text limits within the hud
#define TL_MAX  0.99f
#define TL_MIN  0.01f
#define R_MIN   0.2f

#endif
