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
  params.addClassDescription("Executioner for transient MFEM problems.");
  params.addParam<mfem::real_t>("start_time", 0.0, "The start time of the simulation");
  params.addParam<mfem::real_t>("end_time", 1.0e30, "The end time of the simulation");
  params.addParam<mfem::real_t>("dt", 1., "The timestep size between solves");
  params.addParam<unsigned int>(
      "visualisation_steps", 1, "The number of timesteps in a transient run");
  return params;
}

MFEMTransient::MFEMTransient(const InputParameters & params)
  : MFEMExecutioner(params),
    _dt(getParam<mfem::real_t>("dt")),
    _dt_old(getParam<mfem::real_t>("dt")),
    _time(_mfem_problem.time()),
    _start_time(getParam<mfem::real_t>("start_time")),
    _end_time(getParam<mfem::real_t>("end_time")),
    _t_step(0),
    _vis_steps(params.get<unsigned int>("visualisation_steps")),
    _last_step(false)
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

// void
// MFEMTransient::takeStep(mfem::real_t input_dt)
// {
//   _dt_old = _dt;

//   if (input_dt == -1.0)
//     _dt = computeConstrainedDT();
//   else
//     _dt = input_dt;

//   _time_stepper->preSolve();

//   // Increment time
//   _time = _time_old + _dt;

//   _problem.timestepSetup();

//   _problem.onTimestepBegin();

//   _time_stepper->step();
//   _xfem_repeat_step = _fixed_point_solve->XFEMRepeatStep();

//   _last_solve_converged = _time_stepper->converged();

//   if (!lastSolveConverged())
//   {
//     _console << "Aborting as solve did not converge" << std::endl;
//     return;
//   }

//   if (!(_problem.haveXFEM() && _fixed_point_solve->XFEMRepeatStep()))
//   {
//     if (lastSolveConverged())
//       _time_stepper->acceptStep();
//     else
//       _time_stepper->rejectStep();
//   }

//   _time = _time_old;

//   _time_stepper->postSolve();

//   _solution_change_norm =
//       relativeSolutionDifferenceNorm() / (_normalize_solution_diff_norm_by_dt ? _dt : Real(1));
// }

void
MFEMTransient::step(double dt, int) const
{
  // Check if current time step is final
  if (_time + dt >= _end_time - dt / 2)
  {
    _last_step = true;
  }

  // Advance time step.
  _problem_data.ode_solver->Step(_problem_data.f, _time, dt);

  // Synchonise time dependent GridFunctions with updated DoF data.
  _problem_operator->SetTestVariablesFromTrueVectors();

  // Execute user objects at timestep end
  _mfem_problem.execute(EXEC_TIMESTEP_END);
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

void
MFEMTransient::execute()
{
  _mfem_problem.outputStep(EXEC_INITIAL);
  preExecute();

  while (_last_step != true)
  {
    _t_step++;
    step(_dt, _t_step);
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

#endif
