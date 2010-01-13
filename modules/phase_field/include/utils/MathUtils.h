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
};

#endif //MATHUTILS_H
