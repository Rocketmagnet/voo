#ifndef _VECTOR3D_H_INCLUDED
#define _VECTOR3D_H_INCLUDED

#include <cmath>
#include <ostream>

typedef double NonInt;///TODO What is this needed for? :)

extern int Screen_Central_X;///TODO What is this needed for? :)
extern int Screen_Central_Y;///TODO What is this needed for? :)
extern int Screen_Res_X;///TODO What is this needed for? :)
extern int Screen_Res_Y;///TODO What is this needed for? :)
extern NonInt zoom;///TODO What is this needed for? :)


class point3D;
class matrix3x3;
class Frame2D;
class Vector3D;

class Vector2D
{
    friend class Vector3D;
  private:
    // 8 bytes

  public:
    NonInt x,y;

    Vector2D() {}

    Vector2D ( NonInt xx, NonInt yy );
    Vector2D ( int xx, int yy );
    Vector2D ( NonInt t );
    Vector2D ( const Vector3D & V3D );

    NonInt Mag() const;
    NonInt MagSquared() const;
    void Normalise();
    /// @return A Vector2D with the same versus and direction but unitary module.
    Vector2D UnitVector() const;
    // return the vector, rotated 90 deg anti-clock and normalised
    Vector2D NormalVector() const;

    Vector2D Rotate90DegAntiClockwise() const;
    Vector2D Rotate90DegClockwise()     const;
    float Angle() const;

    // vectors operating on vectors
    friend Vector2D operator + ( const Vector2D& v1, const Vector2D& v2 );
    friend Vector2D operator - ( const Vector2D& v1, const Vector2D& v2 );
    friend Vector2D operator / ( const Vector2D& a,  const Vector2D& b );  // cross product
    friend NonInt   operator * ( const Vector2D& a,  const Vector2D& b );

    //friend Vector3D operator *(const matrix3x3& v1, const Vector2D& v);

    //Vector2D operator = ( const Vector2D& a );
    void operator += ( const Vector2D& a );
    void operator -= ( const Vector2D& a );
    void operator /= ( NonInt s );
    void operator *= ( NonInt s );


    // vectors operating on scalars
    friend Vector2D operator * ( const Vector2D& a, const NonInt   s );
    friend Vector2D operator * ( const NonInt   s, const Vector2D& a );
    friend Vector2D operator / ( const Vector2D& a, const NonInt   s );


    Vector2D operator -() const {return Vector2D ( -x,-y );}

    //friend bool operator < (const Vector2D& v1, const Vector2D& v2) {return v1.x < v1.y;}
    //friend bool operator > (const Vector2D& v1, const Vector2D& v2) {return v1.x > v1.y;}

    //friend Vector2D operator -(const point3D& p1, const point3D& p2);
    //friend Vector2D between(const point3D& p1, const point3D& p2);
    //friend point3D operator -(const point3D& p, const Vector3D& v);

    friend int operator == ( const Vector2D& a, const Vector2D& b );
    friend int operator != ( const Vector2D& a, const Vector2D& b );

    void Rotate ( NonInt angle );

    static Vector2D const zeroVector;
    static Vector2D const unitXVector;
    static Vector2D const unitYVector;
};//class

class Frame2D
{
  public:
    Vector2D centre, x_axis;
    bool is_unit;
    float angle;

    Frame2D() :
        centre ( Vector2D::zeroVector ),
        x_axis ( Vector2D::unitXVector ),
        is_unit ( true ),
        angle ( 0 )
    {
    }

    Frame2D ( const Vector2D & C ) :
        centre ( C ),
        x_axis ( Vector2D::unitXVector ),
        is_unit ( true ),
        angle ( 0 )
    {
    }

    Frame2D ( const Vector2D & C, const Vector2D & I ) :
        centre ( C ),
        x_axis ( I ),
        is_unit ( false )
        //TODO ############angle (acos (C.unitVector () * I.unitVector ())) // a.b=|a|*|b|*cos (ab)
    {
    }

    Frame2D ( const Vector2D & C, const float & a ) :
        centre ( C ),
        x_axis ( Vector2D ( ( float ) cos ( a ), ( float ) sin ( a ) ) ),
        angle ( a )
    {
    }

    Vector2D Reorient ( const Vector2D & V )
    {
      return Vector2D ( V.x*x_axis.x - V.y*x_axis.y,
                        V.x*x_axis.y + V.y*x_axis.x );
    }

    void Rotate ( float a )
    {
      angle += a;
      x_axis = Vector2D ( ( float ) cos ( angle ), ( float ) sin ( angle ) );
    }

    friend Vector2D operator * ( const Vector2D & V, const Frame2D & F )
    {
      Vector2D V2 ( V.x-F.centre.x, V.y-F.centre.y );

      V2 = Vector2D ( V.x*F.x_axis.x - V.y*F.x_axis.y + F.centre.x,
                      V.x*F.x_axis.y + V.y*F.x_axis.x + F.centre.y );

      return V2;
    }

