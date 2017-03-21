/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GRAINTRACKERELASTICITY_H
#define GRAINTRACKERELASTICITY_H

#include "GrainDataTracker.h"
#include "RankFourTensor.h"

class GrainTrackerElasticity;
class EulerAngleProvider;

template <>
InputParameters validParams<GrainTrackerElasticity>();

/**
 * Manage a list of elasticity tensors for the grains
 */
class GrainTrackerElasticity : public GrainDataTracker<RankFourTensor>
{
public:
  GrainTrackerElasticity(const InputParameters & parameters);

protected:
  RankFourTensor newGrain(unsigned int new_grain_id);

  /// generate random rotations when the Euler Angle provider runs out of data (otherwise error out)
  const bool _random_rotations;

  /// unrotated elasticity tensor
  RankFourTensor _C_ijkl;

  /// object providing the Euler angles
  const EulerAngleProvider & _euler;
};

#endif // GRAINTRACKERELASTICITY_H
