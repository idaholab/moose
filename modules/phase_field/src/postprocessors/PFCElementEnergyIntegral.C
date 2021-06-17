//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PFCElementEnergyIntegral.h"

// MOOSE includes
#include "MooseVariable.h"

registerMooseObject("PhaseFieldApp", PFCElementEnergyIntegral);

InputParameters
PFCElementEnergyIntegral::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addRequiredParam<VariableName>("variable",
                                        "The name of the variable that this object operates on");
  params.addParam<Real>("temp", 1833.0, "Temperature of simulation");
  return params;
}

PFCElementEnergyIntegral::PFCElementEnergyIntegral(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _var(_subproblem.getStandardVariable(_tid, parameters.get<VariableName>("variable"))),
    _u(_var.sln()),
    _grad_u(_var.gradSln()),
    _u_dot(_var.uDot()),
    _temp(getParam<Real>("temp")) // K
{
  addMooseVariableDependency(&mooseVariableField());
}

Real
PFCElementEnergyIntegral::computeQpIntegral()
{
  // const Real kb = 1.3806488e-23;  // A^2 kg s^-2 K^-1
  // const Real p0 = 0.0801; // A^-3

  return _u[_qp]; // * (kb * _temp);
}
