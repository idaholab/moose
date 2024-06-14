//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolveFailedInterface.h"

#include "FEProblem.h"

InputParameters
SolveFailedInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

SolveFailedInterface::SolveFailedInterface(const InputParameters & params)
  : _sfi_feproblem(*params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
  _sfi_feproblem.notifyOnSolveFailed(this);
}
