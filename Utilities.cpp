/* Utility Functions Implementation
 *
 * Utilities.cpp
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "Utilities.h"      
#include <fstream>          // for ostream, <<, close()
#include "IConfiguration.h" // for WindowAddress()

//---------------------------------- Utilities -----------------------------
//
// strcatenate catenates src to dest[sizeDest+1] without overflow and
// returns the address of dest[]
// this function is a safe version of strcat
//
char* strcatenate(char* dest, const char* str, int sizeDest) {

    char* rc = dest;
    *(dest + sizeDest) = '\0';
    while (*dest++) sizeDest--;
    dest--;
    while (sizeDest-- && (*dest++ = *str++)) ;

    return rc;
}

// strcopy copies src into dest[sizeDest+1] without overflow and
// returns the address of dest[]
// this function is a safe version of strcpy and a version of
// strncpy that does not require last character padding with a null
// byte
//
char* strcopy(char* dest, const char* src, int sizeDest) {

    *(dest + sizeDest) = '\0';
    while (sizeDest-- && (*dest++ = *src++)) ;

    return dest;
}

// error pops up a Message Box displaying msg and adds the
// message to the log file
//
void error(const char* msg, HWND hDbWnd) {

	HWND hwnd = (HWND)WindowAddress()->window();

    if (hDbWnd) hwnd = hDbWnd;
	if (hwnd) MessageBox(hwnd, msg, "Game Error", MB_OK);

    std::ofstream fp("error.log", std::ios::app);
    if (fp) {
         fp << msg << std::endl;
         fp.close();
    }
}

DWORD FtoDw(float f)
{
	return *((DWORD*)&f);
}


BoundingBox::BoundingBox()
{
	// infinite small 
	_min.x = INFINITY;
	_min.y = INFINITY;
	_min.z = INFINITY;

	_max.x = -INFINITY;
	_max.y = -INFINITY;
	_max.z = -INFINITY;
}

bool BoundingBox::isPointInside(D3DXVECTOR3& p)
{
	if( p.x >= _min.x && p.y >= _min.y && p.z >= _min.z &&
		p.x <= _max.x && p.y <= _max.y && p.z <= _max.z )
	{
		return true;
	}
	else
	{
		return false;
	}
}

BoundingSphere::BoundingSphere()
{
	_radius = 0.0f;
}

float GetRandomFloat(float lowBound, float highBound)
{
	if( lowBound >= highBound ) // bad input
		return lowBound;

	// get random float in [0, 1] interval
	float f = (rand() % 10000) * 0.0001f; 

	// return float in [lowBound, highBound] interval. 
	return (f * (highBound - lowBound)) + lowBound; 
}

void GetRandomVector(
	  D3DXVECTOR3* out,
	  D3DXVECTOR3* min,
	  D3DXVECTOR3* max)
{
	out->x = GetRandomFloat(min->x, max->x);
	out->y = GetRandomFloat(min->y, max->y);
	out->z = GetRandomFloat(min->z, max->z);
}