/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeElasticityTensor.h"
#include "RotationTensor.h"

template <>
InputParameters
validParams<ComputeElasticityTensor>()
{
  InputParameters params = validParams<ComputeRotatedElasticityTensorBase>();
  params.addClassDescription("Compute an elasticity tensor.");
  params.addRequiredParam<std::vector<Real>>("C_ijkl", "Stiffness tensor for material");
  params.addParam<MooseEnum>(
      "fill_method", RankFourTensor::fillMethodEnum() = "symmetric9", "The fill method");
  return params;
}

ComputeElasticityTensor::ComputeElasticityTensor(const InputParameters & parameters)
  : ComputeRotatedElasticityTensorBase(parameters),
    _Cijkl(getParam<std::vector<Real>>("C_ijkl"),
           (RankFourTensor::FillMethod)(int)getParam<MooseEnum>("fill_method"))
{
  // Define a rotation according to Euler angle parameters
  RotationTensor R(_Euler_angles); // R type: RealTensorValue

  // rotate elasticity tensor
  _Cijkl.rotate(R);
}

void
ComputeElasticityTensor::computeQpElasticityTensor()
{
  // Assign elasticity tensor at a given quad point
  _elasticity_tensor[_qp] = _Cijkl;
}
