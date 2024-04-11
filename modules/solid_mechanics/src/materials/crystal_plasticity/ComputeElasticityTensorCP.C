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

registerMooseObject("SolidMechanicsApp", ComputeElasticityTensorCP);

InputParameters
ComputeElasticityTensorCP::validParams()
{
  InputParameters params = ComputeElasticityTensor::validParams();
  params.addClassDescription("Compute an elasticity tensor for crystal plasticity.");
  params.addParam<UserObjectName>("read_prop_user_object",
                                  "The ElementReadPropertyFile "
                                  "GeneralUserObject to read element "
                                  "specific property values from file");
  params.addCoupledVar("euler_angle_variables",
                       "Vector of coupled variables representing the Euler angles' components.");
  return params;
}

ComputeElasticityTensorCP::ComputeElasticityTensorCP(const InputParameters & parameters)
  : ComputeElasticityTensor(parameters),
    _read_prop_user_object(isParamValid("read_prop_user_object")
                               ? &getUserObject<PropertyReadFile>("read_prop_user_object")
                               : nullptr),
    _Euler_angles_mat_prop(declareProperty<RealVectorValue>("Euler_angles")),
    _crysrot(declareProperty<RankTwoTensor>(_base_name + "crysrot")),
    _R(_Euler_angles),
    _n_euler_angle_vars(coupledComponents("euler_angle_variables")),
    _euler_angle_vars(coupledValues("euler_angle_variables"))
{
  // the base class guarantees constant in time, but in this derived class the
  // tensor will rotate over time once plastic deformation sets in
  revokeGuarantee(_elasticity_tensor_name, Guarantee::CONSTANT_IN_TIME);

  // the base class performs a passive rotation, but the crystal plasticity
  // materials use active rotation: recover unrotated _Cijkl here
  if (parameters.isParamValid("rotation_matrix"))
  {
    _user_provided_rotation_matrix = true;
    _Cijkl.rotate(_rotation_matrix.transpose());
  }
  else
  {
    _user_provided_rotation_matrix = false;
    _Cijkl.rotate(_R.transpose());
  }

  if (_user_provided_rotation_matrix &&
      (_read_prop_user_object || (parameters.isParamSetByUser("euler_angle_1")) ||
       (parameters.isParamSetByUser("euler_angle_2")) ||
       (parameters.isParamSetByUser("euler_angle_3"))))
    mooseError("Bunge Euler angle information and the rotation_matrix cannot both be specified. "
               "Provide only one type of orientation information in the input file.");

  // Check if source of Euler angle values has a conflict
  if (_read_prop_user_object && _n_euler_angle_vars)
    paramError("euler_angle_variables",
               "Euler angles cannot be supplied from both coupled variables and auxiliary input "
               "file in the option `read_prop_user_object`.");

  if (isCoupled("euler_angle_variables") && _n_euler_angle_vars != 3)
    paramError("euler_angle_variables", "The Euler angles should have three components.");
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
  else if (_n_euler_angle_vars)
  {
    _Euler_angles_mat_prop[_qp](0) = (*_euler_angle_vars[0])[_qp];
    _Euler_angles_mat_prop[_qp](1) = (*_euler_angle_vars[1])[_qp];
    _Euler_angles_mat_prop[_qp](2) = (*_euler_angle_vars[2])[_qp];
  }
  else
    _Euler_angles_mat_prop[_qp] = _Euler_angles;

  _R.update(_Euler_angles_mat_prop[_qp]);
}

void
ComputeElasticityTensorCP::computeQpElasticityTensor()
{
  // Properties assigned at the beginning of every call to material calculation
  // is required by the monolithic and user object versions. If those classes
  // are deprecated, these update can be removed and save time
  if (!_user_provided_rotation_matrix)
  {
    assignEulerAngles();
    _crysrot[_qp] = _R.transpose();
  }
  else
    _crysrot[_qp] = _rotation_matrix.transpose();

  _elasticity_tensor[_qp] = _Cijkl;
  _elasticity_tensor[_qp].rotate(_crysrot[_qp]);
}
