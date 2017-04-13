/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEVARIABLEELASTICCONSTANTSTRESS_H
#define COMPUTEVARIABLEELASTICCONSTANTSTRESS_H

#include "ComputeFiniteStrainElasticStress.h"

/**
 * ComputeVariableElasticConstantStress computes the elastic stress for finite strains
 * when the elasticity tensor constants vary during the simulation, e.g. the Young's
 * Modulus is a function of temperature.
 */
class ComputeVariableElasticConstantStress : public ComputeFiniteStrainElasticStress
{
public:
  ComputeVariableElasticConstantStress(const InputParameters & parameters);

protected:
  virtual void computeQpStress();

  /// The old elasticity tensor is required to recover the old strain state from the old stress
  const MaterialProperty<RankFourTensor> & _elasticity_tensor_old;
};

#endif // COMPUTEVARIABLEELASTICCONSTANTSTRESS_H
