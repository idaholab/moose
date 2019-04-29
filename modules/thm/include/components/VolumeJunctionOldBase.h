#pragma once

#include "JunctionWithLossesBase.h"

class VolumeJunctionOldBase;

template <>
InputParameters validParams<VolumeJunctionOldBase>();

/**
 * Base class for junctions with a non-zero volume
 */
class VolumeJunctionOldBase : public JunctionWithLossesBase
{
public:
  VolumeJunctionOldBase(const InputParameters & parameters);

protected:
  virtual void computeDeltaH(Real H_junction);

  /// Initial pressure from user input
  Real _initial_p;
  /// Initial velocity from user input
  Real _initial_vel;
  /// Initial temperature from user input
  Real _initial_T;
  /// Initial void fraction from user input (if provided)
  Real _initial_void_fraction;

  /// Physical position in the space
  Point _center;
  /// Volume of the junction
  const Real & _volume;
  /// scale factors for scalar variables
  std::vector<Real> _scale_factors;

  /// Height difference between junction center and connecting nodes
  std::vector<Real> _deltaH;
};
