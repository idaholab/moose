#include "ComputeInverseElasticityTensor.h"
#include "RotationTensor.h"

registerMooseObject("TensorMechanicsApp", ComputeInverseElasticityTensor);
registerMooseObject("TensorMechanicsApp", ADComputeInverseElasticityTensor);

template <bool is_ad>
InputParameters
ComputeInverseElasticityTensorTempl<is_ad>::validParams()
{
  InputParameters params = ComputeInverseRotatedElasticityTensorBaseTempl<is_ad>::validParams();
  params.addClassDescription("Compute an compliance tensor.");
  params.addRequiredParam<std::vector<Real>>("C_ijkl", "Stiffness tensor for material");
  params.addParam<MooseEnum>(
      "fill_method", RankFourTensor::fillMethodEnum() = "symmetric9", "The fill method");
  return params;
}

template <bool is_ad>
ComputeInverseElasticityTensorTempl<is_ad>::ComputeInverseElasticityTensorTempl(
    const InputParameters & parameters)
  : ComputeInverseRotatedElasticityTensorBaseTempl<is_ad>(parameters),
    _Cijkl(this->template getParam<std::vector<Real>>("C_ijkl"),
           (RankFourTensor::FillMethod)(int)this->template getParam<MooseEnum>("fill_method"))
{
}

template <bool is_ad>
void
ComputeInverseElasticityTensorTempl<is_ad>::computeQpElasticityTensor()
{
  // Assign compliance tensor at a given quad point
  // This code is made for symmetric9

  _Sijkl = _Cijkl.invSymm();

  _elasticity_tensor[_qp] = _Cijkl;
  _compliance_tensor[_qp] = _Sijkl;
}

template class ComputeInverseElasticityTensorTempl<false>;
template class ComputeInverseElasticityTensorTempl<true>;
