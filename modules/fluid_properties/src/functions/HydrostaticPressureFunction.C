//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HydrostaticPressureFunction.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", HydrostaticPressureFunction);

InputParameters
HydrostaticPressureFunction::validParams()
{
  InputParameters params = Function::validParams();
  params += GravityVectorInterface::validParams();

  params.addClassDescription("Computes hydrostatic pressure from a reference point.");

  params.addRequiredParam<Real>("reference_pressure", "Pressure at the reference point [Pa]");
  params.addRequiredParam<Real>("reference_temperature",
                                "Reference temperature for density evaluation [K]");
  params.addRequiredParam<Point>("reference_point",
                                 "Reference point where pressure is specified [m]");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name of the SinglePhaseFluidProperties user object");

  return params;
}

HydrostaticPressureFunction::HydrostaticPressureFunction(const InputParameters & parameters)
  : Function(parameters),
    GravityVectorInterface(this),
    _p_ref(getParam<Real>("reference_pressure")),
    _T_ref(getParam<Real>("reference_temperature")),
    _r_ref(getParam<Point>("reference_point"))
{
}

void
HydrostaticPressureFunction::initialSetup()
{
  Function::initialSetup();

  const auto & fp = getUserObject<SinglePhaseFluidProperties>("fluid_properties");
  _rho = fp.rho_from_p_T(_p_ref, _T_ref);
}

Real
HydrostaticPressureFunction::value(Real /*t*/, const Point & r) const
{
  return _p_ref - _rho * gravityVector() * (_r_ref - r);
}
