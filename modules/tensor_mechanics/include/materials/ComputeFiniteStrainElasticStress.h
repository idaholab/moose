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

  /**
   * InitialStress Deprecation: remove this method
   *
   * Rotates initial_stress via rotation_increment.
   * In large-strain scenarios this must be used before addQpInitialStress
   */
  virtual void rotateQpInitialStress();

  const MaterialProperty<RankTwoTensor> & _strain_increment;
  const MaterialProperty<RankTwoTensor> & _rotation_increment;
  const MaterialProperty<RankTwoTensor> & _stress_old;

  /**
   * The old elastic strain is used to calculate the old stress in the case
   * of variable elasticity tensors
   */
  const MaterialProperty<RankTwoTensor> & _elastic_strain_old;
};

#endif // COMPUTEFINITESTRAINELASTICSTRESS_H
