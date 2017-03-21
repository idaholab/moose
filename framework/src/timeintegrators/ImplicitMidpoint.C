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

#include "ImplicitMidpoint.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "PetscSupport.h"

template <>
InputParameters
validParams<ImplicitMidpoint>()
{
  InputParameters params = validParams<TimeIntegrator>();

  return params;
}

ImplicitMidpoint::ImplicitMidpoint(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    _stage(1),
    _residual_stage1(_nl.addVector("residual_stage1", false, GHOSTED))
{
}

ImplicitMidpoint::~ImplicitMidpoint() {}

void
ImplicitMidpoint::computeTimeDerivatives()
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
ImplicitMidpoint::solve()
{
  Real time_new = _fe_problem.time();
  Real time_old = _fe_problem.timeOld();
  Real time_half = (time_new + time_old) / 2.;

  // Compute first stage
  _fe_problem.initPetscOutput();
  _console << "1st stage\n";
  _stage = 1;
  _fe_problem.time() = time_half;
  _fe_problem.getNonlinearSystemBase().system().solve();

  // Compute second stage
  _fe_problem.initPetscOutput();
  _console << "2nd stage\n";
  _stage = 2;
  _fe_problem.time() = time_new;
  _fe_problem.getNonlinearSystemBase().system().solve();
}

void
ImplicitMidpoint::postStep(NumericVector<Number> & residual)
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
    _residual_stage1 = _Re_non_time;
    _residual_stage1.close();

    residual.add(1., _Re_time);
    residual.add(0.5, _Re_non_time);
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
    residual.add(1., _Re_time);
    residual.add(1., _residual_stage1);
    residual.close();
  }
  else
    mooseError(
        "ImplicitMidpoint::postStep(): _stage = ", _stage, ", only _stage = 1, 2 is allowed.");
}
