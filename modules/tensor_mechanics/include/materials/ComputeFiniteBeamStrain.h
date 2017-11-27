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
 * ComputeFiniteBeamStrain calculates rotation increment to account for finite rotations of the
 * beam.
 */

class ComputeFiniteBeamStrain;

template <>
InputParameters validParams<ComputeFiniteBeamStrain>();

class ComputeFiniteBeamStrain : public ComputeIncrementalBeamStrain
{
public:
  ComputeFiniteBeamStrain(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;

  // Compute incremental rotation matrix from the previous time step to the current configuration
  void computeRotation();

  const MaterialProperty<RankTwoTensor> & _total_rotation_old;
};

#endif // COMPUTEFINITEBEAMSTRAIN_H
