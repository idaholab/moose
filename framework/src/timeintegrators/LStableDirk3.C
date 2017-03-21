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

#include "LStableDirk3.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "PetscSupport.h"

template <>
InputParameters
validParams<LStableDirk3>()
{
  InputParameters params = validParams<TimeIntegrator>();
  return params;
}

LStableDirk3::LStableDirk3(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    _stage(1),
    _gamma(-std::sqrt(2.) * std::cos(std::atan(std::sqrt(2.) / 4.) / 3.) / 2. +
           std::sqrt(6.) * std::sin(std::atan(std::sqrt(2.) / 4.) / 3.) / 2. + 1.)
{
  // Name the stage residuals "residual_stage1", "residual_stage2", etc.
  for (unsigned int stage = 0; stage < 3; ++stage)
  {
    std::ostringstream oss;
    oss << "residual_stage" << stage + 1;
    _stage_residuals[stage] = &(_nl.addVector(oss.str(), false, GHOSTED));
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

LStableDirk3::~LStableDirk3() {}

void
LStableDirk3::computeTimeDerivatives()
{
  // We are multiplying by the method coefficients in postStep(), so
  // the time derivatives are of the same form at every stage although
  // the current solution varies depending on the stage.
  _u_dot = *_solution;
  _u_dot -= _solution_old;
  _u_dot *= 1. / _dt;
  _u_dot.close();
  _du_dot_du = 1. / _dt;
}

void
LStableDirk3::solve()
{
  // Time at end of step
  Real time_old = _fe_problem.timeOld();

  // A for-loop would increment _stage too far, so we use an extra
  // loop counter.
  for (unsigned int current_stage = 1; current_stage < 4; ++current_stage)
  {
    // Set the current stage value
    _stage = current_stage;

    // This ensures that all the Output objects in the OutputWarehouse
    // have had solveSetup() called, and sets the default solver
    // parameters for PETSc.
    _fe_problem.initPetscOutput();

    _console << "Stage " << _stage << "\n";

    // Set the time for this stage
    _fe_problem.time() = time_old + _c[_stage - 1] * _dt;

    // Do the solve
    _fe_problem.getNonlinearSystemBase().system().solve();
  }
}

void
LStableDirk3::postStep(NumericVector<Number> & residual)
{
  // Error if _stage got messed up somehow.
  if (_stage > 3)
    mooseError("LStableDirk3::postStep(): Member variable _stage can only have values 1, 2, or 3.");

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
