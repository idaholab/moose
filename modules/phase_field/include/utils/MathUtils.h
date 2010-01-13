#ifndef MATHUTILS_H
#define MATHUTILS_H

#include "libmesh.h"

namespace MathUtils 
{
  inline Real badLog(Real x)
  {
    return std::log(x);
  };
};

#endif //MATHUTILS_H
