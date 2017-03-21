/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEFINITESTRAINELASTICSTRESS_H
#define COMPUTEFINITESTRAINELASTICSTRESS_H

#include "ComputeStressBase.h"

/**
 * ComputeFiniteStrainElasticStress computes the stress following elasticity
 * theory for finite strains
 */
class ComputeFiniteStrainElasticStress : public ComputeStressBase
{
public:
  ComputeFiniteStrainElasticStress(const InputParameters & parameters);

protected:
  virtual void computeQpStress();

  const MaterialProperty<RankTwoTensor> & _strain_increment;
  const MaterialProperty<RankTwoTensor> & _rotation_increment;
  MaterialProperty<RankTwoTensor> & _stress_old;
};

#endif // COMPUTEFINITESTRAINELASTICSTRESS_H
