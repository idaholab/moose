#ifndef SOLID_PROPERTIES_ENUMS_H
#define SOLID_PROPERTIES_ENUMS_H

#include "MooseEnum.h"

MooseEnum getSurfaceEnum(const std::string & default_val);

namespace surface
{
enum SurfaceEnum
{
  oxidized,
  polished
};
}

#endif
