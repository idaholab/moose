//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionSeries.h"
#include "FunctionSeriesToAux.h"

registerMooseObject("FunctionalExpansionToolsApp", FunctionSeriesToAux);

InputParameters
FunctionSeriesToAux::validParams()
{
  InputParameters params = FunctionAux::validParams();

  params.addClassDescription("AuxKernel to convert a functional expansion"
                             " (Functions object, type = FunctionSeries) to an AuxVariable");

  // Force this AuxKernel to execute at "timestep_begin"
  params.set<ExecFlagEnum>("execute_on", true) = EXEC_TIMESTEP_BEGIN;
  // Don't let the user change the execution time
  params.suppressParameter<ExecFlagEnum>("execute_on");

  return params;
}

FunctionSeriesToAux::FunctionSeriesToAux(const InputParameters & parameters)
  : FunctionAux(parameters)
{
  FunctionSeries::checkAndConvertFunction(_func, getParam<std::string>("_moose_base"), name());
}
