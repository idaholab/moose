/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTERETURNMAPPINGSTRESS_H
#define COMPUTERETURNMAPPINGSTRESS_H

#include "ComputeFiniteStrainElasticStress.h"

/**
 * ComputeReturnMappingStress computes the stress, with a return mapping
 * stress increment following elasticity theory for finite strains. The elastic
 * strain is calculated by subtracting the return mapping inelastic strain
 * increment tensor from the mechanical strain tensor.  Mechanical strain is
 * considered as the sum of the elastic and inelastic (plastic, creep, ect) strains.
 *
 * This material is used to call the recompute iterative materials through
 * RecomputeReturnStressIncrement; RecomputeReturnStressIncrement defines the
 * _return_stress_increment and _inelastic_strain_increment Rank2 tensors.
 */

class ComputeReturnMappingStress : public ComputeFiniteStrainElasticStress
{
public:
  ComputeReturnMappingStress(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpStress();

  MaterialProperty<RankTwoTensor> & _elastic_strain_old;
  const MaterialProperty<RankTwoTensor> & _return_stress_increment;
  const MaterialProperty<RankTwoTensor> & _inelastic_strain_increment;
  Material & _recompute_return_material;
};

#endif //COMPUTERETURNMAPPINGSTRESS_H
