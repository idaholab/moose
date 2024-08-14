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
  InputParameters params = Executioner::validParams();
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
  : Executioner(parameters),
    _problem(_fe_problem),
    _feproblem_solve(*this),
    _time_step(_problem.timeStep()),
    _time(_problem.time()),
    _first_nl_sys(_problem.nlSysNum(getParam<NonlinearSystemName>("first_nl_sys_to_solve"))),
    _second_nl_sys(_problem.nlSysNum(getParam<NonlinearSystemName>("second_nl_sys_to_solve"))),
    _number_of_iterations(getParam<unsigned int>("number_of_iterations"))
{
  _fixed_point_solve->setInnerSolve(_feproblem_solve);

  _time = 0;
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

void
SteadySolve2::execute()
{
  if (_app.isRecovering())
    return;

  _time_step = 0;
  _problem.outputStep(EXEC_INITIAL);

  preExecute();

  // first step in any steady state solve is always 1 (preserving backwards compatibility)
  _time_step = 1;

#ifdef LIBMESH_ENABLE_AMR

  // Define the refinement loop
  unsigned int steps = _problem.adaptivity().getSteps();
  for (unsigned int r_step = 0; r_step <= steps; r_step++)
  {
#endif // LIBMESH_ENABLE_AMR
    _problem.timestepSetup();

    preSolve();
    _problem.execute(EXEC_TIMESTEP_BEGIN);
    _problem.outputStep(EXEC_TIMESTEP_BEGIN);
    _problem.updateActiveObjects();

    auto solve = [this](const auto & sys_num)
    {
      _problem.solve(sys_num);

      if (_problem.shouldSolve())
      {
        if (_problem.converged(sys_num))
          _console << COLOR_GREEN << " Nonlinear system " << sys_num << " solve converged!"
                   << COLOR_DEFAULT << std::endl;
        else
        {
          _console << COLOR_RED << " Nonlinear system " << sys_num << " solve did not converge!"
                   << COLOR_DEFAULT << std::endl;
          return false;
        }
      }
      else
        _console << COLOR_GREEN << " Nonlinear system " << sys_num << " solve skipped!"
                 << COLOR_DEFAULT << std::endl;

      return _problem.converged(sys_num);
    };

    bool converged = true;

    for (unsigned int i = 0; i < _number_of_iterations; i++)
    {
      converged = converged && solve(_first_nl_sys) && solve(_second_nl_sys);
      if (!converged)
      {
        _console << "Aborting as solve did not converge" << std::endl;
        break;
      }
    }
    _problem.onTimestepEnd();
    _problem.execute(EXEC_TIMESTEP_END);

    _problem.computeIndicators();
    _problem.computeMarkers();

    // need to keep _time in sync with _time_step to get correct output
    _time = _time_step;
    _problem.outputStep(EXEC_TIMESTEP_END);
    _time = _system_time;

#ifdef LIBMESH_ENABLE_AMR
    if (r_step != steps)
      _problem.adaptMesh();

    _time_step++;
  }
#endif

  {
    TIME_SECTION("final", 1, "Executing Final Objects")
    _problem.postExecute();
    _problem.execute(EXEC_FINAL);
    _time = _time_step;
    _problem.outputStep(EXEC_FINAL);
  }

  postExecute();
}
