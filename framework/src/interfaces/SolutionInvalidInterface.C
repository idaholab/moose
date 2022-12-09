//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SolutionInvalidInterface.h"
#include "MooseApp.h"
#include "NonlinearSystemBase.h"
#include "FEProblemBase.h"
#include "MooseObject.h"

SolutionInvalidInterface::SolutionInvalidInterface(MooseObject * moose_object)
  : _si_fe_problem(
        *moose_object->parameters().getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
}

void
SolutionInvalidInterface::setSolutionInvalid(bool solution_invalid)
{
  _si_fe_problem.getNonlinearSystemBase().setSolutionInvalid(solution_invalid);
}
