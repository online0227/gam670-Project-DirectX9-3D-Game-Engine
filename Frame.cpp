/* Implementation of Kinematics
 *
 * Frame.cpp
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "Frame.h"

//-------------------------------- Frame -----------------------------------
//
// a Frame describes a position and orientation with respect to another 
// Frame, or if independent, with respect to world space
//
Frame::Frame() : parent(0), T(1), s(Vector(1, 1, 1)) {}

// rotate? rotates the Frame "rad" radians about an axis.
//
// about the local x axis
//
void Frame::rotatex(float rad) {

	Matrix rot = ::rotatex(rot, rad);
    rotate(rot);
}
// about the local y axis
//
void Frame::rotatey(float rad) {

	Matrix rot = ::rotatey(rot, rad);
    rotate(rot);
}
// about the local z axis
//
void Frame::rotatez(float rad) {

	Matrix rot = ::rotatez(rot, rad);
    rotate(rot);
}
// about axis a through rad radians
//
void Frame::rotate(Vector a, float rad) {

	Matrix rot = ::rotate(a, rad);
    rotate(rot);
}

// rotate rotates the Frame using rotation matrix rot
//
void Frame::rotate(const Matrix& rot) {

    Vector cr(T.m41, T.m42, T.m43);
    Frame::move(-cr.x, -cr.y, -cr.z);
    T *= rot;
    Frame::move(cr.x, cr.y, cr.z);
}
void Frame::planetRotate(float rad) {
	Matrix rot;
	T *= ::rotatez(rot, rad);

}

// move translates the Frame by vector [x, y, z]
//
void Frame::move(float x, float y, float z) {

    Matrix trans;
	T *= ::translate(trans, x, y, z);
}

// scale scales the Frame by factors [x, y, z]
//
void Frame::scale(float sx, float sy, float sz) {

    Matrix trans;
    Vector cs(T.m41, T.m42, T.m43);
    Frame::move(-cs.x, -cs.y, -cs.z);
	T *= ::scale(trans, sx, sy, sz);
    Frame::move(cs.x, cs.y, cs.z);
	s.x *= sx;
	s.y *= sy;
	s.z *= sz;
}

// position returns the position of the Frame in world space
//
// Note that this function constructs this position recursively
//
Vector Frame::position() const {

	return parent ? ::position(T) * parent->rotation() + parent->position() : 
		::position(T);
}

// world returns the homogeneous transformation of the Frame with
// respect to world space
//
// Note that this function constructs the transformation recursively
//
Matrix Frame::world() const {

    return parent ? T * parent->world() : T;
}

// rotation returns the orientation of the Frame with respect to 
// world space
//
// Note that this function constructs the orientation recursively
//
Matrix Frame::rotation() const {

	return parent ? ::rotation(T) * parent->rotation() : ::rotation(T);
}

// orientation returns the orientation of vector v in world space
//
Vector Frame::orientation(const Vector& v) const {

    return v * rotation();
}


// orientation returns the orientation of the ? axis of the
// Frame in world space
//
Vector Frame::orientation(char axis) const {

    Vector v;

    switch(axis) {
        case 'x':
            v = Vector(1, 0, 0);
            break;
        case 'y':
            v = Vector(0, 1, 0);
            break;
        case 'z':
            v = Vector(0, 0, 1);
            break;
    }

    return orientation(v);
}

// orient orients the Frame to the orientation specified by rot
//
void Frame::orient(const Matrix& rot) {

    Matrix trans;
	Vector p(T.m41, T.m42, T.m43);
    T.isIdentity();
    T *= rot;
	T *= ::scale(trans, s.x, s.y, s.z);
    move(p.x, p.y, p.z);
}

// save saves the orientation matrix rot in Rold
//
void Frame::save(const Matrix& rot) {

	Rold = rot;
}

// restore returns the orientation matrix stored in Rold
//
Matrix Frame::restore() const {

	return Rold;
}

// attach attaches the current frame to Frame* parent and optionally
// resets the transformation to align the Frame with the parent; 
// otherwise applies the existing transformation 
//
void Frame::attach(IObject* newParent, bool reset) {

	if (parent) T = world();
	parent = (Frame*)newParent;
	if (reset) {
		Vector& p = parent->position();
		move(-p.x, -p.y, -p.z);
		Matrix m = parent->rotation();
		m = m.transpose();
		rotate(m);
	}
}

// detach detahces the current Frame and resets the 
// transformation to one wrt to world space
//
void Frame::detach() {

	if (parent) T = world();
	parent = 0;
}

//-------------------------------- AnimatedFrame -------------------------
//
// An AnimatedFrame is a Frame with linear and angular velocities and 
// possibly linear and angular accelerations
//
AnimatedFrame::AnimatedFrame() : parent(0) {}

// velocity returns the linear velocity of the Frame in world space
//
// Note that this function constructs this velocity recursively
//
Vector AnimatedFrame::velocity() const {

	return parent ? v * parent->rotation() + parent->velocity() : v;
}

// velocity sets the Frame's linear velocity to vx, vy, vz relative
// to its parent
//
void AnimatedFrame::velocity(float vx, float vy, float vz) {

	if (parent)
		v = (Vector(vx, vy, vz) - parent->velocity()) * 
		 parent->rotation().transpose();
	else
		v = Vector(vx, vy, vz);
}

// acceleration returns the Frame's linear acceleration in world space
//
// Note that this function constructs this acceleration recursively
//
Vector AnimatedFrame::acceleration() const {

	return parent ? a * parent->rotation() + parent->acceleration() : a;
}

// accelerate sets the Frame's linear acceleration to ax, ay, az relative
// to its parent
//
void AnimatedFrame::accelerate(float ax, float ay, float az) {

	if (parent)
		a = (Vector(ax, ay, az) - parent->acceleration()) * 
		 parent->rotation().transpose();
	else
		a = Vector(ax, ay, az);
}

// velocity sets the Frame's angular velocity to vx, vy, vz
//
void AnimatedFrame::angularVelocity(float vx, float vy, float vz) {

	angular_v = Vector(vx, vy, vz);
}

// accelerate sets the Frame's angular acceleration to ax, ay, az
//
void AnimatedFrame::angularAcceleration(float ax, float ay, float az) {

	angular_a = Vector(ax, ay, az);
}

// update moves the animated frame into its new position after time
// step dt
//
// this function receives the time step in SECONDS while much of the code 
// uses milliseconds as the time unit
//
void AnimatedFrame::update(float dt) {

	// calculate linear displacement
	Vector disp = dt * v + 0.5f * dt * dt * a;
	// displace the frame
	move(disp.x, disp.y, disp.z);
	
	// update the linear velocity
	v += dt * a;

	// calculate the change in orientation 
	Matrix omega(           0,  angular_v.z, -angular_v.y, 0,
				 -angular_v.z,            0,  angular_v.x, 0,
				  angular_v.y, -angular_v.x,            0, 0,
				            0,            0,            0, 0);
	Matrix Rcurrent = rotation();
	Matrix Rdelta   = dt * (omega * Rcurrent);
	Matrix Rnew     = orthoNormalize(Rcurrent + Rdelta);
	Matrix Rnet     = orthoNormalize(Rcurrent.transpose() * Rnew);
	// rotate the frame
	rotate(Rnet);

	// update the angular velocity
	angular_v += dt * angular_a;
}

// reflect adjusts the Frame's linear velocity to create the effect
// of a reflection off the plane defined by the normal n
//
void AnimatedFrame::reflect(const Vector& n) {

	v -= 2 * dot(n, v) * n;
}

// attach attaches the current Frame to *parent and optionally
// resets the transformation to align the Frame with the parent; 
// otherwise applies the existing transformation 
//
void AnimatedFrame::attach(IObject* newParent, bool reset) {

	if (parent) {
		v = velocity();
		a = acceleration();
	}
	parent = (Frame*)newParent;
	if (reset) {
		// to do
	}
	Frame::attach(newParent, reset);
}

// detach detaches the current Frame and resets the 
// transformation to one wrt to world space
//
void AnimatedFrame::detach() {

	if (parent) {
		v = velocity();
		a = acceleration();
	}
	parent = 0;
	Frame::detach();
}