    void operator *= ( const Frame2D & F )
    {
      Vector2D y_axis = Vector2D ( -x_axis.y, x_axis.x );
      Vector2D new_centre = centre + F.centre.x*x_axis + F.centre.y*y_axis;
      Vector2D new_x_axis = x_axis * Frame2D ( Vector2D ( 0,0 ), F.x_axis );
      centre = new_centre;
      x_axis = new_x_axis;
    }

    friend Frame2D operator * ( const Frame2D& f1, const Frame2D& f2 );

};



class Vector3D
{
    //friend class point3D;
    friend class Vector2D;

  private:
// 12 bytes

  public:
    NonInt x,y,z;


    Vector3D();
    Vector3D ( NonInt xx, NonInt yy, NonInt zz );
    Vector3D ( int xx, int yy, int zz );
    Vector3D ( NonInt t );
    Vector3D ( Vector2D V2D ) {x=V2D.x; y=V2D.y; z=0.0f;}

    NonInt Mag() const;
    NonInt MagSquared() const;
    void Normalise();
    /// @return A Vector3D with the same versus and direction but unitary module.
    Vector3D UnitVector () const;

    /// This is just a 2D operation.
    /// @return The vector, rotated 90 deg anti-clock and normalised
    Vector3D NormalVector() const;

    // vectors operating on vectors
    friend Vector3D operator + ( const Vector3D& v1, const Vector3D& v2 );
    friend Vector3D operator - ( const Vector3D& v1, const Vector3D& v2 );
    friend Vector3D operator / ( const Vector3D& a, const Vector3D& b );  // cross product
    friend NonInt operator * ( const Vector3D& a, const Vector3D& b );

    void operator *= ( const Frame2D& frame2D )
    {
        Vector2D *vptr = (Vector2D*)this;
        Vector2D v2d;
        v2d.x = x;
        v2d.y = y;

        (*vptr) = v2d*frame2D;
    }


    friend Vector3D operator * ( const matrix3x3& v1, const Vector3D& v );

    Vector3D operator = ( const Vector3D& a );
    void operator += ( const Vector3D& a );
    void operator -= ( const Vector3D& a );
    void operator /= ( NonInt s );
    void operator *= ( NonInt s );

    // vectors operating on scalars
    friend Vector3D operator * ( const Vector3D& a, const NonInt s );
    friend Vector3D operator * ( const NonInt s, const Vector3D& a );
    friend Vector3D operator / ( const Vector3D& a, const NonInt s );


    Vector3D operator -()   {return Vector3D ( -x,-y,-z );}

    //friend bool operator < (const Vector3D& v1, const Vector3D& v2) {return v1.x < v1.y;}
    //friend bool operator > (const Vector3D& v1, const Vector3D& v2) {return v1.x > v1.y;}

    friend Vector3D operator - ( const point3D& p1, const point3D& p2 );
    friend Vector3D between ( const point3D& p1, const point3D& p2 );
    friend point3D operator - ( const point3D& p, const Vector3D& v );

    friend int operator == ( const Vector3D& a, const Vector3D& b );
    friend int operator != ( const Vector3D& a, const Vector3D& b );

    void Rotate ( char axis, NonInt angle );

    static const Vector3D zeroVector3d;
    static const Vector3D unitXVector3d;
    static const Vector3D unitYVector3d;
    static const Vector3D unitZVector3d;
};

//1842 9836 4977 6129


class point3D
{
  private:
    // 32 bytes

  public:

    Vector3D fixed; // origional location
    Vector3D trans; // after transformations
    NonInt scrx, scry;  // screen location

    // constructors

    //p_a
    point3D();

    //p_b
    point3D ( NonInt xx, NonInt yy, NonInt zz );
    point3D ( Vector3D v );

    NonInt  x();
    NonInt  y();
    NonInt  z();
    NonInt sx() const;
    NonInt sy() const;

    void project();

    //p_e
    point3D& operator = ( const point3D& a );
    //p_i

    friend Vector3D operator - ( const point3D& p1, const point3D& p2 );

};


class matrix3x3
{
    //friend class point3D;

  private:
// 12 bytes

  public:
    Vector3D a,b,c;

    matrix3x3();
    matrix3x3 ( Vector3D aa, Vector3D bb, Vector3D cc );
};



extern NonInt distance ( point3D *p1, point3D *p2 );
extern NonInt distance ( Vector3D *p1, Vector3D *p2 );
extern NonInt distance_sq ( Vector3D *p1, Vector3D *p2 );
extern Vector3D randomvector();
extern Vector3D ZERO_VECTOR;
float cosangle ( Vector3D v1, Vector3D v2 );

//ostream& operator << (ostream& os, const Vector2D& v);
//ostream& operator << (ostream& os, const Vector3D& v);

#endif

