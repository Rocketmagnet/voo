#include "vector3d.h"
#include <math.h>
#include <iostream>
#include "mathhelpers.h"

using namespace std;

#define PI 3.14159265f

int Screen_Central_X;
int Screen_Central_Y;
int Screen_Res_X;
int Screen_Res_Y;
NonInt zoom;

Vector2D const Vector2D::zeroVector  (static_cast<NonInt> (0), static_cast<NonInt> (0));
Vector2D const Vector2D::unitXVector (static_cast<NonInt> (1), static_cast<NonInt> (0));
Vector2D const Vector2D::unitYVector (static_cast<NonInt> (0), static_cast<NonInt> (1));
  

Vector2D::Vector2D(NonInt x, NonInt y)
: x (x),
  y (y)
{
}

Vector2D::Vector2D (int xx, int yy)
: x (NonInt (xx)),
  y (NonInt (yy))
{
}

Vector2D::Vector2D (NonInt t)
{
  //x = cos (t * TWOPI<float> ());      // I hope nobody used this !
  //y = sin (t * TWOPI<float> ());
  x = cos (t);
  y = sin (t);
}

// vectors operating on vectors
Vector2D operator -(const Vector2D& v1, const Vector2D& v2)   {return Vector2D(v1.x-v2.x, v1.y-v2.y);}
Vector2D operator +(const Vector2D& v1, const Vector2D& v2)   {return Vector2D(v1.x+v2.x, v1.y+v2.y);}
NonInt   operator *(const Vector2D& a,  const Vector2D& b)    {return ((a.x*b.x)+(a.y*b.y));}

// vectors operating on scalars
Vector2D operator *(const Vector2D& a, const NonInt   s)    {return Vector2D((a.x*s),(a.y*s));}  // Scaling a vector
Vector2D operator *(const NonInt s,   const Vector2D& a)    {return Vector2D((a.x*s),(a.y*s));}  // Scaling a vector
Vector2D operator /(const Vector2D& a, const NonInt   s)    {return Vector2D((a.x/s),(a.y/s));}  // Scaling a vector

//Vector2D Vector2D::operator = ( const Vector2D& a ) {x  = a.x;  y  = a.y; return a;}
void Vector2D::operator += ( const Vector2D& a ) {x += a.x;  y += a.y;}
void Vector2D::operator -= ( const Vector2D& a ) {x -= a.x;  y -= a.y;}
void Vector2D::operator /= ( NonInt s ) {x /= s;    y /= s;}
void Vector2D::operator *= ( NonInt s ) {x *= s; y *= s;}

    
int  operator ==(const Vector2D& a, const Vector2D& b) {return (b.x==a.x) && (b.y==a.y);}
int  operator !=(const Vector2D& a, const Vector2D& b) {return (b.x!=a.x) || (b.y!=a.y);}

NonInt Vector2D::Mag() const
{
  return (NonInt)sqrt((x*x)+(y*y));
}
NonInt Vector2D::MagSquared() const
{
  return (x*x)+(y*y);
}
void Vector2D::Normalise()
{
  *this *= ((NonInt)1.) / Mag();
}
Vector2D Vector2D::UnitVector () const
{
  return *this / Mag ();
}
Vector2D Vector2D::NormalVector () const
{
  NonInt inv_len = ((NonInt)1.) / Mag ();
  return Vector2D(y*inv_len, -x*inv_len);
}

Vector2D Vector2D::Rotate90DegAntiClockwise() const
{
  return Vector3D(y, -x, 0.);
}

Vector2D Vector2D::Rotate90DegClockwise() const
{
  return Vector3D(-y, x, 0.);
}

void Vector2D::Rotate(NonInt angle)
{
    NonInt Xtemp, Ytemp, costheta, sintheta;

    costheta = (NonInt)cos(angle);
    sintheta = (NonInt)sin(angle);

    Xtemp =  (costheta*x) + (sintheta*y);
    Ytemp =  (costheta*y) - (sintheta*x);

    x = Xtemp;
    y = Ytemp;
}


float Vector2D::Angle() const
{
    float angle;

    if (fabs(x)>fabs(y))
	{
        angle = atan(y/x);
		if (x<0.0f) angle += PI;
	}
	else
	{
		if (y==0.0f)                // x and y are zero
            angle = 0.0f;
		else
		{
			angle = PI*0.5f - (float)atan(x/y);
			if (y<0.0f)	angle += PI;
		}
	}
	if (angle < 0.0f) angle += PI*2;
	if (angle > PI*2) angle -= PI*2;

    return angle;
}



