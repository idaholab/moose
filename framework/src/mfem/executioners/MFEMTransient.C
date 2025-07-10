//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMTransient.h"
#include "MFEMProblem.h"
#include "TimeDomainEquationSystemProblemOperator.h"

registerMooseObject("MooseApp", MFEMTransient);

InputParameters
MFEMTransient::validParams()
{
  InputParameters params = MFEMExecutioner::validParams();
  params += Executioner::validParams();
  params.addClassDescription("Executioner for transient MFEM problems.");
  params.addParam<mfem::real_t>("start_time", 0.0, "The start time of the simulation");
  params.addParam<mfem::real_t>("end_time", 1.0e30, "The end time of the simulation");
  params.addParam<mfem::real_t>("dt", 1., "The timestep size between solves");
  params.addParam<mfem::real_t>("dtmin", 1.0e-12, "The minimum timestep size in an adaptive run");
  params.addParam<mfem::real_t>("dtmax", 1.0e30, "The maximum timestep size in an adaptive run");
  params.addParam<unsigned int>("num_steps",
                                std::numeric_limits<unsigned int>::max(),
                                "The number of timesteps in a transient run");
  params.addParam<unsigned int>(
      "visualisation_steps", 1, "The number of timesteps in a transient run");
  params.addParam<bool>(
      "abort_on_solve_fail", false, "abort if solve not converged rather than cut timestep");
  params.addParam<bool>(
      "error_on_dtmin",
      true,
      "Throw error when timestep is less than dtmin instead of just aborting solve.");
  params.addParam<mfem::real_t>("timestep_tolerance",
                        1.0e-12,
                        "the tolerance setting for final timestep size and sync times");
  return params;
}

MFEMTransient::MFEMTransient(const InputParameters & params)
  : Executioner(params),
    MFEMExecutioner(params, dynamic_cast<MFEMProblem &>(feProblem())),
    _dt(getParam<mfem::real_t>("dt")),
    _dt_old(getParam<mfem::real_t>("dt")),
    _time(_mfem_problem.time()),
    _start_time(getParam<mfem::real_t>("start_time")),
    _end_time(getParam<mfem::real_t>("end_time")),
    _t_step(0),
    _vis_steps(params.get<unsigned int>("visualisation_steps")),
    _last_step(false),
    _timestep_tolerance(getParam<mfem::real_t>("timestep_tolerance")),
    _dtmin(getParam<mfem::real_t>("dtmin")),
    _dtmax(getParam<mfem::real_t>("dtmax")),
    _num_steps(getParam<unsigned int>("num_steps")),
    _abort(getParam<bool>("abort_on_solve_fail")),
    _error_on_dtmin(getParam<bool>("error_on_dtmin"))
{
  _app.setStartTime(_start_time);
  _time = _start_time;
  _mfem_problem.transient(true);
}

void
MFEMTransient::constructProblemOperator()
{
  _problem_data.eqn_system = std::make_shared<Moose::MFEM::TimeDependentEquationSystem>();
  auto problem_operator =
      std::make_unique<Moose::MFEM::TimeDomainEquationSystemProblemOperator>(_problem_data);
  _problem_operator.reset();
  _problem_operator = std::move(problem_operator);
}

void
MFEMTransient::takeStep(mfem::real_t input_dt)
{
  _dt = input_dt;

  // Advance time step. Time is also updated here.
  _problem_data.ode_solver->Step(_problem_data.f, _time, _dt);

  // Synchonise time dependent GridFunctions with updated DoF data.
  _problem_operator->SetTestVariablesFromTrueVectors();

  // Execute user objects at timestep end
  _mfem_problem.execute(EXEC_TIMESTEP_END);
}

void
MFEMTransient::endStep()
{
  // Compute the Error Indicators and Markers
  _mfem_problem.computeIndicators();
  _mfem_problem.computeMarkers();

  // Perform the output of the current time step
  _mfem_problem.outputStep(EXEC_TIMESTEP_END);
}

