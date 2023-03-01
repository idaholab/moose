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
#include "MooseObject.h"
#include "MooseApp.h"
#include "FEProblemBase.h"
#include "SolutionInvalidityRegistry.h"

SolutionInvalidInterface::SolutionInvalidInterface(MooseObject * const moose_object)
  : _si_moose_object(*moose_object),
    _si_problem(
        *_si_moose_object.parameters().getCheckedPointerParam<FEProblemBase *>("_fe_problem_base"))
{
}

/// Set solution invalid mark for the given solution ID
void
SolutionInvalidInterface::flagInvalidSolutionInternal(InvalidSolutionID _invalid_solution_id)
{
  auto & solution_invalidity = _si_moose_object.getMooseApp().solutionInvalidity();
  if (_si_problem.immediatelyPrintInvalidSolution())
    solution_invalidity.printDebug(_invalid_solution_id);
  return solution_invalidity.flagInvalidSolutionInternal(_invalid_solution_id);
}

InvalidSolutionID
SolutionInvalidInterface::registerInvalidSolutionInternal(const std::string & message) const
{
  return moose::internal::getSolutionInvalidityRegistry().registerInvalidity(
      _si_moose_object.type(), message);
}
