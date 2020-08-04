//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeIncrementalTrussStrain.h"

/**
 * ComputeFiniteTrussStrain calculates the rotation increment to account for finite rotations of the
 * truss. The small/large strain increments in the current rotated configuration of the truss are also
 * computed.
 */
class ComputeFiniteTrussStrain : public ComputeIncrementalTrussStrain
{
public:
  static InputParameters validParams();

  ComputeFiniteTrussStrain(const InputParameters & parameters);

protected:
  // Compute incremental rotation matrix from the previous time step to the current configuration
  // void computeRotation() override;

  /// Rotational transformation from the global to truss local coordinate system at time t.
  // const MaterialProperty<RankTwoTensor> & _total_rotation_old;
};
