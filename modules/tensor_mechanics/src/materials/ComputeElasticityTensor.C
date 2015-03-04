/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeElasticityTensor.h"

template<>
InputParameters validParams<ComputeElasticityTensor>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Compute an elasticity tensor.");
  params.addRequiredParam<std::vector<Real> >("C_ijkl", "Stiffness tensor for material");
  params.addParam<MooseEnum>("fill_method", RankFourTensor::fillMethodEnum() = "symmetric9", "The fill method");
  params.addParam<Real>("euler_angle_1", 0.0, "Euler angle in direction 1");
  params.addParam<Real>("euler_angle_2", 0.0, "Euler angle in direction 2");
  params.addParam<Real>("euler_angle_3", 0.0, "Euler angle in direction 3");
  params.addParam<FunctionName>("elasticity_tensor_prefactor", "Optional function to use as a scalar prefactor on the elasticity tensor.");
  params.addParam<std::string>("base_name", "Optional parameter that allows the user to define multiple mechanics material systems on the same block, i.e. for multiple phases");
  return params;
}

ComputeElasticityTensor::ComputeElasticityTensor(const std::string & name,
                                                 InputParameters parameters) :
    DerivativeMaterialInterface<Material>(name, parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _elasticity_tensor(declareProperty<ElasticityTensorR4>(_base_name + "elasticity_tensor")),
    _Euler_angles(getParam<Real>("euler_angle_1"),
                  getParam<Real>("euler_angle_2"),
                  getParam<Real>("euler_angle_3")),
    _Cijkl(getParam<std::vector<Real> >("C_ijkl"), (RankFourTensor::FillMethod)(int)getParam<MooseEnum>("fill_method")),
    _prefactor_function(isParamValid("elasticity_tensor_prefactor") ? &getFunction("elasticity_tensor_prefactor") : NULL)
{
}

void
ComputeElasticityTensor::computeQpProperties()
{
  // Define a rotation according to Euler angle parameters
  RotationTensor R(_Euler_angles); // R type: RealTensorValue

  //Assign elasticity tensor at a given quad point
  _elasticity_tensor[_qp] = _Cijkl;

  //Multiply by prefactor
  if (_prefactor_function)
    _elasticity_tensor[_qp] *= _prefactor_function->value(_t, _q_point[_qp]);

  //Rotate tensor
  _elasticity_tensor[_qp].rotate(R);
}
