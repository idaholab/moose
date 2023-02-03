//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeElasticityTensorCP.h"
#include "RotationTensor.h"

registerMooseObject("TensorMechanicsApp", ComputeElasticityTensorCP);

InputParameters
ComputeElasticityTensorCP::validParams()
{
  InputParameters params = ComputeElasticityTensor::validParams();
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
                               ? &getUserObject<PropertyReadFile>("read_prop_user_object")
                               : nullptr),
    _Euler_angles_mat_prop(declareProperty<RealVectorValue>("Euler_angles")),
    _crysrot(declareProperty<RankTwoTensor>(_base_name + "crysrot")),
    _R(_Euler_angles)
{
  // the base class guarantees constant in time, but in this derived class the
  // tensor will rotate over time once plastic deformation sets in
  revokeGuarantee(_elasticity_tensor_name, Guarantee::CONSTANT_IN_TIME);

  // the base class performs a passive rotation, but the crystal plasticity
  // materials use active rotation: recover unrotated _Cijkl here
  _Cijkl.rotate(_R.transpose());
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
ComputeElasticityTensorCP::initQpStatefulProperties()
{
  // Properties assigned at the beginning of every call to material calculation
  assignEulerAngles();
  _R.update(_Euler_angles_mat_prop[_qp]);

  _crysrot[_qp] = _R.transpose();
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
