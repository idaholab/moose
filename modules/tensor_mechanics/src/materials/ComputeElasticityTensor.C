/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeElasticityTensor.h"
#include "RotationTensor.h"

template<>
InputParameters validParams<ComputeElasticityTensor>()
{
  InputParameters params = validParams<ComputeElasticityTensorBase>();
  params.addClassDescription("Compute an elasticity tensor.");
  params.addRequiredParam<std::vector<Real> >("C_ijkl", "Stiffness tensor for material");
  params.addParam<MooseEnum>("fill_method", RankFourTensor::fillMethodEnum() = "symmetric9", "The fill method");
  params.addParam<Real>("euler_angle_1", 0.0, "Euler angle in direction 1");
  params.addParam<Real>("euler_angle_2", 0.0, "Euler angle in direction 2");
  params.addParam<Real>("euler_angle_3", 0.0, "Euler angle in direction 3");
  return params;
}

ComputeElasticityTensor::ComputeElasticityTensor(const std::string & name,
                                                 InputParameters parameters) :
    ComputeElasticityTensorBase(name, parameters),
    _Euler_angles(getParam<Real>("euler_angle_1"),
                  getParam<Real>("euler_angle_2"),
                  getParam<Real>("euler_angle_3")),
    _Cijkl(getParam<std::vector<Real> >("C_ijkl"), (RankFourTensor::FillMethod)(int)getParam<MooseEnum>("fill_method"))
{
  // Define a rotation according to Euler angle parameters
  RotationTensor R(_Euler_angles); // R type: RealTensorValue

  // rotate elasticity tensor
  _Cijkl.rotate(R);
}

void
ComputeElasticityTensor::computeQpElasticityTensor()
{
  //Assign elasticity tensor at a given quad point
  _elasticity_tensor[_qp] = _Cijkl;
}
