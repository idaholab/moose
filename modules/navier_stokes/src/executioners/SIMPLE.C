//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SIMPLE.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"

#include "libmesh/equation_systems.h"

registerMooseObject("NavierStokesApp", SIMPLE);

InputParameters
SIMPLE::validParams()
{
  InputParameters params = Executioner::validParams();
  params += FEProblemSolve::validParams();
  params.addRequiredParam<NonlinearSystemName>("momentum_system",
                                               "The nonlinear system for the momentum equation");
  params.addRequiredParam<NonlinearSystemName>("pressure_system",
                                               "The nonlinear system for the pressure equation");
  return params;
}

SIMPLE::SIMPLE(const InputParameters & parameters)
  : Executioner(parameters),
    _problem(_fe_problem),
    _feproblem_solve(*this),
    _time_step(_problem.timeStep()),
    _time(_problem.time()),
    _momentum_sys_number(_problem.nlSysNum(getParam<NonlinearSystemName>("momentum_system"))),
    _pressure_sys_number(_problem.nlSysNum(getParam<NonlinearSystemName>("pressure_system")))
{
  _fixed_point_solve->setInnerSolve(_feproblem_solve);

  _time = 0;
}

void
SIMPLE::init()
{
  _problem.initialSetup();
}

void
SIMPLE::execute()
{
  _console << "This will be something smart" << std::endl;
}
