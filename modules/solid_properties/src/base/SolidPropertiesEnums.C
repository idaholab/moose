#include "SolidPropertiesEnums.h"

MooseEnum getSurfaceEnum(const std::string & default_val)
{
  return MooseEnum("oxidized polished", default_val);
}
