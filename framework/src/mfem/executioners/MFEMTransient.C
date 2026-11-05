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
    _mfem_problem_solve(*this, _mfem_problem.getProblemOperators())
{
  _mfem_problem.setAssemblyLevel(
      getParam<MooseEnum>("assembly_level").getEnum<mfem::AssemblyLevel>());
}

void
MFEMTransient::init()
{
  TransientBase::init();

  // verify that the requested time integration scheme is actually supported by MFEM transient
  if (getTimeScheme() != Moose::TimeIntegratorType::TI_IMPLICIT_EULER)
    paramError("scheme",
               "Time Integration scheme \"" + stringify(getTimeScheme()) +
                   "\" is not supported by MFEMTransient Executioner.");
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

  // Advance time to the end of the step, as TransientBase does, so that everything executed during
  // the step, including MultiApps and objects run at TIMESTEP_BEGIN, sees the same time
  _time = _time_old + _dt;
  // Coefficients evaluated before the solve, such as by auxkernels at TIMESTEP_BEGIN, see it too
  _mfem_problem.getProblemData().coefficients.setTime(_time);
  _problem.timestepSetup();

  _problem.onTimestepBegin();
  if (!_problem.execMultiApps(EXEC_TIMESTEP_BEGIN, true))
  {
    _last_solve_converged = false;
    return;
  }
  _problem.execute(EXEC_TIMESTEP_BEGIN);

  // Advance time step of the MFEM problem.
  _time_stepper->step();

  // Continue with usual TransientBase::takeStep() finalisation
  _last_solve_converged = _time_stepper->converged();

  if (!lastSolveConverged())
  {
    _console << "Aborting as solve did not converge" << std::endl;
    return;
  }

  _problem.execute(EXEC_TIMESTEP_END);
  _problem.execMultiApps(EXEC_TIMESTEP_END, true);

  if (lastSolveConverged())
    _time_stepper->acceptStep();
  else
    _time_stepper->rejectStep();

  // Set time to time old, since final time is updated in TransientBase::endStep()
  _time = _time_old;

  _time_stepper->postSolve();
}

#endif
