#ifndef _UTILITIES_H_
#define _UTILITIES_H_

/* Header for the Utility Functions
 *
 * Utilities.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <d3dx9.h>

	// particle implementation
	// Constants
	//

	const float INFINITY = FLT_MAX;
	const float EPSILON  = 0.001f;


void error(const char* msg, HWND hwnd = NULL);
char* strcatenate(char* dest, const char* src, int sizeDest);
char* strcopy(char* dest, const char* src, int sizeDest);

	// particle implementation
DWORD FtoDw(float f);

	//
	// Randomness
	//

	// Desc: Return random float in [lowBound, highBound] interval.
	float GetRandomFloat(float lowBound, float highBound);
	

	// Desc: Returns a random vector in the bounds specified by min and max.
	void GetRandomVector(
		D3DXVECTOR3* out,
		D3DXVECTOR3* min,
		D3DXVECTOR3* max);


	// Cleanup
	//
	template<class T> void Release(T t)
	{
		if( t )
		{
			t->Release();
			t = 0;
		}
	}
		
	template<class T> void Delete(T t)
	{
		if( t )
		{
			delete t;
			t = 0;
		}
	}

	//
	// Bounding Objects
	//

	struct BoundingBox
	{
		BoundingBox();

		bool isPointInside(D3DXVECTOR3& p);

		D3DXVECTOR3 _min;
		D3DXVECTOR3 _max;
	};

	struct BoundingSphere
	{
		BoundingSphere();

		D3DXVECTOR3 _center;
		float       _radius;
	};



#endif