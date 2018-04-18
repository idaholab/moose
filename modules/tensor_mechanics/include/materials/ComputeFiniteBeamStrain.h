/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEFINITEBEAMSTRAIN_H
#define COMPUTEFINITEBEAMSTRAIN_H

#include "ComputeIncrementalBeamStrain.h"

/**
 * ComputeFiniteBeamStrain calculates the rotation increment to account for finite rotations of the
 * beam. The small/large strain increments in the current rotated configuration of the beam are also
 * computed.
 */

class ComputeFiniteBeamStrain;

template <>
InputParameters validParams<ComputeFiniteBeamStrain>();

class ComputeFiniteBeamStrain : public ComputeIncrementalBeamStrain
{
public:
  ComputeFiniteBeamStrain(const InputParameters & parameters);

protected:
  // Compute incremental rotation matrix from the previous time step to the current configuration
  void computeRotation() override;

  /// Rotational transformation from the global to beam local coordinate system at time t.
  const MaterialProperty<RankTwoTensor> & _total_rotation_old;
};

#endif // COMPUTEFINITEBEAMSTRAIN_H
