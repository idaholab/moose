/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeElasticityTensorCP.h"
#include "RotationTensor.h"

template <>
InputParameters
validParams<ComputeElasticityTensorCP>()
{
  InputParameters params = validParams<ComputeElasticityTensor>();
  params.addClassDescription("Compute an elasticity tensor for crystal plasticity.");
  params.addParam<UserObjectName>("read_prop_user_object",
                                  "The ElementReadPropertyFile "
                                  "GeneralUserObject to read element "
                                  "specific property values from file");
  return params;
}

ComputeElasticityTensorCP::ComputeElasticityTensorCP(const InputParameters & parameters)
  : ComputeElasticityTensor(parameters),
    _read_prop_user_object(isParamValid("read_prop_user_object")
                               ? &getUserObject<ElementPropertyReadFile>("read_prop_user_object")
                               : NULL),
    _Euler_angles_mat_prop(declareProperty<RealVectorValue>("Euler_angles")),
    _crysrot(declareProperty<RankTwoTensor>("crysrot")),
    _R(_Euler_angles)
{
}

void
ComputeElasticityTensorCP::assignEulerAngles()
{
  if (_read_prop_user_object)
  {
    _Euler_angles_mat_prop[_qp](0) = _read_prop_user_object->getData(_current_elem, 0);
    _Euler_angles_mat_prop[_qp](1) = _read_prop_user_object->getData(_current_elem, 1);
    _Euler_angles_mat_prop[_qp](2) = _read_prop_user_object->getData(_current_elem, 2);
  }
  else
    _Euler_angles_mat_prop[_qp] = _Euler_angles;
}

void
ComputeElasticityTensorCP::computeQpElasticityTensor()
{
  // Properties assigned at the beginning of every call to material calculation
  assignEulerAngles();

  _R.update(_Euler_angles_mat_prop[_qp]);

  _crysrot[_qp] = _R.transpose();
  _elasticity_tensor[_qp] = _Cijkl;
  _elasticity_tensor[_qp].rotate(_crysrot[_qp]);
}
