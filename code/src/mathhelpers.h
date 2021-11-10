#ifndef _MATHHELPERS_H_
#define _MATHHELPERS_H_

template<typename T> T PI ()
{
  static const T sPI = (T)4 * (T)atan ((T)1);
  return sPI;
}

template<typename T> T TWOPI ()
{
  static const T s2PI = (T)2 * PI<T>();
  return s2PI;
}

template<typename T> T HALFPI ()
{
  static const T sPI2 = PI<T>() / (T)2;
  return sPI2;
}


//TODO Move them in a common math util place.
template<typename T>
T deg2rad (T angle)
{
  return (angle * PI<T>() / static_cast<T>(180));
}
template<typename T>
T rad2deg (T angle)
{
  return (angle * static_cast<T>(180) / PI<T>());
}

template<typename T>
T mm2mils (T x)
{
  return (x * static_cast<T>(39.37007874));
}

template<typename T>
T mils2mm (T x)
{
  return (x * static_cast<T>(0.0254));
}

template<typename T>
T clamp(T x, T min, T max)
{
  if (x<min)
    return min;
  if (x>max)
    return max;
  return x;
}

template<typename T>
T deadband(T x, T bandCentre, T bandWidth)
{
    if (x < (bandCentre-bandWidth*0.5))
        return x + bandWidth*0.5;

    if (x > (bandCentre+bandWidth*0.5))
        return x - bandWidth*0.5;

    return bandCentre;
}

template<typename T>
T remap(T x, T s1, T s2, T d1, T d2)
{
  return ((x-s1)/(s2-s1))*(d2-d1)+d1;
}

template<typename T>
T fmax(T a, T b)
{
    if (a > b)
        return a;
    else
        return b;
}
/*
float fmax(float a, float b)
{
    if (a > b)
        return a;
    else
        return b;
}
*/
#endif //_MATHHELPERS_H_
