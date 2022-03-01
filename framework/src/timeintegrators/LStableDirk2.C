//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LStableDirk2.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "PetscSupport.h"

registerMooseObject("MooseApp", LStableDirk2);

InputParameters
LStableDirk2::validParams()
{
  InputParameters params = TimeIntegrator::validParams();
  params.addClassDescription(
      "Second order diagonally implicit Runge Kutta method (Dirk) with two stages.");
  return params;
}

LStableDirk2::LStableDirk2(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    _stage(1),
    _residual_stage1(_nl.addVector("residual_stage1", false, GHOSTED)),
    _residual_stage2(_nl.addVector("residual_stage2", false, GHOSTED)),
    _alpha(1. - 0.5 * std::sqrt(2))
{
  mooseInfo("LStableDirk2 and other multistage TimeIntegrators are known not to work with "
            "Materials/AuxKernels that accumulate 'state' and should be used with caution.");
}

void
LStableDirk2::computeTimeDerivatives()
{
  // We are multiplying by the method coefficients in postResidual(), so
  // the time derivatives are of the same form at every stage although
  // the current solution varies depending on the stage.
  if (!_sys.solutionUDot())
    mooseError("LStableDirk2: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase befor requesting `u_dot`.");

  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  u_dot = *_solution;
  computeTimeDerivativeHelper(u_dot, _solution_old);
  u_dot.close();
  _du_dot_du = 1. / _dt;
}

void
LStableDirk2::computeADTimeDerivatives(DualReal & ad_u_dot,
                                       const dof_id_type & dof,
                                       DualReal & /*ad_u_dotdot*/) const
{
  computeTimeDerivativeHelper(ad_u_dot, _solution_old(dof));
}

void
LStableDirk2::solve()
{
  // Time at end of step
  Real time_new = _fe_problem.time();

  // Time at beginning of step
  Real time_old = _fe_problem.timeOld();

  // Time at stage 1
  Real time_stage1 = time_old + _alpha * _dt;

  // Reset iteration counts
  _n_nonlinear_iterations = 0;
  _n_linear_iterations = 0;

  // Compute first stage
  _fe_problem.initPetscOutput();
  _console << "1st stage" << std::endl;
  _stage = 1;
  _fe_problem.time() = time_stage1;
  _fe_problem.getNonlinearSystemBase().system().solve();
  _n_nonlinear_iterations += getNumNonlinearIterationsLastSolve();
  _n_linear_iterations += getNumLinearIterationsLastSolve();

  // Abort time step immediately on stage failure - see TimeIntegrator doc page
  if (!_fe_problem.converged())
    return;

  // Compute second stage
  _fe_problem.initPetscOutput();
  _console << "2nd stage" << std::endl;
  _stage = 2;
  _fe_problem.timeOld() = time_stage1;
  _fe_problem.time() = time_new;
  _fe_problem.getNonlinearSystemBase().system().solve();
  _n_nonlinear_iterations += getNumNonlinearIterationsLastSolve();
  _n_linear_iterations += getNumLinearIterationsLastSolve();

  // Reset time at beginning of step to its original value
  _fe_problem.timeOld() = time_old;
}

void
LStableDirk2::postResidual(NumericVector<Number> & residual)
{
  if (_stage == 1)
  {
    // In the standard RK notation, the stage 1 residual is given by:
    //
    // R := (Y_1 - y_n)/dt - alpha*f(t_n + alpha*dt, Y_1) = 0
    //
    // where:
    // .) f(t_n + alpha*dt, Y_1) corresponds to the residuals of the
    //    non-time kernels.  We'll save this as "_residual_stage1" to use
    //    later.
    // .) (Y_1 - y_n)/dt corresponds to the residual of the time kernels.
    // .) The minus sign in front of alpha is already "baked in" to
    //    the non-time residuals, so it does not appear here.
    _residual_stage1 = _Re_non_time;
    _residual_stage1.close();

    residual.add(1., _Re_time);
    residual.add(_alpha, _residual_stage1);
    residual.close();
  }
  else if (_stage == 2)
  {
    // In the standard RK notation, the stage 2 residual is given by:
    //
    // R := (Y_2 - y_n)/dt - (1-alpha)*f(t_n + alpha*dt, Y_1) - alpha*f(t_n + dt, Y_2) = 0
    //
    // where:
    // .) f(t_n + alpha*dt, Y_1) has already been saved as _residual_stage1.
    // .) f(t_n + dt, Y_2) will now be saved as "_residual_stage2".
    // .) (Y_2 - y_n)/dt corresponds to the residual of the time kernels.
    // .) The minus signs are once again "baked in" to the non-time
    //    residuals, so they do not appear here.
    //
    // The solution at the end of stage 2, i.e. Y_2, is also the final solution.
    _residual_stage2 = _Re_non_time;
    _residual_stage2.close();

    residual.add(1., _Re_time);
    residual.add(1. - _alpha, _residual_stage1);
    residual.add(_alpha, _residual_stage2);
    residual.close();
  }
  else
    mooseError("LStableDirk2::postResidual(): Member variable _stage can only have values 1 or 2.");
}
