//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeElasticityTensorBase.h"

/**
 * ComputeIsotropicElasticityTensor defines an elasticity tensor material for
 * isotropic materials.
 */
template <bool is_ad, typename T>
class ComputeIsotropicElasticityTensorTempl : public ComputeElasticityTensorBaseTempl<is_ad, T>
{
public:
  static InputParameters validParams();

  ComputeIsotropicElasticityTensorTempl(const InputParameters & parameters);

  virtual void initialSetup() override { residualSetup(); }

  virtual void residualSetup() override;

protected:
  virtual void computeQpElasticityTensor() override;

  /// Elastic constants
  bool _bulk_modulus_set;
  bool _lambda_set;
  bool _poissons_ratio_set;
  bool _shear_modulus_set;
  bool _youngs_modulus_set;

  const Real & _bulk_modulus;
  const Real & _lambda;
  const Real & _poissons_ratio;
  const Real & _shear_modulus;
  const Real & _youngs_modulus;

  /// Individual elasticity tensor
  T _Cijkl;

  /// Effective stiffness of the element: function of material properties
  Real _effective_stiffness_local;

  using ComputeElasticityTensorBaseTempl<is_ad, T>::name;
  using ComputeElasticityTensorBaseTempl<is_ad, T>::_elasticity_tensor_name;
  using ComputeElasticityTensorBaseTempl<is_ad, T>::issueGuarantee;
  using ComputeElasticityTensorBaseTempl<is_ad, T>::isParamValid;
  using ComputeElasticityTensorBaseTempl<is_ad, T>::_elasticity_tensor;
  using ComputeElasticityTensorBaseTempl<is_ad, T>::_qp;
  using ComputeElasticityTensorBaseTempl<is_ad, T>::_effective_stiffness;
};

typedef ComputeIsotropicElasticityTensorTempl<false, RankFourTensor>
    ComputeIsotropicElasticityTensor;
typedef ComputeIsotropicElasticityTensorTempl<true, RankFourTensor>
    ADComputeIsotropicElasticityTensor;
typedef ComputeIsotropicElasticityTensorTempl<false, SymmetricRankFourTensor>
    SymmetricIsotropicElasticityTensor;
typedef ComputeIsotropicElasticityTensorTempl<true, SymmetricRankFourTensor>
    ADSymmetricIsotropicElasticityTensor;
