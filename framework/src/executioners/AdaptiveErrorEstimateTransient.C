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

#include "AdaptiveErrorEstimateTransient.h"
#include "Problem.h"
#include "TimeScheme.h"

//libMesh includes
#include "implicit_system.h"
#include "nonlinear_implicit_system.h"
#include "nonlinear_solver.h"
#include "transient_system.h"

// C++ Includes
#include <iomanip>

template<>
InputParameters validParams<AdaptiveErrorEstimateTransient>()
{
  InputParameters params = validParams<Transient>();
  params.addRequiredParam<Real>("e_tol","Target error tolerance.");
  params.addRequiredParam<Real>("e_max","Maximum acceptable error.");
  params.addParam<Real>("max_increase", 1.0e9,    "Maximum ratio that the time step can increase.");
  params.addParam<int>("steps_between_increase",1,"the number of time steps before recalculating dt");
  params.addParam<int>("start_adapting",2, "when to start taking adaptive time steps");
  params.addParam<Real>("scaling_parameter", .8, "scaling parameter for dt selection");
  return params;
}

AdaptiveErrorEstimateTransient::AdaptiveErrorEstimateTransient(const std::string & name, InputParameters parameters) :
    Transient(name, parameters),
    _u_diff(NULL),
    _u1(NULL),
    _u2(NULL),
    _u_saved(NULL),
    _u_older_saved(NULL),
    _aux1(NULL),
    _aux_saved(NULL),
    _aux_older_saved(NULL),
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
  _error=0;
  _problem.getNonlinearSystem()._time_scheme->useAB2Predictor();

}

AdaptiveErrorEstimateTransient::~AdaptiveErrorEstimateTransient()
{

}

void
AdaptiveErrorEstimateTransient::preExecute()
{
  TransientNonlinearImplicitSystem & nl_sys = _problem.getNonlinearSystem().sys();
  _u1 = &nl_sys.add_vector("u1", true, GHOSTED);

  TransientExplicitSystem & aux_sys = _problem.getAuxiliarySystem().sys();
  _aux1 = &aux_sys.add_vector("aux1", true, GHOSTED);

  _problem.getNonlinearSystem().setPredictorScale(1.0);
  Transient::preExecute();
}


void
AdaptiveErrorEstimateTransient::preSolve()
{
  // save dt
  _dt_full = _dt;
}

void
AdaptiveErrorEstimateTransient::postSolve()
{
  /*NonlinearSystem & nl =*/ _problem.getNonlinearSystem(); // returned reference is not used for anything?
  TransientNonlinearImplicitSystem & nl_sys = _problem.getNonlinearSystem().sys();
  TransientExplicitSystem & aux_sys = _problem.getAuxiliarySystem().sys();
  if (_converged)
  {

    *_u1 = *nl_sys.current_local_solution;
    _u1->close();

    *_aux1 = *aux_sys.current_local_solution;
    _aux1->close();
    if(_t_step >= _start_adapting)
    {
    	// Calculate error if past the first solve
    	_error = _problem.getNonlinearSystem()._time_scheme->estimateTimeError(*_u1);
    	_infnorm = _u1->linfty_norm();
    	_e_max = 1.1* _e_tol*_infnorm;
    	std::cout<<"Time Error Estimate: "<<_error<<std::endl;
    }
    else
    {
    	//First time step is problematic, sure we converged but what does that mean? We don't know.
    	//Nor can we calculate the error on the first time step.

    }
    Transient::postSolve();
  }
}

bool
AdaptiveErrorEstimateTransient::lastSolveConverged()
{
  if (!_converged)
  {
	_dt_steps_taken =0;
    return false;
  }
  if (_error < _e_max)
  {
    return true;
  }
  else
  {
    std::cout << "AEETransient: Marking last solve not converged " << _error<<" "<<_e_max<< std::endl;
    _dt_steps_taken =0;
    return false;
  }
}

Real
AdaptiveErrorEstimateTransient::computeDT()
{
  if(_t_step <= _start_adapting)
    return Transient::computeDT();

  if(lastSolveConverged())
  {
    _my_dt_old = _dt;
    _dt_steps_taken += 1;
    if(_dt_steps_taken >= _steps_between_increase)
    {
      Real new_dt = _dt_full * _scaling_parameter * std::pow(_infnorm * _e_tol / _error, 1.0 / 3.0);
      if (new_dt/_dt_full > _max_increase)
      {
        _dt = _dt_full*_max_increase;
      }
      else
      {
        _dt = new_dt;
      }

      _dt_steps_taken =0;
    }
  }
  else
  {
    // reject the step
    _time -= _dt;
  }

  return Transient::computeDT();
}
