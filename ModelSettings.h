#ifndef _MODEL_SETTINGS_H_
#define _MODEL_SETTINGS_H_

/* Header with the Model Component Settings
 *
 * ModelSettings.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "Settings.h" // for ZAXIS_DIRECTION and the keypress codes

// background file
//
#define BACKGROUND "space.jpg"
#define BACKGROUND_WIDTH          1280
#define BACKGROUND_HEIGHT         1280
#define BACKGROUND_TOP_LEFT_XMAX   500
#define BACKGROUND_SPEED         0.0f

// lighting parameters
//
// initial light settings
#define LIGHT0 TRUE
#define LIGHT1 FALSE
#define LIGHT2 TRUE
#define AMBIENT_LIGHT 0.2f

// camera parameters
//
// initial position and orientation
#define CAMERA_X    0
#define CAMERA_Y    5
#define CAMERA_H_X  0
#define CAMERA_H_Y  0
#define CAMERA_U_X  0
#define CAMERA_U_Y  1
#define CAMERA_Z  -80 * ZAXIS_DIRECTION
#define CAMERA_H_Z  1 * ZAXIS_DIRECTION
#define CAMERA_U_Z  0 * ZAXIS_DIRECTION
// motion factors
#define ROT_SPEED     0.000833f
#define TILT_SPEED    0.001f
#define FORWARD_SPEED 0.01f
#define UP_SPEED      0.005f
// camera speed control factors
#define CAM_SPEED       0.12f
#define TURNING_RADIUS  120.00f
#define ANG_CAM_SPEED   ( CAM_SPEED / TURNING_RADIUS )
// "Floor" of the world - lowest possible y value
#define FLOOR -50

// scene parameters
//
// object rotation factors
#define ROLL_SPEED 0.001111f
#define SPIN_SPEED 0.000909f
#define CONSTANT_ROLL  0.01f

// sound parameters
//
// initial sound settings
#define SOUND0 true
#define SOUND1 false
#define START_VOLUME 0.7f
#define DELTA_VOLUME 0.001f
#define FREQ_RANGE      5000  // in Hertz (cycles per sec)
#define FREQ_DROP_VELOCITY 8  // rate of fall to idle
#define FREQ_RISE_VELOCITY 4  // rate of rise to top

// input device conversion factors
//
#define MOUSE_DISPLACEMENT_TO_WORLD_ROTATION     80
#define MOUSE_DISPLACEMENT_TO_WORLD_DISPLACEMENT 40
#define MOUSE_ROTATION_TO_WORLD_ROTATION        200
#define MOUSE_BUTTON_SCALE                      400
#define JOY_ROTATION_FACTOR                    0.05f
#define JOY_DISPLACEMENT_FACTOR                0.15f

// Terrain Settings
#define TERRAIN_CHAR_BIT 8
#define HEIGHT_MAP_FLOOR 120
#define CLEARANCE 3
#define BORDER 1
#define TINY_VALUE 1.0e-10f
#define PI 3.14159f
#define MOVE_SPEED 0.1f
#endif
