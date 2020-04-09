//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeIncrementalBeamStrain.h"

/**
 * ComputeFiniteBeamStrain calculates the rotation increment to account for finite rotations of the
 * beam. The small/large strain increments in the current rotated configuration of the beam are also
 * computed.
 */
class ComputeFiniteBeamStrain : public ComputeIncrementalBeamStrain
{
public:
  static InputParameters validParams();

  ComputeFiniteBeamStrain(const InputParameters & parameters);

protected:
  // Compute incremental rotation matrix from the previous time step to the current configuration
  void computeRotation() override;

  /// Rotational transformation from the global to beam local coordinate system at time t.
  const MaterialProperty<RankTwoTensor> & _total_rotation_old;
};
