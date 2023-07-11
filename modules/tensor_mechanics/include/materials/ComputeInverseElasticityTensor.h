#pragma once

#include "ComputeInverseRotatedElasticityTensorBase.h"
#include "DerivativeFunctionMaterialBase.h"

template <bool is_ad>
class ComputeInverseElasticityTensorTempl : public ComputeInverseRotatedElasticityTensorBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  ComputeInverseElasticityTensorTempl(const InputParameters & parameters);

protected:
  virtual void computeQpElasticityTensor() override;

  /// Individual material information
  RankFourTensor _Cijkl;
  RankFourTensor _Sijkl;
   
  using ComputeInverseRotatedElasticityTensorBaseTempl<is_ad>::isParamValid;
  using ComputeInverseRotatedElasticityTensorBaseTempl<is_ad>::_compliance_tensor_name;
  using ComputeInverseRotatedElasticityTensorBaseTempl<is_ad>::_elasticity_tensor_name;
  using ComputeInverseRotatedElasticityTensorBaseTempl<is_ad>::_compliance_tensor;
  using ComputeInverseRotatedElasticityTensorBaseTempl<is_ad>::_elasticity_tensor;
  using ComputeInverseRotatedElasticityTensorBaseTempl<is_ad>::_qp;
};

typedef ComputeInverseElasticityTensorTempl<false> ComputeInverseElasticityTensor;
typedef ComputeInverseElasticityTensorTempl<true> ADComputeInverseElasticityTensor;