Vector2D::Vector2D(const Vector3D & V3D)
{
  x = V3D.x;
  y = V3D.y;
}


//-----------Vector3D starts
const Vector3D zeroVector3d ((NonInt)0, (NonInt)0, (NonInt)0);
const Vector3D unitXVector3d ((NonInt)1, (NonInt)0, (NonInt)0);
const Vector3D unitYVector3d ((NonInt)0, (NonInt)1, (NonInt)0);
const Vector3D unitZVector3d ((NonInt)0, (NonInt)0, (NonInt)1);

Vector3D::Vector3D(NonInt xx, NonInt yy, NonInt zz)
{
  x = xx;
  y = yy;
  z = zz;
}

Vector3D::Vector3D(int xx, int yy, int zz)
{
  x = (NonInt)xx;
  y = (NonInt)yy;
  z = (NonInt)zz;
}

Vector3D::Vector3D(NonInt t)
{
  x = cos(t*TWOPI<float>());
  y = sin(t*TWOPI<float>());
  z = 0.0f;
}
// vectors operating on vectors
Vector3D operator -(const Vector3D& v1, const Vector3D& v2) {return Vector3D(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z);}
Vector3D operator +(const Vector3D& v1, const Vector3D& v2) {return Vector3D(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z);}
Vector3D operator /(const Vector3D& a, const Vector3D& b)   {return Vector3D((a.y*b.z)-(a.z*b.y),(a.z*b.x)-(a.x*b.z),(a.x*b.y)-(a.y*b.x));}
NonInt   operator *(const Vector3D& a, const Vector3D& b)   {return ((a.x*b.x)+(a.y*b.y)+(a.z*b.z));}
// vectors operating on scalars
Vector3D operator *(const Vector3D& a, const NonInt s)    {return Vector3D((a.x*s),(a.y*s),(a.z*s));}  // Scaling a vector
Vector3D operator *(const NonInt s, const Vector3D& a)    {return Vector3D((a.x*s),(a.y*s),(a.z*s));}  // Scaling a vector
Vector3D operator /(const Vector3D& a, const NonInt s)    {return Vector3D((a.x/s),(a.y/s),(a.z/s));}  // Scaling a vector
Vector3D Vector3D::operator  = ( const Vector3D& a ) {x  = a.x;  y  = a.y; z  = a.z; return a;}
void Vector3D::operator += ( const Vector3D& a ) {x += a.x;  y += a.y; z += a.z;}
void Vector3D::operator -= ( const Vector3D& a ) {x -= a.x;  y -= a.y; z -= a.z;}
void Vector3D::operator /= ( NonInt s ) {x /= s; y /= s; z /= s;}
void Vector3D::operator *= ( NonInt s ) {x *= s; y *= s; z *= s;}


int  operator ==(const Vector3D& a, const Vector3D& b) {return (b.x==a.x) && (b.y==a.y) && (b.z==a.z);}
int  operator !=(const Vector3D& a, const Vector3D& b) {return (b.x!=a.x) || (b.y!=a.y) || (b.z!=a.z);}

NonInt Vector3D::Mag() const
{
  return (NonInt)sqrt((x*x)+(y*y)+(z*z));
}
NonInt Vector3D::MagSquared() const
{
  return (x*x)+(y*y)+(z*z);
}
void Vector3D::Normalise()
{
  NonInt invmag = (static_cast<NonInt> (1.))/Mag ();
  *this *= invmag;
}
Vector3D Vector3D::UnitVector () const
{
  return *this / Mag ();
}
Vector3D Vector3D::NormalVector () const
{
  NonInt inv_len = (static_cast<NonInt> (1.)) / Mag ();
  return Vector3D(y*inv_len, -x*inv_len, 0.);
}

//ostream & operator << (ostream & os, const Vector3D & v)
//{
//  return os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
//}
//
//ostream & operator << (ostream & os, const Vector2D & v)
//{
//  return os << "(" << v.x << ", " << v.y << ")";
//}

