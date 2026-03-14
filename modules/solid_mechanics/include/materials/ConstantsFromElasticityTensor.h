//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankFourTensor.h"
#include "GuaranteeConsumer.h"

/**
 * This material computes the elastic constants such as Young's modulus
 * from the elasticity tensor.
 */
template <bool is_ad>
class ConstantsFromElasticityTensorTempl : public Material, public GuaranteeConsumer
{
public:
  static InputParameters validParams();

  ConstantsFromElasticityTensorTempl(const InputParameters & parameters);
  void initialSetup() override;

protected:
  void computeQpProperties() override;

private:
  /// Base name of the material system
  const std::string _base_name;

  ///{@ Elastic constants computed from tensor
  GenericMaterialProperty<Real, is_ad> & _youngs_modulus;
  GenericMaterialProperty<Real, is_ad> & _poissons_ratio;
  GenericMaterialProperty<Real, is_ad> & _shear_modulus;
  GenericMaterialProperty<Real, is_ad> & _bulk_modulus;
  ///@}

  /// Name of elasticity tensor
  const std::string _elasticity_tensor_name;

  /// The computed elastic constant
  const GenericMaterialProperty<RankFourTensor, is_ad> & _elasticity_tensor;
};

typedef ConstantsFromElasticityTensorTempl<false> ConstantsFromElasticityTensor;
typedef ConstantsFromElasticityTensorTempl<true> ADConstantsFromElasticityTensor;
