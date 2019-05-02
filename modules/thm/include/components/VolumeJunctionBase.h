#pragma once

#include "FlowJunction.h"

class VolumeJunctionBase;

template <>
InputParameters validParams<FlowJunction>();

/**
 * Base class for volumetric junction components
 */
class VolumeJunctionBase : public FlowJunction
{
public:
  VolumeJunctionBase(const InputParameters & params);

protected:
  /// Volume of the junction
  const Real _volume;

  /// Spatial position of center of the junction
  const Point & _position;
};
