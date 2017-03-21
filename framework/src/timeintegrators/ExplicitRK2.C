/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ExplicitRK2.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "PetscSupport.h"

template <>
InputParameters
validParams<ExplicitRK2>()
{
  InputParameters params = validParams<TimeIntegrator>();

  return params;
}

ExplicitRK2::ExplicitRK2(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    _stage(1),
    _residual_old(_nl.addVector("residual_old", false, GHOSTED))
{
}

ExplicitRK2::~ExplicitRK2() {}

void
ExplicitRK2::preSolve()
{
  if (_dt == _dt_old)
    _fe_problem.setConstJacobian(true);
  else
    _fe_problem.setConstJacobian(false);
}

void
ExplicitRK2::computeTimeDerivatives()
{
  // Since advanceState() is called in between stages 2 and 3, this
  // changes the meaning of "_solution_old".  In the second stage,
  // "_solution_older" is actually the original _solution_old.
  _u_dot = *_solution;
  if (_stage < 3)
    _u_dot -= _solution_old;
  else
    _u_dot -= _solution_older;

  _u_dot *= 1. / _dt;
  _du_dot_du = 1. / _dt;
  _u_dot.close();
}

void
ExplicitRK2::solve()
{
  Real time_new = _fe_problem.time();
  Real time_old = _fe_problem.timeOld();
  Real time_stage2 = time_old + a() * _dt;

  // There is no work to do for the first stage (Y_1 = y_n).  The
  // first solve therefore happens in the second stage.  Note that the
  // non-time Kernels (which should be marked implicit=false) are
  // evaluated at the old solution during this stage.
  _fe_problem.initPetscOutput();
  _console << "1st solve\n";
  _stage = 2;
  _fe_problem.timeOld() = time_old;
  _fe_problem.time() = time_stage2;
  _fe_problem.getNonlinearSystemBase().system().solve();

  // Advance solutions old->older, current->old.  Also moves Material
  // properties and other associated state forward in time.
  _fe_problem.advanceState();

  // The "update" stage (which we call stage 3) requires an additional
  // solve with the mass matrix.
  _fe_problem.initPetscOutput();
  _console << "2nd solve\n";
  _stage = 3;
  _fe_problem.timeOld() = time_stage2;
  _fe_problem.time() = time_new;
  _fe_problem.getNonlinearSystemBase().system().solve();

  // Reset time at beginning of step to its original value
  _fe_problem.timeOld() = time_old;
}

void
ExplicitRK2::postStep(NumericVector<Number> & residual)
{
  if (_stage == 1)
  {
    // If postStep() is called before solve(), _stage==1 and we don't
    // need to do anything.
  }
  else if (_stage == 2)
  {
    // In the standard RK notation, the stage 2 residual is given by:
    //
    // R := M*(Y_2 - y_n)/dt - a*f(t_n, Y_1) = 0
    //
    // where:
    // .) M is the mass matrix.
    // .) f(t_n, Y_1) is the residual we are currently computing,
    //    since this method is intended to be used with "implicit=false"
    //    kernels.
    // .) M*(Y_2 - y_n)/dt corresponds to the residual of the time kernels.
    // .) The minus signs are "baked in" to the non-time residuals, so
    //    they do not appear here.
    // .) The current non-time residual is saved for the next stage.
    _residual_old = _Re_non_time;
    _residual_old.close();

    residual.add(1., _Re_time);
    residual.add(a(), _residual_old);
    residual.close();
  }
  else if (_stage == 3)
  {
    // In the standard RK notation, the update step residual is given by:
    //
    // R := M*(y_{n+1} - y_n)/dt - f(t_n+dt/2, Y_2) = 0
    //
    // where:
    // .) M is the mass matrix.
    // .) f(t_n+dt/2, Y_2) is the residual from stage 2.
    // .) The minus sign is already baked in to the non-time
    //    residuals, so it does not appear here.
    // .) Although this is an update step, we have to do a "solve"
    //    using the mass matrix.
    residual.add(1., _Re_time);
    residual.add(b1(), _residual_old);
    residual.add(b2(), _Re_non_time);
    residual.close();
  }
  else
    mooseError("ExplicitRK2::postStep(): _stage = ", _stage, ", only _stage = 1-3 is allowed.");
}
