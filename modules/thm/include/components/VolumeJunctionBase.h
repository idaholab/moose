#pragma once

#include "FlowJunction.h"

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

public:
  static InputParameters validParams();
};
