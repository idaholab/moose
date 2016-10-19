/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTERETURNMAPPINGSTRESS_H
#define COMPUTERETURNMAPPINGSTRESS_H

#include "ComputeFiniteStrainElasticStress.h"

class StressUpdateBase;

/**
 * ComputeReturnMappingStress computes the stress, with a return mapping
 * stress increment following elasticity theory for finite strains. The elastic
 * strain is calculated by subtracting the return mapping inelastic strain
 * increment tensor from the mechanical strain tensor.  Mechanical strain is
 * considered as the sum of the elastic and inelastic (plastic, creep, ect) strains.
 *
 * This material is used to call the recompute iterative materials.
 */

class ComputeReturnMappingStress : public ComputeFiniteStrainElasticStress
{
public:
  ComputeReturnMappingStress(const InputParameters & parameters);

protected:
  virtual void initialSetup();

  virtual void computeQpStress();

  virtual void updateQpStress(RankTwoTensor & strain_increment,
                             RankTwoTensor & stress_new);

  const unsigned int _max_its;
  const Real _relative_tolerance;
  const Real _absolute_tolerance;
  const bool _output_iteration_info;

  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
  const MaterialProperty<RankTwoTensor> & _strain_increment;
  MaterialProperty<RankTwoTensor> & _elastic_strain_old;
  std::vector<StressUpdateBase *> _models;
};

#endif //COMPUTERETURNMAPPINGSTRESS_H
