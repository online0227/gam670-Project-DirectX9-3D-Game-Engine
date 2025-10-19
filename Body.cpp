/* Body Module Implementation
 *
 * Body.cpp
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "Body.h" 
#include "math.h" // for Vector, Matrix

//-------------------------------- Body ----------------------------------
//
// A Body is a closed shape attached to a AnimatedFrame that describes the 
// impenetrable region around the origin of the AnimatedFrame.  
//
// This closed shape is used to detect collision with other bodies
//
Body::Body() : radius(0), hasSphere(false), hasCylinder(false), 
 hasBox(false), sx(0), sy(0), sz(0) {}

// setBoundingSphere defines the bounding sphere for the body
//
void Body::setBoundingSphere(float r, const Vector& c) {

    centroid = c;
	radius   = r;
	hasSphere = true;
}

// setBoundingBox defines the bounding box for the body
//
void Body::setBoundingBox(float minx, float miny, float minz, float maxx, 
 float maxy, float maxz) {

    sx = (maxx - minx) / 2;
    sy = (maxy - miny) / 2;
    sz = (maxz - minz) / 2;
    nx = Vector(1, 0, 0);
    ny = Vector(0, 1, 0);
    nz = Vector(0, 0, 1);
	vertex[0] = Vector(minx, miny, minz);
	vertex[1] = Vector(minx, maxy, minz);
	vertex[2] = Vector(maxx, maxy, minz);
	vertex[3] = Vector(maxx, miny, minz);
	vertex[4] = Vector(minx, miny, maxz);
	vertex[5] = Vector(minx, maxy, maxz);
	vertex[6] = Vector(maxx, maxy, maxz);
	vertex[7] = Vector(maxx, miny, maxz);
	hasBox = true;

    centroid = Vector(minx + sx, miny + sy, minz + sz);
	radius   = (Vector(maxx, maxy, maxz) - centroid).length();
	setBoundingSphere(radius, centroid);
}

// boxCollision determines whether a vertex on *movingBody will collide
// with one of the bounding surfaces of the current body during time 
// step dt
//
// this function assumes no significant rotation of *movingBody during dt
// 
// if a vertex on *movingBody will collide with one of the bounding 
// surfaces of the current body, detectCollision reduces the time step to 
// the sub-step at which *movingBody will collide and returns the point at
// which collision will occur and the normal to the surface of collision 
// in world space
//
bool Body::boxCollision(Body* movingBody, float& dt, Vector& contact, 
 Vector& normal) {

	bool   collision;
	Matrix rot, rotInv;

	// initial position of *moving relative to the current body
	initial    = movingBody->position() - position();
	// projected position of *moving relative to the current body after dt
	projected  = initial + dt * (movingBody->velocity() - velocity());
	// transform the initial and projected positions of *moving
	// to the local reference frame of the current body
	rot        = rotation();
	rotInv     = rot.transpose();
	initial   *= rotInv;
	projected *= rotInv;
	toRef      = movingBody->world() * rotInv;
	path       = projected - initial;
	normalPath = ::normal(path);
	// adjust initial and projected positions for the radius of *moving
	correction = movingBody->boundingRadius() * normalPath;
	begin      = initial + correction;
	end        = projected + correction;
	// assume a collision occurs at end point
	lambda     = 1.0f;
    // check for collision with plane normal to x, y, z axis
	collision  =
	 collidesWith(nx, sx, ny, sy, nz, sz) ||
	 collidesWith(ny, sy, nx, sx, nz, sz) ||
	 collidesWith(nz, sz, nx, sx, ny, sy);
	// if point on *moving will collide the current body, 
	// adjust dt, normal and p to the instant of collision
	if(collision) {
		dt     *= lambda;
		normal  = n * rot; // from local to world space
	}

	return collision;
}

// collidesWith determines whether the path [begin,end] will collide with
// the plane that has normal n and is at a distance +-s from the centroid
// of the current body
//
// this function assumes that the begin and end vectors are set and have
// been corrected appropriately
//
bool Body::collidesWith(const Vector& n, float s, const Vector& na, 
 float sa, const Vector& nb, float sb) {

	bool  collision;
	float path_n, a_n;

	if (intersects(n, s, na, sa, nb, sb)) {
		path_n = dot(path, n);
		for (int i = 0; i < 8; i++) {
			a_n = dot(vertex[i] * toRef, n);
			// check that a has a component in the direction
			// opposite to the normal n
			if (a_n < 0) {
				// adjust initial and projected positions of
				// the moving body assuming that vertex[i]
				// will be the contact point
				correction = (a_n / path_n) * normalPath;
				begin      = initial + correction;
				end        = projected + correction;
				// update point of collision variables
				// if this vertex collides earlier
				intersects(n, s, na, sa, nb, sb);
			}
		}
		collision = true;
	}
	else
		collision = false;

	return collision;
}

// intersects determines whether the path [begin,end] crosses the plane
// that is at a distance +-s from the centroid with normal n
//
// if the path crosses this plane, this function further determines if the
// intersection lies within the planes described by na and nb at distances
// +-sa and +-sb from the centre of this plane
//
// intersects returns true if the intersection lies within these bounds,
// false otherwise; sets lambda to the fraction of the path at which the
// intersection will occur and sets normal to the normal to the plane
//
bool Body::intersects(const Vector& n, float s, const Vector& na, 
 float sa, const Vector& nb, float sb) {

	Vector xc;
	bool collision = false;
	float nc, ncbms, ncems, ncbps, nceps, kappa, qb, qc;

	nc    = dot(n, centroid);
	ncbms = dot(n, begin) - nc - s;
	ncems = dot(n, end)   - nc - s;
	ncbps = ncbms + s + s;
	nceps = ncems + s + s;
	if (ncbms == 0 && ncems == 0) {
	    // the path [begin,end] glides along the surface at + s
		lambda = 1.0f;
		this->n = n;
		collision = false;
	} else if (ncbps == 0 && nceps == 0) {
	    // the path [begin,end] glides along the surface at - s
		lambda = 1.0f;
		this->n = -n;
		collision = false;
	} else if (ncbms > 0 && ncems < 0) {
	    // the path [begin,end] crosses the surface at + s
		// so, find the point of crossing x
		kappa = - ncbms / (ncems - ncbms);
		p  = begin + kappa * path;
		// crossing point relative to centre of the bounding surface
		xc = p - centroid - n * s;
		qb = dot(na, xc);
		qc = dot(nb, xc);
		if (qb <= sa && qb >= -sa && qc <= sb && qc >= -sb && kappa < lambda) {
			lambda = kappa;
			this->n = n;
			collision = true;
		}
	} else if (ncbps < 0 && nceps > 0) {
	    // the path [begin,end] crosses the surface at - s
		// so, find the point of crossing x
		kappa = - ncbps / (nceps - ncbps);
		p  = begin + kappa * path;
		// crossing point relative to centre of the bounding surface
		xc = p - centroid + n * s;
		qb = dot(na, xc);
		qc = dot(nb, xc);
		if (qb <= sa && qb >= -sa && qc <= sb && qc >= -sb && kappa < lambda) {
			lambda = kappa;
			this->n = -n;
			collision = true;
		}
	}

	return collision;
}

// detectCollision determines whether *object_i will collide with
// *object_j during time step dt
//
// if the two objects collide, detectCollision reduces the time step to
// the sub-step before the instant of collision and returns the normal
// to the surface of collision where the normal is directed away from
// *object_i and the position of the contact point at the instant of 
// collision
//
bool detectCollision(IObject* object_i, IObject* object_j, float& dt,
 Vector& contact, Vector& normal) {

	bool   collision = false;
	Body*  body_i    = (Body*)object_i;
	Body*  body_j    = (Body*)object_j;
	float  dtij;     // time sub-step to the instant of collision
	Vector nij;      // normal to the collision surface
	Vector cij;      // position of the contact point

	// select the collision detection hierarchy 
	if (body_i->hasBoundingSphere() && body_j->hasBoundingSphere()) {
		// check for sphere-sphere collision
		dtij = dt; // assume collision occurs at end of time step
		if (sphereCollision(body_i, body_j, dtij, nij, cij)) {
			// the two spheres will collide, so check for box collision
			if (body_i->hasBoundingBox()) {
				// does body_j's sphere collide with body_i's box?
				dtij = dt; // assume collision occurs at end of time step
				collision = body_i->boxCollision(body_j, dtij, cij, nij);
			}
			else if (body_j->hasBoundingBox()) {
				// does body_i's sphere collide with body_j's box?
				dtij = dt; // assume collision occurs at end of time step
				collision = body_j->boxCollision(body_i, dtij, cij, nij);
			}
			else
				// no bounding box on either body, so apply sphere result
				collision = true;
		}
	}
	else if (body_i->hasBoundingSphere() && body_j->hasBoundingBox()) {
		// does body_i's sphere collide with body_j's box?
		dtij = dt; // assume collision occurs at end of time step
		collision = body_j->boxCollision(body_i, dtij, cij, nij);
	}
	else if (body_i->hasBoundingBox() && body_j->hasBoundingSphere()) {
		// does body_i's sphere collide with body_j's box?
		dtij = dt; // assume collision occurs at end of time step
		collision = body_i->boxCollision(body_j, dtij, cij, nij);
	}
	else if (body_i->hasBoundingBox() && body_j->hasBoundingBox()) {
		// does body_i's sphere collide with body_j's box?
		dtij = dt; // assume collision occurs at end of time step
		collision = body_i->boxCollision(body_j, dtij, cij, nij);
	}

	// update collision parameters
	if (collision) {
		dt      = dtij;
		normal  = nij;
		contact = cij;
	}

	return collision;
}

// sphereCollision determines whether the bounding sphere of *body_i will 
// collide with the bounding sphere of *body_j during time step dt
//
// if the two spheres will collide sphereCollision reduces the time step
// to the sub-step before the instant of collision
//
// this function is fairly accurate for small relative accelerations - it
// assumes no significant relative acceleration over the time step
//
bool sphereCollision(const Body* body_i, const Body* body_j, float& dt, 
 Vector& contact, Vector& n) {

	Vector c, vr;
	bool   collision = false;
	float  cr, cc, r, cvr, vr2, rr, dt_;

 	c   = body_i->position()       - body_j->position();
	r   = body_i->boundingRadius() + body_j->boundingRadius();
	vr  = body_i->velocity()       - body_j->velocity();
	rr  = r * r;
	cc  = dot(c, c);
	cr  = cc - rr;
	cvr = dot(c, vr);
	vr2 = dot(vr, vr);

	// check for penetration regardless of relative velocity
	if (cr <= 0.0f) {
		// penetration has already occured
		//
		// check if the bodies are relatively stationary or sliding
		// over one another - compare relative velocity with the
		// vector connecting the two bodies
		if (cvr * cvr <= NEAR_ZERO * cc * vr2) {
			// there is continued contact and possibly sliding
			// - relative velocity is zero or perpendicular
		    // to the vector connecting the two bodies 
			// - do not reduce the size of the time step
		} else if (cvr < 0.0f) {
			// the two bodies are approaching one another,
			// their collision has already occured - their
			// relative velocity is in a direction obtuse
			// to the vector connecting the two bodies
		    // assume that collision occured at the start
		    // of the time step - but setting dt to 0
		    // would generate an infinite force: so use
		    // the smallest possible time step instead
		    dt = dtmin;
		}
		// the contact surface normal is parallel to
		// the vector connecting the two bodies
	    n = normal(c);
		collision = true;
	// no penetration - check for relative approach
	} else if (cvr < 0.0f && vr2 * dt * dt + 2.0f * cvr * dt + cr <= 0.0f) {
		// the two bodies are approaching one another and
		// close enough for a collision to occur
		// determine the time sub-step at which the collision will occur
		dt_ = (- cvr - sqrtf(cvr * cvr - vr2 * cr)) / vr2;
		// move slightly beyond this point of collision
		dt_ = 1.01f * dt_;
		if (dt_ < dt) {
		    // as long as the time sub-step is less than the full time
		    // step, reset the time step to the point of collision
		    dt = dt_;
		    if (dt < dtmin)
				// however, do not reduce the time step below
				// the minimum time step
				dt = dtmin;
		}
		// the contact surface normal is parallel to
		// vector connecting the two bodies
		n = normal(c);
		collision = true;
	}

	return collision;
}

// collide applies the effects of a collision to the kinematic
// properties of the colliding objects
//
void collide(IObject* object_i, IObject* object_j, const Vector& c,
 const Vector& n) {

	((AnimatedFrame*)object_i)->reflect(n);
	((AnimatedFrame*)object_j)->reflect(-n);
}
