//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeMaterialInterface.h"
#include "Material.h"
#include "RankFourTensor.h"
#include "SymmetricRankFourTensor.h"
#include "GuaranteeProvider.h"

/**
 * ComputeElasticityTensorBase the base class for computing elasticity tensors
 */
template <bool is_ad, typename T = RankFourTensor>
class ComputeElasticityTensorBaseTempl : public DerivativeMaterialInterface<Material>,
                                         public GuaranteeProvider
{
public:
  static InputParameters validParams();

  ComputeElasticityTensorBaseTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  virtual void computeQpElasticityTensor() = 0;

  /// Base name of the material system
  const std::string _base_name;

  std::string _elasticity_tensor_name;

  GenericMaterialProperty<T, is_ad> & _elasticity_tensor;
  GenericMaterialProperty<Real, is_ad> & _effective_stiffness;

  /// prefactor function to multiply the elasticity tensor with
  const Function * const _prefactor_function;
};

typedef ComputeElasticityTensorBaseTempl<false> ComputeElasticityTensorBase;
typedef ComputeElasticityTensorBaseTempl<true> ADComputeElasticityTensorBase;
