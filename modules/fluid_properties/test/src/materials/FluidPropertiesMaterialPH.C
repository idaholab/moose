//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidPropertiesMaterialPH.h"

registerMooseObject("FluidPropertiesTestApp", FluidPropertiesMaterialPH);

InputParameters
FluidPropertiesMaterialPH::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("pressure", "Fluid pressure (Pa)");
  params.addRequiredCoupledVar("h", "Fluid specific enthalpy (J/kg)");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription(
      "Fluid properties using the (pressure, specific enthalpy) formulation");
  return params;
}

FluidPropertiesMaterialPH::FluidPropertiesMaterialPH(const InputParameters & parameters)
  : Material(parameters),
    _pressure(coupledValue("pressure")),
    _h(coupledValue("h")),

    _T(declareProperty<Real>("T")),
    _s(declareProperty<Real>("s")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

FluidPropertiesMaterialPH::~FluidPropertiesMaterialPH() {}

void
FluidPropertiesMaterialPH::computeQpProperties()
{
  _T[_qp] = _fp.T_from_p_h(_pressure[_qp], _h[_qp]);
  _s[_qp] = _fp.s_from_h_p(_h[_qp], _pressure[_qp]);
}
