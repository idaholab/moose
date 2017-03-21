/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "SpecificEnthalpyAux.h"
#include "SinglePhaseFluidProperties.h"

template <>
InputParameters
validParams<SpecificEnthalpyAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("p", "Pressure");
  params.addRequiredCoupledVar("T", "Temperature");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");

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
  return _fp.h(_pressure[_qp], _temperature[_qp]);
}
