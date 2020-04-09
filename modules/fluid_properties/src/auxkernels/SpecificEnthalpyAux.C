//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SpecificEnthalpyAux.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", SpecificEnthalpyAux);

InputParameters
SpecificEnthalpyAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("p", "Pressure");
  params.addRequiredCoupledVar("T", "Temperature");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addClassDescription("Computes specific enthalpy from pressure and temperature");
  return params;
}

SpecificEnthalpyAux::SpecificEnthalpyAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _pressure(coupledValue("p")),
    _temperature(coupledValue("T")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
SpecificEnthalpyAux::computeValue()
{
  return _fp.h_from_p_T(_pressure[_qp], _temperature[_qp]);
}
