//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExplicitTVDRK2.h"
#include "NonlinearSystemBase.h"
#include "FEProblem.h"
#include "PetscSupport.h"

registerMooseObject("MooseApp", ExplicitTVDRK2);

InputParameters
ExplicitTVDRK2::validParams()
{
  InputParameters params = TimeIntegrator::validParams();
  params.addClassDescription("Explicit TVD (total-variation-diminishing) second-order Runge-Kutta "
                             "time integration method.");
  return params;
}

ExplicitTVDRK2::ExplicitTVDRK2(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    _stage(1),
    _residual_old(_nl.addVector("residual_old", false, GHOSTED)),
    _solution_older(_sys.solutionState(2))
{
  mooseInfo("ExplicitTVDRK2 and other multistage TimeIntegrators are known not to work with "
            "Materials/AuxKernels that accumulate 'state' and should be used with caution.");
}

void
ExplicitTVDRK2::preSolve()
{
  if (_dt == _dt_old)
    _fe_problem.setConstJacobian(true);
  else
    _fe_problem.setConstJacobian(false);
}

void
ExplicitTVDRK2::computeTimeDerivatives()
{
  // Since advanceState() is called in between stages 2 and 3, this
  // changes the meaning of "_solution_old".  In the second stage,
  // "_solution_older" is actually the original _solution_old.
  if (!_sys.solutionUDot())
    mooseError("ExplicitTVDRK2: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase befor requesting `u_dot`.");

  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  u_dot = *_solution;
  computeTimeDerivativeHelper(u_dot, _solution_old, _solution_older);

  _du_dot_du = 1. / _dt;
  u_dot.close();
}

void
ExplicitTVDRK2::computeADTimeDerivatives(DualReal & ad_u_dot,
                                         const dof_id_type & dof,
                                         DualReal & /*ad_u_dotdot*/) const
{
  computeTimeDerivativeHelper(ad_u_dot, _solution_old(dof), _solution_older(dof));
}

void
ExplicitTVDRK2::solve()
{
  Real time_new = _fe_problem.time();
  Real time_old = _fe_problem.timeOld();
  Real time_stage2 = time_old + _dt;

  // Reset numbers of iterations
  _n_nonlinear_iterations = 0;
  _n_linear_iterations = 0;

  // There is no work to do for the first stage (Y_1 = y_n).  The
  // first solve therefore happens in the second stage.  Note that the
  // non-time Kernels (which should be marked implicit=false) are
  // evaluated at the old solution during this stage.
  _fe_problem.initPetscOutput();
  _console << "1st solve" << std::endl;
  _stage = 2;
  _fe_problem.timeOld() = time_old;
  _fe_problem.time() = time_stage2;
  _fe_problem.getNonlinearSystemBase().system().solve();
  _n_nonlinear_iterations += getNumNonlinearIterationsLastSolve();
  _n_linear_iterations += getNumLinearIterationsLastSolve();

  // Abort time step immediately on stage failure - see TimeIntegrator doc page
  if (!_fe_problem.converged())
    return;

  // Advance solutions old->older, current->old.  Also moves Material
  // properties and other associated state forward in time.
  _fe_problem.advanceState();

  // The "update" stage (which we call stage 3) requires an additional
  // solve with the mass matrix.
  _fe_problem.initPetscOutput();
  _console << "2nd solve" << std::endl;
  _stage = 3;
  _fe_problem.timeOld() = time_stage2;
  _fe_problem.time() = time_new;
  _fe_problem.getNonlinearSystemBase().system().solve();
  _n_nonlinear_iterations += getNumNonlinearIterationsLastSolve();
  _n_linear_iterations += getNumLinearIterationsLastSolve();

  // Reset time at beginning of step to its original value
  _fe_problem.timeOld() = time_old;
}

void
ExplicitTVDRK2::postResidual(NumericVector<Number> & residual)
{
  if (_stage == 1)
  {
    // If postResidual() is called before solve(), _stage==1 and we don't
    // need to do anything.
  }
  else if (_stage == 2)
  {
    // In the standard RK notation, the stage 2 residual is given by:
    //
    // R := M*(Y_2 - y_n)/dt - f(t_n, Y_1) = 0
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

    residual.add(1.0, _Re_time);
    residual.add(1.0, _residual_old);
    residual.close();
  }
  else if (_stage == 3)
  {
    // In the standard RK notation, the update step residual is given by:
    //
    // R := M*(2*y_{n+1} - Y_2 - y_n)/(2*dt) - (1/2)*f(t_n+dt/2, Y_2) = 0
    //
    // where:
    // .) M is the mass matrix.
    // .) f(t_n+dt/2, Y_2) is the residual from stage 2.
    // .) The minus sign is already baked in to the non-time
    //    residuals, so it does not appear here.
    // .) Although this is an update step, we have to do a "solve"
    //    using the mass matrix.
    residual.add(1.0, _Re_time);
    residual.add(0.5, _Re_non_time);
    residual.close();
  }
  else
    mooseError(
        "ExplicitTVDRK2::postResidual(): _stage = ", _stage, ", only _stage = 1-3 is allowed.");
}
