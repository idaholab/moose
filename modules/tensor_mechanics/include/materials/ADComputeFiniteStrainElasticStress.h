//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADComputeStressBase.h"
#include "GuaranteeConsumer.h"

/**
 * ADComputeFiniteStrainElasticStress computes the stress following elasticity
 * theory for finite strains
 */
class ADComputeFiniteStrainElasticStress : public ADComputeStressBase, public GuaranteeConsumer
{
public:
  static InputParameters validParams();

  ADComputeFiniteStrainElasticStress(const InputParameters & parameters);

  void initialSetup() override;
  virtual void initQpStatefulProperties() override;

protected:
  virtual void computeQpStress() override;

  /// Name of the elasticity tensor material property
  const std::string _elasticity_tensor_name;
  /// Elasticity tensor material property
  const ADMaterialProperty<RankFourTensor> & _elasticity_tensor;
  const ADMaterialProperty<RankTwoTensor> & _strain_increment;
  /// Rotation up to current step "n" to compute anisotropic elasticity tensor
  ADMaterialProperty<RankTwoTensor> & _rotation_total;
  /// Rotation up to "n - 1" (previous) step to compute anisotropic elasticity tensor
  const MaterialProperty<RankTwoTensor> & _rotation_total_old;

  const ADMaterialProperty<RankTwoTensor> & _rotation_increment;

  /// The old stress tensor
  const MaterialProperty<RankTwoTensor> & _stress_old;

  /**
   * The old elastic strain is used to calculate the old stress in the case
   * of variable elasticity tensors
   */
  const MaterialProperty<RankTwoTensor> & _elastic_strain_old;
};
