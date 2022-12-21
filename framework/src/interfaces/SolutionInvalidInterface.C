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
#include "MooseObject.h"
#include "SolutionInvalidityRegistry.h"
#include "FEProblemBase.h"

SolutionInvalidInterface::SolutionInvalidInterface(MooseApp & moose_app, FEProblemBase & problem)
  : ConsoleStreamInterface(moose_app), _si_moose_app(moose_app), _si_problem(problem)
{
}

/// Set solution invalid mark for the given solution ID
void
SolutionInvalidInterface::setSolutionInvalid(SolutionID _solution_id)
{
  if (_si_problem.ImmediatelyPrintInvalidSolution())
    _si_moose_app.solutionInvalidity().printDebug(_console, _solution_id);
  return _si_moose_app.solutionInvalidity().setSolutionInvalid(_solution_id);
}

/// Register the section with a unique solution ID for the given section_name
SolutionID
SolutionInvalidInterface::registerInvalidSection(const std::string & section_name,
                                                 const std::string & message) const
{
  return moose::internal::getSolutionInvalidityRegistry().registerSection(section_name, message);
}

SolutionID
SolutionInvalidInterface::registerInvalidSection(const std::string & section_name) const
{
  return moose::internal::getSolutionInvalidityRegistry().registerSection(section_name);
}

SolutionInvalidity &
SolutionInvalidInterface::solutionInvalidity()
{
  return _si_moose_app.solutionInvalidity();
}
