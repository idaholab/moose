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

#include "DT2Transient.h"

//Moose includes
#include "Kernel.h"
#include "MaterialFactory.h"
#include "MooseSystem.h"
#include "ComputeJacobian.h"
#include "ComputeResidual.h"

//libMesh includes
#include "implicit_system.h"
#include "nonlinear_implicit_system.h"
#include "transient_system.h"
#include "numeric_vector.h"

// C++ Includes
#include <iomanip>

template<>
InputParameters validParams<DT2Transient>()
{
  InputParameters params = validParams<TransientExecutioner>();
  params.addRequiredParam<Real>("e_tol","Target error tolerance.");
  params.addRequiredParam<Real>("e_max","Maximum acceptable error.");
  params.addParam<Real>("max_increase", 1.0e9,    "Maximum ratio that the time step can increase.");

  return params;
}

DT2Transient::DT2Transient(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :TransientExecutioner(name, moose_system, parameters),
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
   _max_increase(getParam<Real>("max_increase"))
{
}

DT2Transient::~DT2Transient()
{
}

void
DT2Transient::preExecute()
{
  TransientNonlinearImplicitSystem *nl_sys = _moose_system.getNonlinearSystem();
  _u1 = &nl_sys->add_vector("u1", false, GHOSTED);
  _u2 = &nl_sys->add_vector("u2", false, GHOSTED);
  _u_diff = &nl_sys->add_vector("u_diff", false, GHOSTED);

  _u_saved = &nl_sys->add_vector("u_saved", false, GHOSTED);
  _u_older_saved = &nl_sys->add_vector("u_older_saved", false, GHOSTED);

  TransientExplicitSystem *aux_sys = _moose_system.getAuxSystem();
  _aux1 = &aux_sys->add_vector("aux1", false, GHOSTED);
  _aux_saved = &aux_sys->add_vector("aux_saved", false, GHOSTED);
  _aux_older_saved = &aux_sys->add_vector("aux_older_saved", false, GHOSTED);
}

void
DT2Transient::preSolve()
{
  TransientNonlinearImplicitSystem *nl_sys = _moose_system.getNonlinearSystem();
  TransientExplicitSystem *aux_sys = _moose_system.getAuxSystem();

  // save solution vectors
  *_u_saved = *nl_sys->current_local_solution;
  *_u_older_saved = *nl_sys->older_local_solution;

  *_aux_saved = *aux_sys->current_local_solution;
  *_aux_older_saved = *aux_sys->older_local_solution;

  _u_saved->close();
  _u_older_saved->close();
  _aux_saved->close();
  _aux_older_saved->close();

  // save dt
  _dt_full = _dt;
}

void
DT2Transient::postSolve()
{
  TransientNonlinearImplicitSystem *nl_sys = _moose_system.getNonlinearSystem();
  TransientExplicitSystem *aux_sys = _moose_system.getAuxSystem();
  if (_converged)
  {
    // save the solution (for time step with dt)
    *_u1 = *nl_sys->current_local_solution;
    _u1->close();
    
    *_aux1 = *aux_sys->current_local_solution;
    _aux1->close();
    
    // take two steps with dt/2
    std::cout << "Taking two dt/2 time steps" << std::endl;

    // go back in time
    *nl_sys->current_local_solution = *_u_saved;
    *aux_sys->current_local_solution = *_aux_saved;
    nl_sys->current_local_solution->close();
    aux_sys->current_local_solution->close();
    _time -= _dt_full;
    
    // cut the time step in half
    _dt = _dt_full / 2;
    _moose_system.reinitDT();
    
    // 1. step
    _moose_system.onTimestepBegin();
    _time += _dt;
    _moose_system.reinitDT();
    
    std::cout << "  - 1. step" << std::endl;
    Moose::setSolverDefaults(_moose_system, this);
    setScaling();
    nl_sys->solve();

    _converged = nl_sys->nonlinear_solver->converged;
    if (!_converged) return;
    nl_sys->update();

    _moose_system.copy_old_solutions();

    // 2. step
    _moose_system.onTimestepBegin();
    _time += _dt;
    _moose_system.reinitDT();
    
    std::cout << "  - 2. step" << std::endl;   
    Moose::setSolverDefaults(_moose_system, this);   
    setScaling();
    nl_sys->solve();

    _converged = nl_sys->nonlinear_solver->converged;
    if (!_converged) return;
    nl_sys->update();
    
    *_u2 = *nl_sys->current_local_solution;
    _u2->close();

    // compute error
    *_u_diff = *_u2;
    *_u_diff -= *_u1;
    _u_diff->close();
    
    _error = (_u_diff->l2_norm() / std::max(_u1->l2_norm(), _u2->l2_norm())) / _dt_full;
  }
  
}

bool
DT2Transient::lastSolveConverged()
{
  if (!_converged)
    return false;

  if (_error < _e_max)
    return true;
  else
    return false;
  
}  

Real
DT2Transient::computeDT()
{
  if(_t_step == 1)
    return TransientExecutioner::computeDT();

  TransientNonlinearImplicitSystem *nl_sys = _moose_system.getNonlinearSystem();
  TransientExplicitSystem *aux_sys = _moose_system.getAuxSystem();

  if(lastSolveConverged())
  {
    Real new_dt = _dt_full * std::pow(_e_tol / _error, 1.0 / _moose_system.getTimeSteppingOrder());

    if (new_dt/_dt_full > _max_increase)
      _dt = _dt_full*_max_increase;
    else
      _dt = new_dt;
    
    *nl_sys->current_local_solution = *_u1;
    *nl_sys->old_local_solution = *_u1;
    *nl_sys->older_local_solution = *_u_saved;

    *aux_sys->current_local_solution = *_aux1;
    *aux_sys->old_local_solution = *_aux1;
    *aux_sys->older_local_solution = *_aux_saved;

    nl_sys->current_local_solution->close();
    nl_sys->old_local_solution->close();
    nl_sys->older_local_solution->close();
    aux_sys->current_local_solution->close();
    aux_sys->old_local_solution->close();
    aux_sys->older_local_solution->close();
  }
  else
  {
    // reject the step
    _time -= _dt;
    _dt = _dt_full;

    // recover initial state
    *nl_sys->current_local_solution = *_u_saved;
    *nl_sys->old_local_solution = *_u_saved;
    *nl_sys->older_local_solution = *_u_older_saved;

    *aux_sys->current_local_solution = *_aux_saved;
    *aux_sys->old_local_solution = *_aux_saved;
    *aux_sys->older_local_solution = *_aux_older_saved;

    nl_sys->current_local_solution->close();
    nl_sys->old_local_solution->close();
    nl_sys->older_local_solution->close();
    aux_sys->current_local_solution->close();
    aux_sys->old_local_solution->close();
    aux_sys->older_local_solution->close();
  }

  return TransientExecutioner::computeDT();
}
