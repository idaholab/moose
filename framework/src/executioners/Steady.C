//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Steady.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"

registerMooseObject("MooseApp", Steady);

InputParameters
Steady::validParams()
{
  InputParameters params = SteadyBase::validParams();
  params.addClassDescription("Executioner for steady-state simulations.");
  params += FEProblemSolve::validParams();
  return params;
}

Steady::Steady(const InputParameters & parameters) : SteadyBase(parameters), _feproblem_solve(*this)
{
  _fixed_point_solve->setInnerSolve(_feproblem_solve);
}

void
Steady::init()
{
  checkIntegrity();
  _problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _problem.initialSetup();
}

void
Steady::checkIntegrity()
{
  // check to make sure that we don't have any time kernels in this simulation (Steady State)
  for (const auto & system : _feproblem_solve.systemsToSolve())
    if (system->containsTimeKernel())
      mooseError("You have specified time kernels in your steady-state simulation in system ",
                 system->name(),
                 " on the following variables: ",
                 Moose::stringify(system->timeKernelVariableNames()));
}
