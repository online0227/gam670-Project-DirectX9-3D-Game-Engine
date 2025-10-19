#ifndef _FRAME_H_
#define _FRAME_H_

/* Header for an Object's Reference Frame
 *
 * Frame.h
 * version 1.0
 * gam670/dps905
 * Jan 11 2010
 * Chris Szalwinski
 */

#include "IScene.h" // for IObject
#include "math.h"   // for Matrix

//-------------------------------- Frame -----------------------------------
//
// a Frame specifies a position and orientation with respect to another 
// Frame, or if independent, with respect to world space
//
class Frame : public IObject {

    Frame* parent; // points to the parent frame, NULL if independent
    Matrix T;      // relative transformation matrix wrt parent frame or 
	               // wrt world space if independent
	Vector s;      // scaling vector
	Matrix Rold;   // saved orientation matrix

  public:
    Frame();
    void    scale(float sx, float sy, float sz);
    void    rotatex(float rad);
    void    rotatey(float rad);
    void    rotatez(float rad);
	void	planetRotate(float rad);
    void    rotate(Vector a, float rad);
    void    rotate(const Matrix& rot);
    void    move(float x, float y, float z);
    void    orient(const Matrix& rot);
	void    save(const Matrix& rot);
	Matrix  restore() const;
    Vector  position() const;
    Matrix  rotation() const;
    Vector  orientation(const Vector& v) const;
	Vector  orientation(char c) const;
    Matrix  world() const;
	void attach(IObject* newParent, bool reset);
	void detach();
    virtual ~Frame() {}
	friend class ParticleSystem;
	
};

//-------------------------------- AnimatedFrame -------------------------
//
// An AnimatedFrame is a Frame with linear and angular velocities and 
// possibly linear and angular accelerations
//
class AnimatedFrame : public Frame {

    Frame* parent;    // points to the parent frame, NULL if independent
	Vector v;         // relative linear velocity wrt the parent frame
	Vector a;         // relative linear acceleration
	Vector angular_v; // angular velocity wrt the parent frame
	Vector angular_a; // angular acceleration

public:
	AnimatedFrame();
	Vector velocity() const;
	Vector acceleration() const;
	Vector angularVelocity() const;
	Vector angularAcceleration() const;
	void   velocity(float vx, float vy, float vz);
	void   accelerate(float ax, float ay, float az);
	void   angularVelocity(float vx, float vy, float vz);
    void   angularAcceleration(float ax, float ay, float az);
	void   update(float dt);
	void   reflect(const Vector& n);
	void   attach(IObject* newParent, bool reset);
	void   detach();
};

#endif