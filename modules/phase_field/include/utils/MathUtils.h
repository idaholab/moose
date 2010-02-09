#ifndef MATHUTILS_H
#define MATHUTILS_H

#include "libmesh.h"

namespace MathUtils 
{
  inline Real badLog(Real x)
  {
    Real y = (x - 1.) / (x + 1.);
    Real sqy= y*y;
    return 2*y*( 1. + sqy*(1./3. + sqy*(1./5. + sqy*(1./7. + sqy*(1./9. + sqy*(1./11. + sqy*(1./13. + sqy*1./15)))))));
  }

  inline Real polyLog(Real x)
  {
    Real tol = 1.e-4;
    Real c1=-1.0e8;
    Real c2=2.0e4;
    Real c3=-10.71034037197618;

    if(x<tol)
      return c1*x*x/2.0 + c2*x + c3;
    else
      return std::log(x);
  }

  inline Real dpolyLog(Real x)
  {
    Real tol = 1.e-4;
    Real c1=-1.0e8;
    Real c2=2.0e4;

    if(x<tol)
      return c1*x + c2;
    else
      return 1.0/x;
  }

  inline Real d2polyLog(Real x)
  {
    Real tol = 1.e-4;
    Real c1=-1.0e8;

    if(x<tol)
      return c1;
    else
      return -1.0/(x*x);
  }

  inline Real polyLogLT(Real x)
  {
    Real tol = 1.e-2;
    Real c1=-1.0e4;
    Real c2=2.0e2;
    Real c3=-6.105170185988091;

    if(x<tol)
      return c1*x*x/2.0 + c2*x + c3;
    else
      return std::log(x);
  }

  inline Real dpolyLogLT(Real x)
  {
    Real tol = 1.e-2;
    Real c1=-1.0e4;
    Real c2=2.0e2;

    if(x<tol)
      return c1*x + c2;
    else
      return 1.0/x;
  }

  inline Real d2polyLogLT(Real x)
  {
    Real tol = 1.e-2;
    Real c1=-1.0e4;

    if(x<tol)
      return c1;
    else
      return -1.0/(x*x);
  }
}

#endif //MATHUTILS_H
