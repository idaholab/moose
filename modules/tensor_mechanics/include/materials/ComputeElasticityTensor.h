//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeRotatedElasticityTensorBase.h"

/**
 * ComputeElasticityTensor defines an elasticity tensor material object with a given base name.
 */
template <bool is_ad>
class ComputeElasticityTensorTempl : public ComputeRotatedElasticityTensorBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  ComputeElasticityTensorTempl(const InputParameters & parameters);

protected:
  virtual void computeQpElasticityTensor() override;

  /// Individual material information
  RankFourTensor _Cijkl;

  using ComputeRotatedElasticityTensorBaseTempl<is_ad>::isParamValid;
  using ComputeRotatedElasticityTensorBaseTempl<is_ad>::_elasticity_tensor_name;
  using ComputeRotatedElasticityTensorBaseTempl<is_ad>::_Euler_angles;
  using ComputeRotatedElasticityTensorBaseTempl<is_ad>::_elasticity_tensor;
  using ComputeRotatedElasticityTensorBaseTempl<is_ad>::_qp;
  using ComputeRotatedElasticityTensorBaseTempl<is_ad>::issueGuarantee;
  using ComputeRotatedElasticityTensorBaseTempl<is_ad>::_rotation_matrix;
};

typedef ComputeElasticityTensorTempl<false> ComputeElasticityTensor;
typedef ComputeElasticityTensorTempl<true> ADComputeElasticityTensor;
