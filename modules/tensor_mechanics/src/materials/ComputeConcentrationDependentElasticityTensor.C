/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeConcentrationDependentElasticityTensor.h"
#include "RotationTensor.h"

template<>
InputParameters validParams<ComputeConcentrationDependentElasticityTensor>()
{
  InputParameters params = validParams<ComputeElasticityTensor>();
  params.addClassDescription("Compute concentration dependent elasticity tensor.");
  params.suppressParameter<std::vector<Real> >("C_ijkl");
  params.suppressParameter<MooseEnum>("fill_method");
  params.addRequiredParam<std::vector<Real> >("C0_ijkl", "Stiffness tensor for zero concentration phase");
  params.addRequiredParam<std::vector<Real> >("C1_ijkl", "Stiffness tensor for phase having concentration 1.0");
  params.addParam<MooseEnum>("fill_method_phase0", RankFourTensor::fillMethodEnum() = "symmetric9", "The fill method");
  params.addParam<MooseEnum>("fill_method_phase1", RankFourTensor::fillMethodEnum() = "symmetric9", "The fill method");
  params.addRequiredCoupledVar("c", "Concentration");
  return params;
}

ComputeConcentrationDependentElasticityTensor::ComputeConcentrationDependentElasticityTensor(const std::string & name,
                                                                                             InputParameters parameters) :
    ComputeElasticityTensor(name, parameters),
    _Cijkl0(getParam<std::vector<Real> >("C0_ijkl"), (RankFourTensor::FillMethod)(int)getParam<MooseEnum>("fill_method_phase0")),
    _Cijkl1(getParam<std::vector<Real> >("C1_ijkl"), (RankFourTensor::FillMethod)(int)getParam<MooseEnum>("fill_method_phase1")),
    _c(coupledValue("c")),
    _c_name(getVar("c", 0)->name()),
    _delasticity_tensor_dc(declarePropertyDerivative<ElasticityTensorR4>(_elasticity_tensor_name, _c_name))
{
  // Define a rotation according to Euler angle parameters
  RotationTensor R(_Euler_angles); // R type: RealTensorValue

  // Rotate tensors
  C0_ijkl.rotate(R);
  C1_ijkl.rotate(R);
}

void
ComputeConcentrationDependentElasticityTensor::computeQpElasticityTensor()
{
  // Assign elasticity tensor at a given quad point
  _elasticity_tensor[_qp] = _Cijkl0 + (_Cijkl1 - _Cijkl0)*_c[_qp];
  _delasticity_tensor_dc[_qp] = (_Cijkl1 - _Cijkl0);
}
