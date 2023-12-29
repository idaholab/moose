//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeBeamResultants.h"

registerMooseObject("TensorMechanicsApp", ComputeBeamResultants);

InputParameters
ComputeBeamResultants::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute forces and moments using elasticity");
  return params;
}

ComputeBeamResultants::ComputeBeamResultants(const InputParameters & parameters)
  : Material(parameters),
    _disp_strain_increment(
        getMaterialPropertyByName<RealVectorValue>("mech_disp_strain_increment")),
    _rot_strain_increment(getMaterialPropertyByName<RealVectorValue>("mech_rot_strain_increment")),
    _material_stiffness(getMaterialPropertyByName<RealVectorValue>("material_stiffness")),
    _material_flexure(getMaterialPropertyByName<RealVectorValue>("material_flexure")),
    _total_rotation(getMaterialPropertyByName<RankTwoTensor>("total_rotation")),
    _force(declareProperty<RealVectorValue>("forces")),
    _moment(declareProperty<RealVectorValue>("moments")),
    _force_old(getMaterialPropertyOld<RealVectorValue>("forces")),
    _moment_old(getMaterialPropertyOld<RealVectorValue>("moments"))
{
}

void
ComputeBeamResultants::initQpStatefulProperties()
{
  _force[_qp].zero();
  _moment[_qp].zero();
}

void
ComputeBeamResultants::computeQpProperties()
{
  // force = R^T * _material_stiffness * strain_increment + force_old
  RealVectorValue force_increment;
  force_increment(0) = _material_stiffness[_qp](0) * _disp_strain_increment[_qp](0);
  force_increment(1) = _material_stiffness[_qp](1) * _disp_strain_increment[_qp](1);
  force_increment(2) = _material_stiffness[_qp](2) * _disp_strain_increment[_qp](2);

  _force[_qp] = _total_rotation[0].transpose() * force_increment + _force_old[_qp];

  // moment = R^T * _material_flexure * rotation_increment + moment_old
  RealVectorValue moment_increment;
  moment_increment(0) = _material_flexure[_qp](0) * _rot_strain_increment[_qp](0);
  moment_increment(1) = _material_flexure[_qp](1) * _rot_strain_increment[_qp](1);
  moment_increment(2) = _material_flexure[_qp](2) * _rot_strain_increment[_qp](2);

  _moment[_qp] = _total_rotation[0].transpose() * moment_increment + _moment_old[_qp];
}
