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
  };

  inline Real polyLog(Real x)
  {
    Real tol = 1.e-4;
    Real c1=2.0e12;
    Real c2=-3.0e8;
    Real c3=3.0e4;
    Real c4=-11.04367370530952;

    if(x<tol)
      return c1*x*x*x/6.0 + c2*x*x/2.0 + c3*x + c4;
    else
      return std::log(x);
  };

  inline Real dpolyLog(Real x)
  {
    Real tol = 1.e-4;
    Real c1=2.0e12;
    Real c2=-3.0e8;
    Real c3=3.0e4;

    if(x<tol)
      return c1*x*x/2.0 + c2*x + c3;
    else
      return 1.0/x;
  };

  inline Real d2polyLog(Real x)
  {
    Real tol = 1.e-4;
    Real c1=2.0e12;
    Real c2=-3.0e8;

    if(x<tol)
      return c1*x + c2;
    else
      return -1.0/(x*x);
  };
};

#endif //MATHUTILS_H
