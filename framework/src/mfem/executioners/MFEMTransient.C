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
  InputParameters params = MFEMProblemSolve::validParams();
  params += TransientBase::validParams();
  params.addClassDescription("Executioner for transient MFEM problems.");
  return params;
}

MFEMTransient::MFEMTransient(const InputParameters & params)
  : TransientBase(params),
    _mfem_problem(dynamic_cast<MFEMProblem &>(feProblem())),
    _mfem_problem_data(_mfem_problem.getProblemData()),
    _mfem_problem_solve(*this, getProblemOperators())
{
}

void
MFEMTransient::init()
{
  TransientBase::init();

  // Set up initial conditions
  _mfem_problem_data.eqn_system->Init(
      _mfem_problem_data.gridfunctions,
      _mfem_problem_data.fespaces,
      getParam<MooseEnum>("assembly_level").getEnum<mfem::AssemblyLevel>());

  for (const auto & problem_operator : getProblemOperators())
  {
    problem_operator->SetGridFunctions();
    problem_operator->Init(_mfem_problem_data.f);
  }
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
  _problem.timestepSetup();
  _problem.onTimestepBegin();

  // Advance time step of the MFEM problem. Time is also updated here, and
  // _problem_operator->SetTime is called inside the ode_solver->Step method to
  // update the time used by time dependent (function) coefficients.
  _time_stepper->step();

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
}

#endif
