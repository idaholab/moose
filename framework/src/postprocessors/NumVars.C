//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NumVars.h"

#include "AuxiliarySystem.h"
#include "NonlinearSystemBase.h"
#include "SubProblem.h"

template <>
InputParameters
validParams<NumVars>()
{
  InputParameters params = validParams<GeneralPostprocessor>();

  MooseEnum system_options("nonlinear auxiliary", "nonlinear");
  params.addParam<MooseEnum>(
      "system", system_options, "The system for which you want to print the number of variables.");

  return params;
}

NumVars::NumVars(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _system(getParam<MooseEnum>("system"))
{
}

Real
NumVars::getValue()
{
  switch (_system)
  {
    case 0:
      return _fe_problem.getNonlinearSystemBase().system().n_vars();
    case 1:
      return _fe_problem.getAuxiliarySystem().sys().n_vars();
  }

  mooseError("Unknown system type!");
}
