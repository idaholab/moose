//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeTrussResultants.h"

registerMooseObject("TensorMechanicsApp", ComputeTrussResultants);

InputParameters
ComputeTrussResultants::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute forces and moments using elasticity");
  // params.addRequiredCoupledVar(
  //     "area",
  //     "Cross-section area of the truss. Can be supplied as either a number or a variable name.");
  return params;
}

ComputeTrussResultants::ComputeTrussResultants(const InputParameters & parameters)
  : Material(parameters),
    // _area(coupledValue("area")),
    _disp_strain_increment(
        getMaterialPropertyByName<RealVectorValue>("mech_disp_strain_increment")),
    _material_stiffness(getMaterialPropertyByName<RealVectorValue>("material_stiffness")),
    _total_rotation(getMaterialPropertyByName<RankTwoTensor>("total_rotation")),
    _force(declareProperty<RealVectorValue>("forces")),
    _force_old(getMaterialPropertyOld<RealVectorValue>("forces"))
{
}

void
ComputeTrussResultants::initQpStatefulProperties()
{
  _force[_qp].zero();
}

void
ComputeTrussResultants::computeQpProperties()
{
  // force = R^T * _material_stiffness * strain_increment + force_old
  RealVectorValue force_increment;
  force_increment(0) = _material_stiffness[_qp](0) * _disp_strain_increment[_qp](0);
  force_increment(1) = _material_stiffness[_qp](1) * _disp_strain_increment[_qp](1);
  force_increment(2) = _material_stiffness[_qp](2) * _disp_strain_increment[_qp](2);

  // out << " ComputeTrussResultants force increment " << force_increment(0) << " " << force_increment(1) << " " << force_increment(2) << " _material_stiffness "<< _material_stiffness[_qp](0) << " " << _material_stiffness[_qp](1) << " " << _material_stiffness[_qp](2) << std::endl;

  _force[_qp] = _total_rotation[0].transpose() * force_increment + _force_old[_qp];
}
