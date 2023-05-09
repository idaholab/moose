//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AB2PredictorCorrector.h"
#include "AdamsPredictor.h"
#include "Problem.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"
#include "TimeIntegrator.h"
#include "Conversion.h"

#include "libmesh/nonlinear_solver.h"
#include "libmesh/numeric_vector.h"

// C++ Includes
#include <iomanip>
#include <iostream>
#include <fstream>

registerMooseObject("MooseApp", AB2PredictorCorrector);

InputParameters
AB2PredictorCorrector::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addClassDescription(
      "Implements second order Adams-Bashforth method for timestep calculation.");
  params.addRequiredParam<Real>("e_tol", "Target error tolerance.");
  params.addRequiredParam<Real>("e_max", "Maximum acceptable error.");
  params.addRequiredParam<Real>("dt", "Initial time step size");
  params.addParam<Real>("max_increase", 1.0e9, "Maximum ratio that the time step can increase.");
  params.addParam<int>(
      "steps_between_increase", 1, "the number of time steps before recalculating dt");
  params.addParam<int>("start_adapting", 2, "when to start taking adaptive time steps");
  params.addParam<Real>("scaling_parameter", .8, "scaling parameter for dt selection");
  return params;
}

AB2PredictorCorrector::AB2PredictorCorrector(const InputParameters & parameters)
  : TimeStepper(parameters),
    _u1(_fe_problem.getNonlinearSystemBase().addVector("u1", true, GHOSTED)),
    _aux1(_fe_problem.getAuxiliarySystem().addVector("aux1", true, GHOSTED)),
    _pred1(_fe_problem.getNonlinearSystemBase().addVector("pred1", true, GHOSTED)),
    _dt_full(declareRestartableData<Real>("dt_full", 0)),
    _error(declareRestartableData<Real>("error", 0)),
    _e_tol(getParam<Real>("e_tol")),
    _e_max(getParam<Real>("e_max")),
    _max_increase(getParam<Real>("max_increase")),
    _steps_between_increase(getParam<int>("steps_between_increase")),
    _dt_steps_taken(declareRestartableData<int>("dt_steps_taken", 0)),
    _start_adapting(getParam<int>("start_adapting")),
    _my_dt_old(declareRestartableData<Real>("my_dt_old", 0)),
    _infnorm(declareRestartableData<Real>("infnorm", 0)),
    _scaling_parameter(getParam<Real>("scaling_parameter"))
{
  Real predscale = 1.;
  InputParameters params = _app.getFactory().getValidParams("AdamsPredictor");
  params.set<Real>("scale") = predscale;
  _fe_problem.addPredictor("AdamsPredictor", "adamspredictor", params);
}

void
AB2PredictorCorrector::preExecute()
{
  TimeStepper::preExecute();
}

void
AB2PredictorCorrector::preSolve()
{
  // save dt
  _dt_full = _dt;
}

void
AB2PredictorCorrector::step()
{
  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();
  AuxiliarySystem & aux = _fe_problem.getAuxiliarySystem();

  _fe_problem.solve();
  _converged = _fe_problem.converged();
  if (_converged)
  {
    _u1 = *nl.currentSolution();
    _u1.close();

    _aux1 = *aux.currentSolution();
    _aux1.close();
    if (_t_step >= _start_adapting)
    {
      // Calculate error if past the first solve
      _error = estimateTimeError(_u1);

      _infnorm = _u1.linfty_norm();
      _e_max = 1.1 * _e_tol * _infnorm;
      _console << "Time Error Estimate: " << _error << std::endl;
    }
    else
    {
      // First time step is problematic, sure we converged but what does that mean? We don't know.
      // Nor can we calculate the error on the first time step.
    }
  }
}

bool
AB2PredictorCorrector::converged() const
{
  if (!_converged)
    return false;
  if (_error < _e_max)
    return true;
  else
    return false;
}

void
AB2PredictorCorrector::postSolve()
{
  if (!converged())
    _dt_steps_taken = 0;

  if (_error >= _e_max)
    _console << "Marking last solve not converged " << _error << " " << _e_max << std::endl;
}

Real
AB2PredictorCorrector::computeDT()
{
  if (_t_step <= _start_adapting)
    return _dt;

  _my_dt_old = _dt;

  _dt_steps_taken += 1;
  if (_dt_steps_taken >= _steps_between_increase)
  {

    Real new_dt = _dt_full * _scaling_parameter * std::pow(_infnorm * _e_tol / _error, 1.0 / 3.0);

    if (new_dt / _dt_full > _max_increase)
      new_dt = _dt_full * _max_increase;
    _dt_steps_taken = 0;
    return new_dt;
  }

  return _dt;
}

Real
AB2PredictorCorrector::computeInitialDT()
{
  return getParam<Real>("dt");
}

Real
AB2PredictorCorrector::estimateTimeError(NumericVector<Number> & solution)
{
  _pred1 = _fe_problem.getNonlinearSystemBase().getPredictor()->solutionPredictor();
  TimeIntegrator * ti = _fe_problem.getNonlinearSystemBase().getTimeIntegrator();
  auto scheme = Moose::stringToEnum<Moose::TimeIntegratorType>(ti->type());
  Real dt_old = _my_dt_old;
  if (dt_old == 0)
    dt_old = _dt;

  switch (scheme)
  {
    case Moose::TI_IMPLICIT_EULER:
    {
      _pred1 *= -1;
      _pred1 += solution;
      Real calc = _dt * _dt * .5;
      _pred1 *= calc;
      return _pred1.l2_norm();
    }
    case Moose::TI_CRANK_NICOLSON:
    {
      _pred1 -= solution;
      _pred1 *= (_dt) / (3.0 * (_dt + dt_old));
      return _pred1.l2_norm();
    }
    case Moose::TI_BDF2:
    {
      _pred1 *= -1.0;
      _pred1 += solution;
      Real topcalc = 2.0 * (_dt + dt_old) * (_dt + dt_old);
      Real bottomcalc = 6.0 * _dt * _dt + 12.0 * _dt * dt_old + 5.0 * dt_old * dt_old;
      _pred1 *= topcalc / bottomcalc;

      return _pred1.l2_norm();
    }
    default:
      break;
  }
  return -1;
}
