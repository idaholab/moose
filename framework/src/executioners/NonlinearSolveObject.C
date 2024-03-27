//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonlinearSolveObject.h"
#include "FEProblemBase.h"

InputParameters
NonlinearSolveObject::validParams()
{
  return emptyInputParameters();
}

NonlinearSolveObject::NonlinearSolveObject(Executioner & ex)
  : SolveObject(ex), _nl(_problem.getNonlinearSystemBase(/*nl_sys=*/0))
{
}
