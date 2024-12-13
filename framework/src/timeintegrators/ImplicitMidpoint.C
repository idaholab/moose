//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ImplicitMidpoint.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "PetscSupport.h"

registerMooseObject("MooseApp", ImplicitMidpoint);

InputParameters
ImplicitMidpoint::validParams()
{
  InputParameters params = TimeIntegrator::validParams();
  params.addClassDescription("Second-order Runge-Kutta (implicit midpoint) time integration.");
  return params;
}

ImplicitMidpoint::ImplicitMidpoint(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    _stage(1),
    _residual_stage1(addVector("residual_stage1", false, libMesh::GHOSTED))
{
  mooseInfo("ImplicitMidpoint and other multistage TimeIntegrators are known not to work with "
            "Materials/AuxKernels that accumulate 'state' and should be used with caution.");
}

void
ImplicitMidpoint::computeTimeDerivatives()
{
  // We are multiplying by the method coefficients in postResidual(), so
  // the time derivatives are of the same form at every stage although
  // the current solution varies depending on the stage.
  if (!_sys.solutionUDot())
    mooseError("ImplicitMidpoint: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase befor requesting `u_dot`.");

  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  u_dot = *_solution;
  computeTimeDerivativeHelper(u_dot, _solution_old);
  u_dot.close();
  computeDuDotDu();
}

void
ImplicitMidpoint::computeADTimeDerivatives(ADReal & ad_u_dot,
                                           const dof_id_type & dof,
                                           ADReal & /*ad_u_dotdot*/) const
{
  computeTimeDerivativeHelper(ad_u_dot, _solution_old(dof));
}

void
ImplicitMidpoint::solve()
{
  Real time_new = _fe_problem.time();
  Real time_old = _fe_problem.timeOld();
  Real time_half = (time_new + time_old) / 2.;

  // Reset iteration counts
  _n_nonlinear_iterations = 0;
  _n_linear_iterations = 0;

  // Compute first stage
  _fe_problem.initPetscOutputAndSomeSolverSettings();
  _console << "1st stage" << std::endl;
  _stage = 1;
  _fe_problem.time() = time_half;
  _nl->system().solve();
  _n_nonlinear_iterations += getNumNonlinearIterationsLastSolve();
  _n_linear_iterations += getNumLinearIterationsLastSolve();

  // Abort time step immediately on stage failure - see TimeIntegrator doc page
  if (!_fe_problem.converged(_nl->number()))
    return;

  // Compute second stage
  _fe_problem.initPetscOutputAndSomeSolverSettings();
  _console << "2nd stage" << std::endl;
  _stage = 2;
  _fe_problem.time() = time_new;
  _nl->system().solve();
  _n_nonlinear_iterations += getNumNonlinearIterationsLastSolve();
  _n_linear_iterations += getNumLinearIterationsLastSolve();
}

void
ImplicitMidpoint::postResidual(NumericVector<Number> & residual)
{
  if (_stage == 1)
  {
    // In the standard RK notation, the stage 1 residual is given by:
    //
    // R := M*(Y_1 - y_n)/dt - (1/2)*f(t_n + dt/2, Y_1) = 0
    //
    // where:
    // .) M is the mass matrix
    // .) f(t_n + dt/2, Y_1) is saved in _residual_stage1
    // .) The minus sign is baked in to the non-time residuals, so it does not appear here.
    *_residual_stage1 = *_Re_non_time;
    _residual_stage1->close();

    residual.add(1., *_Re_time);
    residual.add(0.5, *_Re_non_time);
    residual.close();
  }
  else if (_stage == 2)
  {
    // The update step.  In the standard RK notation, the update
    // residual is given by:
    //
    // R := M*(y_{n+1} - y_n)/dt - f(t_n + dt/2, Y_1) = 0
    //
    // where:
    // .) M is the mass matrix.
    // .) f(t_n + dt/2, Y_1) is the residual from stage 1, it has already
    //    been saved as _residual_stage1.
    // .) The minus signs are "baked in" to the non-time residuals, so
    //    they do not appear here.
    residual.add(1., *_Re_time);
    residual.add(1., *_residual_stage1);
    residual.close();
  }
  else
    mooseError(
        "ImplicitMidpoint::postResidual(): _stage = ", _stage, ", only _stage = 1, 2 is allowed.");
}
