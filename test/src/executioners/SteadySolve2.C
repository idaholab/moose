//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SteadySolve2.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"

#include "libmesh/equation_systems.h"

registerMooseObject("MooseTestApp", SteadySolve2);

InputParameters
SteadySolve2::validParams()
{
  InputParameters params = SteadyBase::validParams();
  params += FEProblemSolve::validParams();
  params.addRequiredParam<NonlinearSystemName>("first_nl_sys_to_solve",
                                               "The first nonlinear system to solve");
  params.addRequiredParam<NonlinearSystemName>("second_nl_sys_to_solve",
                                               "The second nonlinear system to solve");
  params.addRangeCheckedParam<unsigned int>("number_of_iterations",
                                            1,
                                            "number_of_iterations>0",
                                            "The number of iterations between the two systems.");
  return params;
}

SteadySolve2::SteadySolve2(const InputParameters & parameters)
  : SteadyBase(parameters),
    _first_nl_sys(_problem.nlSysNum(getParam<NonlinearSystemName>("first_nl_sys_to_solve"))),
    _second_nl_sys(_problem.nlSysNum(getParam<NonlinearSystemName>("second_nl_sys_to_solve"))),
    _number_of_iterations(getParam<unsigned int>("number_of_iterations")),
    _feproblem_solve(*this, {_first_nl_sys, _second_nl_sys}, _number_of_iterations)
{
  _fixed_point_solve->setInnerSolve(_feproblem_solve);
}

void
SteadySolve2::init()
{
  if (_app.isRecovering())
  {
    _console << "\nCannot recover steady solves!\nExiting...\n" << std::endl;
    return;
  }

  _problem.initialSetup();
}
