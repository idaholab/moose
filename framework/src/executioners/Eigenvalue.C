//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Eigenvalue.h"
#include "EigenProblem.h"
#include "Factory.h"
#include "MooseApp.h"
#include "NonlinearEigenSystem.h"

registerMooseObject("MooseApp", Eigenvalue);

InputParameters
Eigenvalue::validParams()
{
  InputParameters params = Executioner::validParams();

  params.addClassDescription(
      "Eigenvalue solves a standard/generalized linear or nonlinear eigenvalue problem");

  params += EigenProblemSolve::validParams();
  params.addParam<Real>("time", 0.0, "System time");

  return params;
}

Eigenvalue::Eigenvalue(const InputParameters & parameters)
  : Executioner(parameters),
    _eigen_problem(*getCheckedPointerParam<EigenProblem *>(
        "_eigen_problem", "This might happen if you don't have a mesh")),
    _eigen_problem_solve(*this),
    _system_time(getParam<Real>("time")),
    _time_step(_eigen_problem.timeStep()),
    _time(_eigen_problem.time()),
    _final_timer(registerTimedSection("final", 1))
{
  _fixed_point_solve->setInnerSolve(_eigen_problem_solve);
  _time = _system_time;
}

#ifdef LIBMESH_HAVE_SLEPC
void
Eigenvalue::init()
{
  // Does not allow time kernels
  checkIntegrity();

  // Provide vector of ones to solver
  // "auto_initialization" is on by default and we init the vector values associated
  // with eigen-variables as ones. If "auto_initialization" is turned off by users,
  // it is up to users to provide an initial guess. If "auto_initialization" is off
  // and users does not provide an initial guess, slepc will automatically generate
  // a random vector as the initial guess. The motivation to offer this option is
  // that we have to initialize ONLY eigen variables in multiphysics simulation.
  // auto_initialization can be overriden by initial conditions.
  if (getParam<bool>("auto_initialization") && !_app.isRestarting())
    _eigen_problem.initEigenvector(1.0);

  _eigen_problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _eigen_problem.initialSetup();
  _fixed_point_solve->initialSetup();
  _eigen_problem_solve.initialSetup();
}

void
Eigenvalue::checkIntegrity()
{
  // check to make sure that we don't have any time kernels in eigenvalue simulation
  if (_eigen_problem.getNonlinearSystemBase(/*nl_sys=*/0).containsTimeKernel())
    mooseError("You have specified time kernels in your eigenvalue simulation");
}
#endif

void
Eigenvalue::execute()
{
#ifdef LIBMESH_HAVE_SLEPC
  // Recovering makes sense for only transient simulations since the solution from
  // the previous time steps is required.
  if (_app.isRecovering())
  {
    _console << "\nCannot recover eigenvalue solves!\nExiting...\n" << std::endl;
    _last_solve_converged = true;
    return;
  }

  // Outputs initial conditions set by users
  // It is consistent with Steady
  _time_step = 0;
  _time = _time_step;
  _eigen_problem.outputStep(EXEC_INITIAL);
  _time = _system_time;

  preExecute();

  // The following code of this function is copied from "Steady"
  // "Eigenvalue" implementation can be considered a one-time-step simulation to
  // have the code compatible with the rest moose world.
  _eigen_problem.advanceState();

  // First step in any eigenvalue state solve is always 1 (preserving backwards compatibility)
  _time_step = 1;

#ifdef LIBMESH_ENABLE_AMR

  // Define the refinement loop
  auto steps = _eigen_problem.adaptivity().getSteps();
  for (const auto r_step : make_range(steps + 1))
  {
#endif // LIBMESH_ENABLE_AMR
    _eigen_problem.timestepSetup();

    _last_solve_converged = _fixed_point_solve->solve();
    if (!lastSolveConverged())
    {
      _console << "Aborting as solve did not converge" << std::endl;
      break;
    }

    // Compute markers and indicators only when we do have at least one adaptivity step
    if (steps)
    {
      _eigen_problem.computeIndicators();
      _eigen_problem.computeMarkers();
    }
    // need to keep _time in sync with _time_step to get correct output
    _time = _time_step;
    _eigen_problem.outputStep(EXEC_TIMESTEP_END);
    _time = _system_time;

#ifdef LIBMESH_ENABLE_AMR
    if (r_step < steps)
    {
      _eigen_problem.adaptMesh();
    }

    _time_step++;
  }
#endif

  {
    TIME_SECTION(_final_timer)
    _eigen_problem.execMultiApps(EXEC_FINAL);
    _eigen_problem.finalizeMultiApps();
    _eigen_problem.postExecute();
    _eigen_problem.execute(EXEC_FINAL);
    _time = _time_step;
    _eigen_problem.outputStep(EXEC_FINAL);
    _time = _system_time;
  }

  postExecute();

#else
  mooseError("SLEPc is required for eigenvalue executioner, please use --download-slepc when "
             "configuring PETSc ");
#endif
}
