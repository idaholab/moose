//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVariableTimeIntegrationAux.h"

registerKokkosAuxKernel("MooseApp", KokkosVariableTimeIntegrationAux);

InputParameters
KokkosVariableTimeIntegrationAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Integrates a field variable in time.");
  params.addRequiredCoupledVar("variable_to_integrate", "The variable to be integrated");
  params.addParam<Real>("coefficient", 1.0, "A simple coefficient multiplying the integral");
  params.addParam<unsigned int>(
      "order", 2, "The order of global truncation error: midpoint=1, trapezoidal=2, Simpson=3");
  return params;
}

KokkosVariableTimeIntegrationAux::KokkosVariableTimeIntegrationAux(
    const InputParameters & parameters)
  : AuxKernel(parameters),
    _coef(getParam<Real>("coefficient")),
    _order(getParam<unsigned int>("order")),
    _u_old(_order != 3 ? uOld() : kokkosZeroValue()),
    _u_older(_order == 3 ? uOlder() : kokkosZeroValue())
{
  _integration_coef.create(_order);
  _coupled_vars.create(_order);

  switch (_order)
  {
    case 1:
      _integration_coef[0] = 1.0;
      _coupled_vars[0] = kokkosCoupledValue("variable_to_integrate");
      break;
    case 2:
      _integration_coef[0] = 0.5;
      _integration_coef[1] = 0.5;
      _coupled_vars[0] = kokkosCoupledValue("variable_to_integrate");
      _coupled_vars[1] = kokkosCoupledValueOld("variable_to_integrate");
      break;
    case 3:
      _integration_coef[0] = 1.0 / 3.0;
      _integration_coef[1] = 4.0 / 3.0;
      _integration_coef[2] = 1.0 / 3.0;
      _coupled_vars[0] = kokkosCoupledValue("variable_to_integrate");
      _coupled_vars[1] = kokkosCoupledValueOld("variable_to_integrate");
      _coupled_vars[2] = kokkosCoupledValueOlder("variable_to_integrate");
      break;
    default:
      mooseError("KokkosVariableTimeIntegrationAux: unknown time integration order specified");
  }

  _integration_coef.copyToDevice();
  _coupled_vars.copyToDevice();
}
