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
    Real tol=1.0e-4;
    Real c1=2.0e12;
    Real c2=-3.0e8;
    Real c3=3.0e4;
    Real c4=-1.104367370530952e+01;

    if(x<tol)
      return c1*tol*tol*tol/6.0 + c2*tol*tol/2.0 + c3*tol + c4;
    else
      return std::log(x);
  };  
};

#endif //MATHUTILS_H
