/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PFCElementEnergyIntegral.h"

template <>
InputParameters
validParams<PFCElementEnergyIntegral>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addRequiredParam<VariableName>("variable",
                                        "The name of the variable that this object operates on");
  params.addParam<Real>("temp", 1833.0, "Temperature of simulation");
  return params;
}

PFCElementEnergyIntegral::PFCElementEnergyIntegral(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    MooseVariableInterface(this, false),
    _var(_subproblem.getVariable(_tid, parameters.get<VariableName>("variable"))),
    _u(_var.sln()),
    _grad_u(_var.gradSln()),
    _u_dot(_var.uDot()),
    _temp(getParam<Real>("temp")) // K
{
  addMooseVariableDependency(mooseVariable());
}

Real
PFCElementEnergyIntegral::computeQpIntegral()
{
  // const Real kb = 1.3806488e-23;  // A^2 kg s^-2 K^-1
  // const Real p0 = 0.0801; // A^-3

  return _u[_qp]; // * (kb * _temp);
}
