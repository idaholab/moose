//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMGcIntegral.h"

registerMooseObject("ShiftedBoundaryMethodApp", CZMGcIntegral);

InputParameters
CZMGcIntegral::validParams()
{
  InputParameters params = InterfaceMaterial::validParams();
  params.addClassDescription("Do the integration of traction multiplied by effective jump.");
  return params;
}

CZMGcIntegral::CZMGcIntegral(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _interface_traction_new(getADMaterialPropertyByName<RealVectorValue>("interface_traction")),
    _interface_traction_old(getMaterialPropertyOldByName<RealVectorValue>("interface_traction")),
    _interface_effective_displacement_jump_new(
        getADMaterialPropertyByName<RealVectorValue>("interface_effective_displacement_jump")),
    _interface_effective_displacement_jump_old(
        getMaterialPropertyOldByName<RealVectorValue>("interface_effective_displacement_jump")),
    _gc_integral(declarePropertyByName<Real>("gc_integral")),
    _gc_integral_old(getMaterialPropertyOldByName<Real>("gc_integral"))
{
}

void
CZMGcIntegral::initQpStatefulProperties()
{
  _gc_integral[_qp] = 0;
}

void
CZMGcIntegral::computeQpProperties()
{
  const auto gc_integral_inc = 0.5 * (_interface_traction_new[_qp] + _interface_traction_old[_qp]) *
                               (_interface_effective_displacement_jump_new[_qp] -
                                _interface_effective_displacement_jump_old[_qp]);

  _gc_integral[_qp] = _gc_integral_old[_qp] + gc_integral_inc.value();
}
