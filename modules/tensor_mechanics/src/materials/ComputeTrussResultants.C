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
  params.addRequiredCoupledVar(
      "area",
      "Cross-section area of the truss. Can be supplied as either a number or a variable name.");
  return params;
}

ComputeTrussResultants::ComputeTrussResultants(const InputParameters & parameters)
  : Material(parameters),
    _area(coupledValue("area")),
    _disp_strain_increment(
        getMaterialPropertyByName<RealVectorValue>("mech_disp_strain_increment")),
    _material_stiffness(getMaterialPropertyByName<Real>("material_stiffness")),
    _axial_stress(declareProperty<Real>("axial_stress")),
    _axial_stress_old(getMaterialPropertyOld<Real>("axial_stress")),
    _force(declareProperty<Real>("forces")),
    _force_old(getMaterialPropertyOld<Real>("forces"))
{
}

void
ComputeTrussResultants::initQpStatefulProperties()
{
  _force[_qp] = 0;
  _axial_stress[_qp] = 0;
}

void
ComputeTrussResultants::computeQpProperties()
{
  _force[_qp] =
      _material_stiffness[_qp] * _disp_strain_increment[_qp](0) * _area[_qp] + _force_old[_qp];
  _axial_stress[_qp] =
      _material_stiffness[_qp] * _disp_strain_increment[_qp](0) + _axial_stress_old[_qp];
}
