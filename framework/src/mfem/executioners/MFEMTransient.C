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
#include "TimeStepper.h"

registerMooseObject("MooseApp", MFEMTransient);

InputParameters
MFEMTransient::validParams()
{
  InputParameters params = MFEMExecutioner::validParams();
  params += TransientBase::validParams();
  params.addClassDescription("Executioner for transient MFEM problems.");
  return params;
}

MFEMTransient::MFEMTransient(const InputParameters & params)
  : TransientBase(params), MFEMExecutioner(params, dynamic_cast<MFEMProblem &>(feProblem()))
{
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
  _problem_operator->SetTime(_start_time);
}

void
MFEMTransient::takeStep(Real input_dt)
{
  _dt_old = _dt;

  if (input_dt == -1.0)
    _dt = computeConstrainedDT();
  else
    _dt = input_dt;

  _time_stepper->preSolve();

  // Advance time step of the MFEM problem. Time is also updated here.
  // Takes place instead of TimeStepper::step().
  _problem_data.ode_solver->Step(_problem_data.f, _time, _dt);

  // Synchonise time dependent GridFunctions with updated DoF data.
  _problem_operator->SetTestVariablesFromTrueVectors();

  // Sync Host/Device
  _problem_data.f.HostRead();

  // Execute user objects at timestep end
  _mfem_problem.execute(EXEC_TIMESTEP_END);

  // Continue with usual TransientBase::takeStep() finalisation
  _last_solve_converged = _time_stepper->converged();

  if (!lastSolveConverged())
  {
    _console << "Aborting as solve did not converge" << std::endl;
    return;
  }

  if (lastSolveConverged())
    _time_stepper->acceptStep();
  else
    _time_stepper->rejectStep();

  // Set time to time old, since final time is updated in TransientBase::endStep()
  _time = _time_old;

  _time_stepper->postSolve();

  _solution_change_norm =
      relativeSolutionDifferenceNorm() / (_normalize_solution_diff_norm_by_dt ? _dt : Real(1));
}

#endif