void
MFEMTransient::init()
{
  _mfem_problem.execute(EXEC_PRE_MULTIAPP_SETUP);
  _mfem_problem.initialSetup();

  // Set up initial conditions
  _problem_data.eqn_system->Init(
      _problem_data.gridfunctions,
      _problem_data.fespaces,
      getParam<MooseEnum>("assembly_level").getEnum<mfem::AssemblyLevel>());

  _problem_operator->SetGridFunctions();
  _problem_operator->Init(_problem_data.f);

  // Set timestepper
  _problem_data.ode_solver = std::make_unique<mfem::BackwardEulerSolver>();
  _problem_data.ode_solver->Init(*(_problem_operator));
  _problem_operator->SetTime(0.0);
}

bool
MFEMTransient::keepGoing()
{
  bool keep_going = !_mfem_problem.isSolveTerminationRequested();

  // Check for stop condition based upon steady-state check flag:
  if (lastSolveConverged())
  {
    // Check for stop condition based upon number of simulation steps and/or solution end time:
    if (static_cast<unsigned int>(_t_step) >= _num_steps)
      keep_going = false;

    if ((_time >= _end_time) || (fabs(_time - _end_time) <= _timestep_tolerance))
      keep_going = false;
  }
  else if (_abort)
  {
    _console << "Aborting as solve did not converge and input selected to abort" << std::endl;
    keep_going = false;
  }
  else if (!_error_on_dtmin && _dt <= _dtmin)
  {
    _console << "Aborting as timestep already at or below dtmin" << std::endl;
    keep_going = false;
  }

  return keep_going;
}

void
MFEMTransient::execute()
{
  _mfem_problem.outputStep(EXEC_INITIAL);
  preExecute();

  while (keepGoing())
  {
    incrementStepOrReject();
    takeStep(_dt);
    endStep();
  }

  _mfem_problem.finishMultiAppStep(EXEC_MULTIAPP_FIXED_POINT_BEGIN,
                                   /*recurse_through_multiapp_levels=*/true);
  _mfem_problem.finishMultiAppStep(EXEC_TIMESTEP_BEGIN, /*recurse_through_multiapp_levels=*/true);
  _mfem_problem.finishMultiAppStep(EXEC_TIMESTEP_END, /*recurse_through_multiapp_levels=*/true);
  _mfem_problem.finishMultiAppStep(EXEC_MULTIAPP_FIXED_POINT_END,
                                   /*recurse_through_multiapp_levels=*/true);

  TIME_SECTION("final", 1, "Executing Final Objects");
  _mfem_problem.execMultiApps(EXEC_FINAL);
  _mfem_problem.finalizeMultiApps();
  _mfem_problem.execute(EXEC_FINAL);
  _mfem_problem.outputStep(EXEC_FINAL);
  _mfem_problem.postExecute();

  postExecute();
}

void
MFEMTransient::incrementStepOrReject()
{
  _t_step++;
  _mfem_problem.advanceState();
  if (_t_step == 1)
    return;

  /*
    * Call the multi-app executioners endStep and
    * postStep methods when doing Picard or when not automatically advancing sub-applications for
    * some other reason. We do not perform these calls for loose-coupling/auto-advancement
    * problems because TransientBase::endStep and TransientBase::postStep get called from
    * TransientBaseMultiApp::solveStep in that case.
    */
  _mfem_problem.finishMultiAppStep(EXEC_MULTIAPP_FIXED_POINT_BEGIN);
  _mfem_problem.finishMultiAppStep(EXEC_TIMESTEP_BEGIN);
  _mfem_problem.finishMultiAppStep(EXEC_TIMESTEP_END);
  _mfem_problem.finishMultiAppStep(EXEC_MULTIAPP_FIXED_POINT_END);

  /*
    * Ensure that we increment the sub-application time steps so that
    * when dt selection is made in the master application, we are using
    * the correct time step information
    */
  _mfem_problem.incrementMultiAppTStep(EXEC_MULTIAPP_FIXED_POINT_BEGIN);
  _mfem_problem.incrementMultiAppTStep(EXEC_TIMESTEP_BEGIN);
  _mfem_problem.incrementMultiAppTStep(EXEC_TIMESTEP_END);
  _mfem_problem.incrementMultiAppTStep(EXEC_MULTIAPP_FIXED_POINT_END);
}
#endif
