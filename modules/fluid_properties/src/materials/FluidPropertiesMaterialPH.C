//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidPropertiesMaterialPH.h"

registerMooseObject("FluidPropertiesApp", FluidPropertiesMaterialPH);

InputParameters
FluidPropertiesMaterialPH::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar("pressure", "Fluid pressure (Pa)");
  params.addRequiredCoupledVar("h", "Fluid specific enthalpy (J/kg)");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription("formulation for s(h,p) and rho(v,e)");
  return params;
}

FluidPropertiesMaterialPH::FluidPropertiesMaterialPH(const InputParameters & parameters)
  : Material(parameters),
    _pressure(coupledValue("pressure")),
    _h(coupledValue("h")),

    _s(declareProperty<Real>("s")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

FluidPropertiesMaterialPH::~FluidPropertiesMaterialPH() {}

void
FluidPropertiesMaterialPH::computeQpProperties()
{
  _s[_qp] = _fp.s_from_h_p(_h[_qp], _pressure[_qp]);
}
