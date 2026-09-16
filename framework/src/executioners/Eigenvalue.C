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

#include <cmath>

registerMooseObject("MooseApp", Eigenvalue);

InputParameters
Eigenvalue::validParams()
{
  InputParameters params = Executioner::validParams();

  params.addClassDescription(
      "Eigenvalue solves a standard/generalized linear or nonlinear eigenvalue problem");

  params += EigenProblemSolve::validParams();
  params.addParam<Real>("time", 0.0, "System time");

  params.addParam<bool>("output_all_eigenvectors",
                        false,
                        "Whether to write one output step per converged eigenvector instead of "
                        "writing the active eigenvector only");
  params.addParam<MooseEnum>("eigenvector_time",
                             MooseEnum("index eigenvalue sqrt_eigenvalue", "index"),
                             "What the output time of each eigenvector step is: the one-based "
                             "index of the eigenvector, the real part of its eigenvalue, or the "
                             "square root of the real part of its eigenvalue. This parameter "
                             "requires 'output_all_eigenvectors' to be true.");
  params.addParamNamesToGroup("output_all_eigenvectors eigenvector_time", "Eigenvector output");

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
    _output_all_eigenvectors(getParam<bool>("output_all_eigenvectors")),
    _eigenvector_time(getParam<MooseEnum>("eigenvector_time")),
    _final_timer(registerTimedSection("final", 1))
{
  _fixed_point_solve->setInnerSolve(_eigen_problem_solve);
  _time = _system_time;

  if (!_output_all_eigenvectors && isParamSetByUser("eigenvector_time"))
    paramError("eigenvector_time",
               "'eigenvector_time' requires 'output_all_eigenvectors' to be true, as a single "
               "eigenvector is written at a single output time otherwise.");

  // Output objects take their time from the time step number unless the problem is transient, so
  // flagging the problem transient is the only way the per-eigenvector output times reach the
  // output files, as the legacy EigenExecutionerBase does as well. The executioner is constructed
  // before the outputs are added, and an output captures this flag at construction, so this is the
  // place to set it.
  //
  // The flag is problem-wide rather than output-only. Besides Output::time() returning _time
  // instead of _t_step, it turns on TransientInterface::_is_transient for every object constructed
  // after this executioner, the old- and older-state branches of MooseVariableData, and the time
  // and dt banners of the Console. It also changes FixedPointSolve::autoAdvance(), which turns
  // auto-advance off for a transient problem with fixed point iterations; the cast to Eigenvalue
  // added there exists only to keep auto-advance behaving for eigen problems with multiapps
  // exactly as it did before this flag was set.
  if (_output_all_eigenvectors)
    _eigen_problem.transient(true);
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

void
Eigenvalue::loadEigenvector(dof_id_type i)
{
  auto & nl = _eigen_problem.getNonlinearEigenSystem(/*nl_sys=*/0);
  nl.getConvergedEigenvector(i, nl.solution());
  nl.update();
  // Refresh the objects the normalization hook reads before and after it scales the eigenvector,
  // then let the objects executing at the end of a time step see the scaled eigenvector. Neither
  // execution runs multiapps or transfers.
  _eigen_problem.execute(EXEC_LINEAR);
  _eigen_problem.postScaleEigenVector(i);
  _eigen_problem.execute(EXEC_TIMESTEP_END);
}

Real
Eigenvalue::eigenvectorTime(dof_id_type i, const std::pair<Real, Real> & eig) const
{
  // The time of an eigenvector step is based on the real part of its eigenvalue only
  if (_eigenvector_time == "index")
    // One-based, so that the time equals the time step as it does for a single output step
    return i + 1;

  if (_eigenvector_time == "eigenvalue")
    return eig.first;

  // sqrt_eigenvalue
  if (eig.first < 0)
    mooseError("Cannot take the square root of the negative eigenvalue ",
               eig.first,
               " of mode ",
               i + 1,
               " to form an output time. Use 'eigenvector_time = eigenvalue' or 'eigenvector_time "
               "= index' instead.");

  return std::sqrt(eig.first);
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
    if (!_output_all_eigenvectors)
    {
      // need to keep _time in sync with _time_step to get correct output
      _time = _time_step;
      _eigen_problem.outputStep(EXEC_TIMESTEP_END);
      _time = _system_time;
    }
    else
    {
      // Write one output step per converged eigenvector
      auto & nl = _eigen_problem.getNonlinearEigenSystem(/*nl_sys=*/0);
      const auto & eigenvalues = nl.getAllConvergedEigenvalues();
      const int first_step = _time_step;
      for (const auto i : index_range(eigenvalues))
      {
        loadEigenvector(i);
        _time_step = first_step + static_cast<int>(i);
        _time = eigenvectorTime(i, eigenvalues[i]);
        _eigen_problem.outputStep(EXEC_TIMESTEP_END);
      }
      // Restore the eigenvector the rest of the simulation expects in the solution
      loadEigenvector(_eigen_problem.activeEigenvalueIndex());
      _time = _system_time;
    }

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
    if (_output_all_eigenvectors)
    {
      // Keep the final step on the requested time axis, at the time of the active eigenvector,
      // which is the one restored into the solution at the end of the per-eigenvector loop
      const auto & eigenvalues =
          _eigen_problem.getNonlinearEigenSystem(/*nl_sys=*/0).getAllConvergedEigenvalues();
      const auto active_index = _eigen_problem.activeEigenvalueIndex();
      if (active_index < eigenvalues.size())
        _time = eigenvectorTime(active_index, eigenvalues[active_index]);
    }
    _eigen_problem.outputStep(EXEC_FINAL);
    _time = _system_time;
  }

  postExecute();

#else
  mooseError("SLEPc is required for eigenvalue executioner, please use --download-slepc when "
             "configuring PETSc ");
#endif
}