void Vector3D::Rotate(char axis, NonInt angle)
{
  NonInt Xtemp, Ytemp, Ztemp, costheta, sintheta;

  costheta = (NonInt)cos(angle);
  sintheta = (NonInt)sin(angle);

  switch (axis)
  {
    case ('x'):
    {
      Ytemp =  (costheta*y) + (sintheta*z);
      Ztemp =  (costheta*z) - (sintheta*y);
      y = Ytemp;
      z = Ztemp;
    }break;
    case ('y'):
    {
      Xtemp =  (costheta*x) + (sintheta*z);
      Ztemp =  (costheta*z) - (sintheta*x);
      x = Xtemp;
      z = Ztemp;
    }break;
    case ('z'):
    {
      Xtemp =  (costheta*x) + (sintheta*y);
      Ytemp =  (costheta*y) - (sintheta*x);
      x = Xtemp;
      y = Ytemp;
    }break;
  }
}



NonInt distance(Vector3D *p1, Vector3D *p2)
{
  NonInt xd,yd,zd;

  xd = p1->x - p2->x;
  yd = p1->y - p2->y;
  zd = p1->z - p2->z;

  return (NonInt)sqrt(xd*xd + yd*yd + zd*zd);
}

NonInt distance_sq(Vector3D *p1, Vector3D *p2)
{
  NonInt xd,yd,zd;

  xd = p1->x - p2->x;
  yd = p1->y - p2->y;
  zd = p1->z - p2->z;

  return (xd*xd + yd*yd + zd*zd);
}

// return distance between 2 points
NonInt distance(point3D *p1, point3D *p2)
{
  NonInt xd,yd,zd;

  xd = p1->fixed.x - p2->fixed.x;
  yd = p1->fixed.y - p2->fixed.y;
  zd = p1->fixed.z - p2->fixed.z;

  return (NonInt)sqrt(xd*xd + yd*yd + zd*zd);
}

Frame2D operator *(const Frame2D& f1, const Frame2D& f2)
{
  Frame2D frame2D;
  Vector2D y_axis = Vector2D(-f1.x_axis.y, f1.x_axis.x);
  Vector2D new_centre = f1.centre + f2.centre.x*f1.x_axis + f2.centre.y*y_axis;
  Vector2D new_x_axis = f1.x_axis * Frame2D(Vector2D::zeroVector, f2.x_axis);
  frame2D.centre = new_centre;
  frame2D.x_axis = new_x_axis;
  return frame2D;
}


//p_a
point3D::point3D()
{
  fixed = zeroVector3d;
  trans = zeroVector3d;
  scrx=scry=(NonInt)0;
}

//p_b
point3D::point3D(NonInt xx, NonInt yy, NonInt zz)
{
  fixed = Vector3D(xx,yy,zz);
  trans = Vector3D(xx,yy,zz);
  scrx=scry=(NonInt)0;
}

point3D::point3D(Vector3D v)
{
  fixed = v;
  trans = v;
  scrx=scry=(NonInt)0;
}

//p_c
/*
void point3D::setx(NonInt xx) {xp=xt=xx;]
void point3D::sety(NonInt yy) {yp=yt=yy;]
void point3D::setz(NonInt zz) {zp=zt=zz;]
*/

/*
NonInt point3D::xt() {return xt;}
NonInt point3D::yt() {return yt;}
NonInt point3D::zt() {return zt;}
*/

NonInt point3D::sx() const {return scrx;}
NonInt point3D::sy() const {return scry;}
/*
void point3D::setx(NonInt x) {xp=xt=x;}
void point3D::sety(NonInt y) {yp=yt=y;}
void point3D::setz(NonInt z) {zp=zt=z;}
*/
void point3D::project(){
  if (trans.z>=(NonInt)0) {
    scrx = ((trans.x*zoom/trans.z)+(NonInt)Screen_Central_X);
    scry = ((trans.y*zoom/trans.z)+(NonInt)Screen_Central_Y);
  }
}

point3D& point3D::operator = (const point3D& a)
{
  fixed = a.fixed;
  trans = a.trans;
  scrx=a.scrx;  scry=a.scry;
  return *this;
}

/*
point3D operator+ (const point3D& p1, const point3D& p2)
{
  point3D r;
  r.xp = p1.xp + p2.xp;
  r.yp = p1.yp + p2.yp;
  r.zp = p1.zp + p2.zp;
  r.xt = p1.xt + p2.xt;
  r.yt = p1.yt + p2.yt;
  r.zt = p1.zt + p2.zt;
  return r;
}
*/
/*
// subtract a vector from a point
point3D operator -(const point3D& p, const Vector3D& v)
{
  point3D r;
  r = p;
  r.xp -= v.vx;
  r.yp -= v.vy;
  r.zp -= v.vz;
  return r;
}
*/



Vector3D::Vector3D() {x=(NonInt)0.0;y=(NonInt)0.0;z=(NonInt)0.0;}

