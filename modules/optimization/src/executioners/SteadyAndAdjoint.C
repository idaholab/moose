//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SteadyAndAdjoint.h"

registerMooseObject("OptimizationApp", SteadyAndAdjoint);

InputParameters
SteadyAndAdjoint::validParams()
{
  InputParameters params = Steady::validParams();
  params += AdjointSolve::validParams();
  params.addClassDescription(
      "Executioner for evaluating steady-state simulations and their adjoint.");

  // We need the full matrix for the adjoint solve, so set this to NEWTON
  params.set<MooseEnum>("solve_type") = "newton";
  params.suppressParameter<MooseEnum>("solve_type");

  // The adjoint system (second one) is solved by _adjoint_solve
  // This is a parameter of the MultiSystemSolveObject, which we set from here, the executioner.
  // We seek to prevent the MultiSystemSolveObject from solving both systems
  // This is abusing input parameters, but SolveObjects do not have their own syntax
  // and we need to send this parameter from the executioner to the default nested SolveObject
  params.renameParam("system_names", "forward_system", "");

  return params;
}

SteadyAndAdjoint::SteadyAndAdjoint(const InputParameters & parameters)
  : Steady(parameters), _adjoint_solve(*this)
{
}

void
SteadyAndAdjoint::execute()
{
  // This is basically copied from Steady (without AMR)
  if (_app.isRecovering())
  {
    _console << "\nCannot recover steady solves!\nExiting...\n" << std::endl;
    _last_solve_converged = false;
    return;
  }

  _time_step = 0;
  _time = _time_step;
  _problem.outputStep(EXEC_INITIAL);
  _time = _system_time;

  preExecute();

  _problem.advanceState();
  _time_step = 1;
  _problem.timestepSetup();

  // Solving forward and adjoint problem here (only difference from Steady)
  _last_solve_converged = _fixed_point_solve->solve() && _adjoint_solve.solve();

  if (!lastSolveConverged())
    _console << "Aborting as solve did not converge" << std::endl;
  else
  {
    _time = _time_step;
    _problem.outputStep(EXEC_TIMESTEP_END);
    _time = _system_time;
  }

  {
    TIME_SECTION("final", 1, "Executing Final Objects")
    _problem.execMultiApps(EXEC_FINAL);
    _problem.finalizeMultiApps();
    _problem.postExecute();
    _problem.execute(EXEC_FINAL);
    _time = _time_step;
    _problem.outputStep(EXEC_FINAL);
    _time = _system_time;
  }

  postExecute();
}
