//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LStableDirk3.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "PetscSupport.h"
using namespace libMesh;

registerMooseObject("MooseApp", LStableDirk3);

InputParameters
LStableDirk3::validParams()
{
  InputParameters params = TimeIntegrator::validParams();
  params.addClassDescription(
      "Third order diagonally implicit Runge Kutta method (Dirk) with three stages.");
  return params;
}

LStableDirk3::LStableDirk3(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    _stage(1),
    _gamma(-std::sqrt(2.) * std::cos(std::atan(std::sqrt(2.) / 4.) / 3.) / 2. +
           std::sqrt(6.) * std::sin(std::atan(std::sqrt(2.) / 4.) / 3.) / 2. + 1.)
{
  mooseInfo("LStableDirk3 and other multistage TimeIntegrators are known not to work with "
            "Materials/AuxKernels that accumulate 'state' and should be used with caution.");

  // Name the stage residuals "residual_stage1", "residual_stage2", etc.
  for (unsigned int stage = 0; stage < 3; ++stage)
  {
    std::ostringstream oss;
    oss << "residual_stage" << stage + 1;
    _stage_residuals[stage] = addVector(oss.str(), false, GHOSTED);
  }

  // Initialize parameters
  _c[0] = _gamma;
  _c[1] = .5 * (1 + _gamma);
  _c[2] = 1.0;

  _a[0][0] = _gamma;
  _a[1][0] = .5 * (1 - _gamma); /**/
  _a[1][1] = _gamma;
  _a[2][0] = .25 * (-6 * _gamma * _gamma + 16 * _gamma - 1); /**/
  _a[2][1] = .25 * (6 * _gamma * _gamma - 20 * _gamma + 5);  /**/
  _a[2][2] = _gamma;
}

void
LStableDirk3::computeTimeDerivatives()
{
  // We are multiplying by the method coefficients in postResidual(), so
  // the time derivatives are of the same form at every stage although
  // the current solution varies depending on the stage.
  if (!_sys.solutionUDot())
    mooseError("LStableDirk3: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase befor requesting `u_dot`.");

  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  u_dot = *_solution;
  computeTimeDerivativeHelper(u_dot, _solution_old);
  u_dot.close();
  computeDuDotDu();
}

void
LStableDirk3::computeADTimeDerivatives(ADReal & ad_u_dot,
                                       const dof_id_type & dof,
                                       ADReal & /*ad_u_dotdot*/) const
{
  computeTimeDerivativeHelper(ad_u_dot, _solution_old(dof));
}

void
LStableDirk3::solve()
{
  // Time at end of step
  Real time_old = _fe_problem.timeOld();

  // Reset iteration counts
  _n_nonlinear_iterations = 0;
  _n_linear_iterations = 0;

  // A for-loop would increment _stage too far, so we use an extra
  // loop counter.
  for (unsigned int current_stage = 1; current_stage < 4; ++current_stage)
  {
    // Set the current stage value
    _stage = current_stage;

    // This ensures that all the Output objects in the OutputWarehouse
    // have had solveSetup() called, and sets the default solver
    // parameters for PETSc.
    _fe_problem.initPetscOutputAndSomeSolverSettings();

    _console << "Stage " << _stage << std::endl;

    // Set the time for this stage
    _fe_problem.time() = time_old + _c[_stage - 1] * _dt;

    // If we previously used coloring, destroy the old object so it doesn't leak when we allocate a
    // new object in the following lines
    _nl->destroyColoring();

    // Potentially setup finite differencing contexts for the solve
    _nl->potentiallySetupFiniteDifferencing();

    // Do the solve
    _nl->system().solve();

    // Update the iteration counts
    _n_nonlinear_iterations += getNumNonlinearIterationsLastSolve();
    _n_linear_iterations += getNumLinearIterationsLastSolve();

    // Abort time step immediately on stage failure - see TimeIntegrator doc page
    if (!_fe_problem.converged(_nl->number()))
      return;
  }
}

void
LStableDirk3::postResidual(NumericVector<Number> & residual)
{
  // Error if _stage got messed up somehow.
  if (_stage > 3)
    mooseError(
        "LStableDirk3::postResidual(): Member variable _stage can only have values 1, 2, or 3.");

  // In the standard RK notation, the residual of stage 1 of s is given by:
  //
  // R := M*(Y_i - y_n)/dt - \sum_{j=1}^s a_{ij} * f(t_n + c_j*dt, Y_j) = 0
  //
  // where:
  // .) M is the mass matrix
  // .) Y_i is the stage solution
  // .) dt is the timestep, and is accounted for in the _Re_time residual.
  // .) f are the "non-time" residuals evaluated for a given stage solution.
  // .) The minus signs are already "baked in" to the residuals and so do not appear below.

  // Store this stage's non-time residual.  We are calling operator=
  // here, and that calls close().
  *_stage_residuals[_stage - 1] = *_Re_non_time;

  // Build up the residual for this stage.
  residual.add(1., *_Re_time);
  for (unsigned int j = 0; j < _stage; ++j)
    residual.add(_a[_stage - 1][j], *_stage_residuals[j]);
  residual.close();
}
