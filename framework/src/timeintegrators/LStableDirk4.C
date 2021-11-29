//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LStableDirk4.h"
#include "NonlinearSystemBase.h"
#include "FEProblem.h"
#include "PetscSupport.h"

registerMooseObject("MooseApp", LStableDirk4);

InputParameters
LStableDirk4::validParams()
{
  InputParameters params = TimeIntegrator::validParams();
  params.addClassDescription(
      "Fourth-order diagonally implicit Runge Kutta method (Dirk) with five stages.");
  return params;
}

// Initialize static data
const Real LStableDirk4::_c[LStableDirk4::_n_stages] = {.25, 0., .5, 1., 1.};

const Real LStableDirk4::_a[LStableDirk4::_n_stages][LStableDirk4::_n_stages] = {
    {.25, 0, 0, 0, 0},
    {-.25, .25, 0, 0, 0},
    {.125, .125, .25, 0, 0},
    {-1.5, .75, 1.5, .25, 0},
    {0, 1. / 6, 2. / 3, -1. / 12, .25}};

LStableDirk4::LStableDirk4(const InputParameters & parameters)
  : TimeIntegrator(parameters), _stage(1)
{
  mooseInfo("LStableDirk4 and other multistage TimeIntegrators are known not to work with "
            "Materials/AuxKernels that accumulate 'state' and should be used with caution.");

  // Name the stage residuals "residual_stage1", "residual_stage2", etc.
  for (unsigned int stage = 0; stage < _n_stages; ++stage)
  {
    std::ostringstream oss;
    oss << "residual_stage" << stage + 1;
    _stage_residuals[stage] = &(_nl.addVector(oss.str(), false, GHOSTED));
  }
}

void
LStableDirk4::computeTimeDerivatives()
{
  // We are multiplying by the method coefficients in postResidual(), so
  // the time derivatives are of the same form at every stage although
  // the current solution varies depending on the stage.
  if (!_sys.solutionUDot())
    mooseError("LStableDirk4: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase befor requesting `u_dot`.");

  NumericVector<Number> & u_dot = *_sys.solutionUDot();
  u_dot = *_solution;
  computeTimeDerivativeHelper(u_dot, _solution_old);
  u_dot.close();
  _du_dot_du = 1. / _dt;
}

void
LStableDirk4::computeADTimeDerivatives(DualReal & ad_u_dot,
                                       const dof_id_type & dof,
                                       DualReal & /*ad_u_dotdot*/) const
{
  computeTimeDerivativeHelper(ad_u_dot, _solution_old(dof));
}

void
LStableDirk4::solve()
{
  // Time at end of step
  Real time_old = _fe_problem.timeOld();

  // Reset iteration counts
  _n_nonlinear_iterations = 0;
  _n_linear_iterations = 0;

  // A for-loop would increment _stage too far, so we use an extra
  // loop counter.
  for (unsigned int current_stage = 1; current_stage <= _n_stages; ++current_stage)
  {
    // Set the current stage value
    _stage = current_stage;

    // This ensures that all the Output objects in the OutputWarehouse
    // have had solveSetup() called, and sets the default solver
    // parameters for PETSc.
    _fe_problem.initPetscOutput();

    _console << "Stage " << _stage << std::endl;

    // Set the time for this stage
    _fe_problem.time() = time_old + _c[_stage - 1] * _dt;

    // Do the solve
    _fe_problem.getNonlinearSystemBase().system().solve();

    // Update the iteration counts
    _n_nonlinear_iterations += getNumNonlinearIterationsLastSolve();
    _n_linear_iterations += getNumLinearIterationsLastSolve();

    // Abort time step immediately on stage failure - see TimeIntegrator doc page
    if (!_fe_problem.converged())
      return;
  }
}

void
LStableDirk4::postResidual(NumericVector<Number> & residual)
{
  // Error if _stage got messed up somehow.
  if (_stage > _n_stages)
    // the explicit cast prevents strange compiler weirdness with the static
    // const variable and the variadic mooseError function
    mooseError("LStableDirk4::postResidual(): Member variable _stage can only have values 1-",
               (unsigned int)_n_stages,
               ".");

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
  *_stage_residuals[_stage - 1] = _Re_non_time;

  // Build up the residual for this stage.
  residual.add(1., _Re_time);
  for (unsigned int j = 0; j < _stage; ++j)
    residual.add(_a[_stage - 1][j], *_stage_residuals[j]);
  residual.close();
}
