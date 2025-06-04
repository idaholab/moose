//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GrainDataTracker.h"
#include "RankFourTensor.h"

class EulerAngleProvider;

/**
 * Manage a list of elasticity tensors for the grains
 */
class GrainTrackerElasticity_OR : public GrainDataTracker<RankFourTensor>
{
public:
  static InputParameters validParams();

  GrainTrackerElasticity_OR(const InputParameters & parameters);

protected:
  RankFourTensor newGrain(unsigned int new_grain_id);

  /// generate random rotations when the Euler Angle provider runs out of data (otherwise error out)
  const bool _random_rotations;

  /// unrotated elasticity tensor
  RankFourTensor _C_ijkl;

  /// object providing the Euler angles
  const EulerAngleProvider & _euler;

  // Input additional Euler angles for the orientation relationship
  const Real & _Euler_angles_OR_1;
  const Real & _Euler_angles_OR_2;
  const Real & _Euler_angles_OR_3;

  // new function to obtain the orientation of each grain
  // Real newGrain(unsigned int new_grain_id);
  /// orientation (first euler angle)
  // Real _orientation;


};
