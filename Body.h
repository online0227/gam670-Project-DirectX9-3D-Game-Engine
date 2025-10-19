#ifndef _BODY_H_
#define _BODY_H_

/* Header for the Body Module
 *
 * Body.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "Frame.h" // for AnimatedFrame
#include "math.h"  // for Vector

//-------------------------------- Body ----------------------------------
//
// A Body is a closed shape attached to a AnimatedFrame that describes the 
// impenetrable region around the origin of the AnimatedFrame.  
//
// This closed shape is used to detect collision with other Bodies
//
class Body : public AnimatedFrame {

	// level 1 bounds on the body - sphere or cylinder
	bool   hasSphere;   // has a bounding sphere?
	bool   hasCylinder; // has a bounding cylinder?
	float  radius;      // radius of the bounding sphere/cylinder
	float  height;      // height of the bounding cylinder
	Vector centroid;    // geometric centre of body wrt to AnimatedFrame

	// level 2 bounds on the body - box
	bool   hasBox;      // has a bounding box?
	Vector nx;          // normal to the y-z plane
	Vector ny;          // normal to the x-z plane
	Vector nz;          // normal to the x-y plane
	float  sx;          // half side length in x direction
	float  sy;          // half side length in y direction
	float  sz;          // half side length in z direction
	Vector vertex[8];   // list of corner vertices - local space

	// collision detection with respect to the current body
	Vector initial;     // starting position of the moving body
	Vector projected;   // end position of the moving body
	Vector correction;  // correction to moving contact point
	Vector begin;       // start of the moving contact point's path
	Vector end;         // end of the moving contact point's path
	Vector path;        // direction of the moving contact point
	Vector normalPath;  // normalized direction of moving contact point
	Matrix toRef;       // transformation to reference body's frame
	float  lambda;      // fraction of time step to collision point
	Vector n;           // normal to collision surface
	Vector p;           // point of collision

	bool collidesWith(const Vector& na, float sa, const Vector& nb, 
	 float sb, const Vector& nc, float sc);
	bool intersects(const Vector& na, float sa, const Vector& nb, 
	 float sb, const Vector& nc, float sc);

protected:
	void setBoundingSphere(float radius, const Vector& c);
	void setBoundingCylinder(float radius, float height, const Vector& c,
	 const Vector& y);
	void setBoundingBox(float minx, float miny, float minz, float maxx, 
	 float maxy, float maxz);

public:
	Body();
	bool  boxCollision(Body* movingBody, float& dt, Vector& contact, 
	 Vector& normal);
	float boundingRadius()      const { return radius; }
	bool  hasBoundingSphere()   const { return hasSphere; }
	bool  hasBoundingCylinder() const { return hasCylinder; }
	bool  hasBoundingBox()      const { return hasBox; }
	//particle implementation
	Vector returnBoundingMin() { return vertex[0]; }
	Vector returnBoungingMax() { return vertex[6]; }
};

bool sphereCollision(const Body* body_i, const Body* body_j, float& dt, 
 Vector& normal, Vector& contact);

#endif
