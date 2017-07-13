/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEFINITESTRAINELASTICSTRESS_H
#define COMPUTEFINITESTRAINELASTICSTRESS_H

#include "ComputeStressBase.h"
#include "GuaranteeConsumer.h"

/**
 * ComputeFiniteStrainElasticStress computes the stress following elasticity
 * theory for finite strains
 */
class ComputeFiniteStrainElasticStress : public ComputeStressBase, public GuaranteeConsumer
{
public:
  ComputeFiniteStrainElasticStress(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpStress() override;

  const MaterialProperty<RankTwoTensor> & _strain_increment;
  const MaterialProperty<RankTwoTensor> & _rotation_increment;
  const MaterialProperty<RankTwoTensor> & _stress_old;

  /**
   * The old elastic strain is used to calculate the old stress in the case
   * of variable elasticity tensors
   */
  const MaterialProperty<RankTwoTensor> & _elastic_strain_old;

  /// flag for if the elasticity tensor does NOT change value over time
  bool _is_elasticity_tensor_guaranteed_constant_in_time;
};

#endif // COMPUTEFINITESTRAINELASTICSTRESS_H
