/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeConcentrationDependentElasticityTensor.h"
#include "RotationTensor.h"

template <>
InputParameters
validParams<ComputeConcentrationDependentElasticityTensor>()
{
  InputParameters params = validParams<ComputeRotatedElasticityTensorBase>();
  params.addClassDescription("Compute concentration dependent elasticity tensor.");
  params.addRequiredParam<std::vector<Real>>("C0_ijkl",
                                             "Stiffness tensor for zero concentration phase");
  params.addRequiredParam<std::vector<Real>>("C1_ijkl",
                                             "Stiffness tensor for phase having concentration 1.0");
  params.addParam<MooseEnum>(
      "fill_method0", RankFourTensor::fillMethodEnum() = "symmetric9", "The fill method");
  params.addParam<MooseEnum>(
      "fill_method1", RankFourTensor::fillMethodEnum() = "symmetric9", "The fill method");
  params.addRequiredCoupledVar("c", "Concentration");
  return params;
}

ComputeConcentrationDependentElasticityTensor::ComputeConcentrationDependentElasticityTensor(
    const InputParameters & parameters)
  : ComputeRotatedElasticityTensorBase(parameters),
    _Cijkl0(getParam<std::vector<Real>>("C0_ijkl"),
            (RankFourTensor::FillMethod)(int)getParam<MooseEnum>("fill_method0")),
    _Cijkl1(getParam<std::vector<Real>>("C1_ijkl"),
            (RankFourTensor::FillMethod)(int)getParam<MooseEnum>("fill_method1")),
    _c(coupledValue("c")),
    _c_name(getVar("c", 0)->name()),
    _delasticity_tensor_dc(
        declarePropertyDerivative<RankFourTensor>(_elasticity_tensor_name, _c_name))
{
  // Define a rotation according to Euler angle parameters
  RotationTensor R(_Euler_angles); // R type: RealTensorValue

  // Rotate tensors
  _Cijkl0.rotate(R);
  _Cijkl1.rotate(R);
}

void
ComputeConcentrationDependentElasticityTensor::computeQpElasticityTensor()
{
  // Assign elasticity tensor at a given quad point
  _elasticity_tensor[_qp] = _Cijkl0 + (_Cijkl1 - _Cijkl0) * _c[_qp];
  // Define derivative of elasticity tensor with respect to concentration.
  _delasticity_tensor_dc[_qp] = (_Cijkl1 - _Cijkl0);
}
