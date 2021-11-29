//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableTimeIntegrationAux.h"

registerMooseObject("MooseApp", VariableTimeIntegrationAux);

InputParameters
VariableTimeIntegrationAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Integrates a field variable in time.");
  params.addRequiredCoupledVar("variable_to_integrate", "The variable to be integrated");
  params.addParam<Real>("coefficient", 1.0, "A simple coefficient multiplying the integral");
  params.addParam<unsigned int>(
      "order", 2, "The order of global truncation error: midpoint=1, trapezoidal=2, Simpson=3");
  return params;
}

VariableTimeIntegrationAux::VariableTimeIntegrationAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _coef(getParam<Real>("coefficient")),
    _order(getParam<unsigned int>("order")),
    _u_old(_order != 3 ? uOld() : genericZeroValue<false>()),
    _u_older(_order == 3 ? uOlder() : genericZeroValue<false>())
{
  switch (_order)
  {
    case 1:
      _integration_coef.push_back(1.0);
      _coupled_vars.push_back(&coupledValue("variable_to_integrate"));
      break;
    case 2:
      _integration_coef.push_back(0.5);
      _integration_coef.push_back(0.5);
      _coupled_vars.push_back(&coupledValue("variable_to_integrate"));
      _coupled_vars.push_back(&coupledValueOld("variable_to_integrate"));
      break;
    case 3:
      _integration_coef.push_back(1.0 / 3.0);
      _integration_coef.push_back(4.0 / 3.0);
      _integration_coef.push_back(1.0 / 3.0);
      _coupled_vars.push_back(&coupledValue("variable_to_integrate"));
      _coupled_vars.push_back(&coupledValueOld("variable_to_integrate"));
      _coupled_vars.push_back(&coupledValueOlder("variable_to_integrate"));
      break;
    default:
      mooseError("VariableTimeIntegrationAux: unknown time integration order specified");
  }
}

Real
VariableTimeIntegrationAux::computeValue()
{
  Real integral = getIntegralValue();

  if (_order == 3)
    return _u_older[_qp] + _coef * integral;

  return _u_old[_qp] + _coef * integral;
}

Real
VariableTimeIntegrationAux::getIntegralValue()
{
  Real integral_value = 0.0;

  for (unsigned int i = 0; i < _order; ++i)
    integral_value += _integration_coef[i] * (*_coupled_vars[i])[_qp] * _dt;

  /**
   * Subsequent timesteps may be unequal, so the standard Simpson rule
   * cannot be used. Use a different set of coefficients here.
   * J. McNAMEE, "A PROGRAM TO INTEGRATE A FUNCTION TABULATED AT
   * UNEQUAL INTERVALS," Internation Journal for Numerical Methods in
   * Engineering, Vol. 17, 217-279. (1981).
   */
  if (_order == 3 && _dt != _dt_old)
  {
    Real x0 = 0.0;
    Real x1 = _dt_old;
    Real x2 = _dt + _dt_old;
    Real y0 = (*_coupled_vars[2])[_qp];
    Real y1 = (*_coupled_vars[1])[_qp];
    Real y2 = (*_coupled_vars[0])[_qp];
    Real term1 = (x2 - x0) * (y0 + (x2 - x0) * (y1 - y0) / (2.0 * (x1 - x0)));
    Real term2 = (2.0 * x2 * x2 - x0 * x2 - x0 * x0 + 3.0 * x0 * x1 - 3.0 * x1 * x2) / 6.0;
    Real term3 = (y2 - y1) / (x2 - x1) - (y1 - y0) / (x1 - x0);
    integral_value = term1 + term2 * term3;
  }

  return integral_value;
}
