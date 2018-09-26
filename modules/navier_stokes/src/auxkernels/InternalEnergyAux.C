//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InternalEnergyAux.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("NavierStokesApp", InternalEnergyAux);

template <>
InputParameters
validParams<InternalEnergyAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("density", "Density (conserved form)");
  params.addRequiredCoupledVar("pressure", "Pressure");
  params.addRequiredParam<UserObjectName>("fp", "The name of the equation of state user object");

  return params;
}

InternalEnergyAux::InternalEnergyAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _pressure(coupledValue("pressure")),
    _rho(coupledValue("density")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

Real
InternalEnergyAux::computeValue()
{
  return _fp.e_from_p_rho(_pressure[_qp], _rho[_qp]);
}
