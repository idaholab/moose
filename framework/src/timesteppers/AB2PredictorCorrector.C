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

#include "AB2PredictorCorrector.h"
#include "AdamsPredictor.h"
#include "Problem.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"
//libMesh includes
#include "libmesh/nonlinear_solver.h"

// C++ Includes
#include <iomanip>
#include <iostream>
#include <fstream>

template<>
InputParameters validParams<AB2PredictorCorrector>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addRequiredParam<Real>("e_tol","Target error tolerance.");
  params.addRequiredParam<Real>("e_max","Maximum acceptable error.");
  params.addRequiredParam<Real>("dt", "Initial time step size");
  params.addParam<Real>("max_increase", 1.0e9,    "Maximum ratio that the time step can increase.");
  params.addParam<int>("steps_between_increase",1,"the number of time steps before recalculating dt");
  params.addParam<int>("start_adapting",2, "when to start taking adaptive time steps");
  params.addParam<Real>("scaling_parameter", .8, "scaling parameter for dt selection");
  return params;
}

AB2PredictorCorrector::AB2PredictorCorrector(const std::string & name, InputParameters parameters) :
    TimeStepper(name, parameters),
    _u1(_fe_problem.getNonlinearSystem().addVector("u1", true, GHOSTED)),
    _aux1(_fe_problem.getAuxiliarySystem().addVector("aux1", true, GHOSTED)),
    _e_tol(getParam<Real>("e_tol")),
    _e_max(getParam<Real>("e_max")),
    _max_increase(getParam<Real>("max_increase")),
    _steps_between_increase(getParam<int>("steps_between_increase")),
    _dt_steps_taken(0),
    _start_adapting(getParam<int>("start_adapting")),
    _my_dt_old(0),
    _infnorm(0),
    _scaling_parameter(getParam<Real>("scaling_parameter"))
{
  _error = 0;
  Real predscale = 1.;
  InputParameters params = _app.getFactory().getValidParams("AdamsPredictor");
  params.set<Real>("scale") = predscale;
  _fe_problem.addPredictor("AdamsPredictor", "adamspredictor", params);

}

AB2PredictorCorrector::~AB2PredictorCorrector()
{
}

void
AB2PredictorCorrector::preExecute()
{
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
  NonlinearSystem & nl = _fe_problem.getNonlinearSystem();
  AuxiliarySystem & aux = _fe_problem.getAuxiliarySystem();

  _fe_problem.step();
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
      std::cout << "Time Error Estimate: " << _error << std::endl;
    }
    else
    {
      //First time step is problematic, sure we converged but what does that mean? We don't know.
      //Nor can we calculate the error on the first time step.
    }
  }
}

bool
AB2PredictorCorrector::converged()
{
  if (!_converged)
  {
    _dt_steps_taken = 0;
    return false;
  }
  if (_error < _e_max)
  {
    return true;
  }
  else
  {
    std::cout << "Marking last solve not converged " << _error << " " << _e_max << std::endl;
    _dt_steps_taken = 0;
    return false;
  }
}

Real
AB2PredictorCorrector::computeDT()
{
  if (_t_step <= _start_adapting)
    return _dt;

  _my_dt_old = _dt;
  _dt_steps_taken += 1;
  if(_dt_steps_taken >= _steps_between_increase)
  {
    Real new_dt = _dt_full * _scaling_parameter * std::pow(_infnorm * _e_tol / _error, 1.0 / 3.0);
    if (new_dt / _dt_full > _max_increase)
      _dt = _dt_full*_max_increase;
    else
      _dt = new_dt;
    _dt_steps_taken = 0;
  }
  return _dt;
}

Real
AB2PredictorCorrector::computeInitialDT()
{
  return getParam<Real>("dt");
}

int
AB2PredictorCorrector::stringtoint(std::string string)
{
  if (string == "ImplicitEuler")
    return 0;
  else if (string == "CrankNicolson")
    return 2;
  else if (string == "BDF2")
    return 3;
  return 4;
}

Real
AB2PredictorCorrector::estimateTimeError(NumericVector<Number> & solution)
{
  NumericVector<Number> & predicted_solution = (static_cast<AdamsPredictor*> (_fe_problem.getNonlinearSystem().getPredictor()))->predictedSolution();
  TimeIntegrator * ti = _fe_problem.getNonlinearSystem().getTimeIntegrator();
  std::string scheme = ti->name();
  Real dt_old = _fe_problem.dtOld();
  switch (stringtoint(scheme))
  {
  case 1:
  {
    // NOTE: this is never called, since stringtoint does not return 1 - EVER!
    //I am not sure this is actually correct.
    predicted_solution *= -1;
    predicted_solution += solution;
    Real calc = _dt * _dt * .5;
    predicted_solution *= calc;
    return predicted_solution.l2_norm();
  }
  case 2:
  {
    // Crank Nicolson
    predicted_solution -= solution;
    predicted_solution *= (_dt) / (3.0 * (_dt + dt_old));
    return predicted_solution.l2_norm();
  }
  case 3:
  {
    // BDF2
    predicted_solution *= -1.0;
    predicted_solution += solution;
    Real topcalc = 2.0 * (_dt + dt_old) * (_dt + dt_old);
    Real bottomcalc = 6.0 * _dt * _dt + 12.0 * _dt * dt_old + 5.0 * dt_old * dt_old;
    predicted_solution *= topcalc / bottomcalc;
    return predicted_solution.l2_norm();
  }
  default:
    break;
  }
  return -1;
}
