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
  params += TransientBase::validParams();
  params.addClassDescription("Executioner for transient MFEM problems.");
  return params;
}

MFEMTransient::MFEMTransient(const InputParameters & params)
  : TransientBase(params), MFEMExecutioner(params, dynamic_cast<MFEMProblem &>(feProblem()))
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
MFEMTransient::takeStep(mfem::real_t input_dt)
{
  _dt = input_dt;

  // Advance time step. Time is also updated here.
  _problem_data.ode_solver->Step(_problem_data.f, _time, _dt);

  // Synchonise time dependent GridFunctions with updated DoF data.
  _problem_operator->SetTestVariablesFromTrueVectors();

  // Sync Host/Device
  _problem_data.f.HostRead();

  // Execute user objects at timestep end
  _mfem_problem.execute(EXEC_TIMESTEP_END);
}

void
MFEMTransient::endStep(Real input_time)
{
  // Compute the Error Indicators and Markers
  _mfem_problem.computeIndicators();
  _mfem_problem.computeMarkers();

  // Perform the output of the current time step
  _mfem_problem.outputStep(EXEC_TIMESTEP_END);
}

#endif
