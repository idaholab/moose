#include "THMSetupQuadratureAction.h"

registerMooseAction("THMApp", THMSetupQuadratureAction, "setup_quadrature");

template <>
InputParameters
validParams<THMSetupQuadratureAction>()
{
  InputParameters params = validParams<THMAction>();
  return params;
}

THMSetupQuadratureAction::THMSetupQuadratureAction(InputParameters parameters)
  : THMAction(parameters)
{
}

void
THMSetupQuadratureAction::act()
{
  _simulation.setupQuadrature();
}
