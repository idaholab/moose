//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSDensityAux.h"

registerMooseObject("NavierStokesApp", NSDensityAux);

InputParameters
NSDensityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  // Coupled variables
  params.addRequiredCoupledVar("pressure", "Coupled fluid pressure variable");
  params.addRequiredCoupledVar("temperature", "Coupled fluid temperature variable");
  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");

  return params;
}

NSDensityAux::NSDensityAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _pressure(coupledValue("pressure")),
    _temperature(coupledValue("temperature")),
    _eos(getUserObject<SinglePhaseFluidProperties>("eos"))
{
}

Real
NSDensityAux::computeValue()
{
  return _eos.rho_from_p_T(_pressure[_qp], _temperature[_qp]);
}
