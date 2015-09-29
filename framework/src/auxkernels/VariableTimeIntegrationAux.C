#include "VariableTimeIntegrationAux.h"

template<>
InputParameters validParams<VariableTimeIntegrationAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("variable_to_integrate", "The variable to be integrated");
  params.addParam<Real>("coefficient", 1.0, "A simple coefficient");
  return params;
}

VariableTimeIntegrationAux::VariableTimeIntegrationAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _coupled_var(coupledValue("variable_to_integrate")),
    _coupled_var_old(coupledValueOld("variable_to_integrate")),
    _coef(getParam<Real>("coefficient"))
{
  // Note: initial value should be set by an initial condition.
}

Real
VariableTimeIntegrationAux::computeValue()
{
  return _u_old[_qp] +0.5*_coef* (_coupled_var[_qp] + _coupled_var_old[_qp]) * _dt;
}

