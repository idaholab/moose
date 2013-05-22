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

#include "DT2.h"
#include "FEProblem.h"
//libMesh includes
#include "libmesh/implicit_system.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/transient_system.h"
// C++ Includes
#include <iomanip>


template<>
InputParameters validParams<DT2>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addParam<Real>("dt", 1., "The initial time step size.");
  params.addRequiredParam<Real>("e_tol", "Target error tolerance.");
  params.addRequiredParam<Real>("e_max", "Maximum acceptable error.");
  params.addParam<Real>("max_increase", 1.0e9, "Maximum ratio that the time step can increase.");

  return params;
}

DT2::DT2(const std::string & name, InputParameters parameters) :
    TimeStepper(name, parameters),
    _u_diff(NULL),
    _u1(NULL),
    _u2(NULL),
    _u_saved(NULL),
    _u_older_saved(NULL),
    _aux1(NULL),
    _aux_saved(NULL),
    _aux_older_saved(NULL),
    _error(0.),
    _e_tol(getParam<Real>("e_tol")),
    _e_max(getParam<Real>("e_max")),
    _max_increase(getParam<Real>("max_increase"))
{
  _current_dt = getParam<Real>("dt");
}

DT2::~DT2()
{
}

void
DT2::preExecute()
{
  TransientNonlinearImplicitSystem & nl_sys = _fe_problem.getNonlinearSystem().sys();
  _u1 = &nl_sys.add_vector("u1", true, GHOSTED);
  _u2 = &nl_sys.add_vector("u2", false, GHOSTED);
  _u_diff = &nl_sys.add_vector("u_diff", false, GHOSTED);

  _u_saved = &nl_sys.add_vector("u_saved", false, GHOSTED);
  _u_older_saved = &nl_sys.add_vector("u_older_saved", false, GHOSTED);

  TransientExplicitSystem & aux_sys = _fe_problem.getAuxiliarySystem().sys();
  _aux1 = &aux_sys.add_vector("aux1", true, GHOSTED);
  _aux_saved = &aux_sys.add_vector("aux_saved", false, GHOSTED);
  _aux_older_saved = &aux_sys.add_vector("aux_older_saved", false, GHOSTED);
}

void
DT2::preSolve()
{
  TransientNonlinearImplicitSystem & nl_sys = _fe_problem.getNonlinearSystem().sys();
  TransientExplicitSystem & aux_sys = _fe_problem.getAuxiliarySystem().sys();

  // save solution vectors
  *_u_saved = *nl_sys.current_local_solution;
  *_u_older_saved = *nl_sys.older_local_solution;

  *_aux_saved = *aux_sys.current_local_solution;
  *_aux_older_saved = *aux_sys.older_local_solution;

  _u_saved->close();
  _u_older_saved->close();
  _aux_saved->close();
  _aux_older_saved->close();
}

void
DT2::step()
{
  NonlinearSystem & nl = _fe_problem.getNonlinearSystem(); // returned reference is not used for anything?
  TransientNonlinearImplicitSystem & nl_sys = _fe_problem.getNonlinearSystem().sys();
  TransientExplicitSystem & aux_sys = _fe_problem.getAuxiliarySystem().sys();

  // solve the problem with full dt
  _fe_problem.solve();
  _converged = nl.converged();
  if (_converged)
  {
    // save the solution (for time step with dt)
    *_u1 = *nl_sys.current_local_solution;
    _u1->close();
    *_aux1 = *aux_sys.current_local_solution;
    _aux1->close();

    // take two steps with dt/2
    std::cout << "Taking two dt/2 time steps" << std::endl;

    // restore solutions to the original state
    *nl_sys.current_local_solution = *_u_saved;
    *aux_sys.current_local_solution = *_aux_saved;
    nl_sys.current_local_solution->close();
    aux_sys.current_local_solution->close();

    _dt = _current_dt / 2;                 // cut the time step in half
    _time = _time_old + _dt;

    // 1. step
    _fe_problem.onTimestepBegin();
    // Compute Post-Aux User Objects (Timestep begin)
    _fe_problem.computeUserObjects();

    std::cout << "  - 1. step" << std::endl;
    Moose::setSolverDefaults(_fe_problem);
    nl.solve();
    _converged = nl.converged();
    if (_converged)
    {
      nl_sys.update();

      _fe_problem.computeUserObjects(EXEC_TIMESTEP, UserObjectWarehouse::PRE_AUX);
      _fe_problem.copyOldSolutions();

      _time += _dt;
      // 2. step
      _fe_problem.onTimestepBegin();
      _fe_problem.computeUserObjects();

      std::cout << "  - 2. step" << std::endl;
      Moose::setSolverDefaults(_fe_problem);
      nl.solve();
      _converged = nl.converged();
      if (_converged)
      {
        nl_sys.update();

        *_u2 = *nl_sys.current_local_solution;
        _u2->close();

        // compute error
        *_u_diff = *_u2;
        *_u_diff -= *_u1;
        _u_diff->close();

        _error = (_u_diff->l2_norm() / std::max(_u1->l2_norm(), _u2->l2_norm())) / _current_dt;

        // restore _dt to its original value
        _dt = _current_dt;
      }
      else
      {
        // solve failed, restore _time
        _time = _time_old;
      }
    }
    else
    {
      // solve failed, restore _time
      _time = _time_old;
    }

    if (!_converged)
    {
      *nl_sys.current_local_solution= *_u1;
      *nl_sys.old_local_solution = *_u1;
      *nl_sys.older_local_solution = *_u_saved;

      *aux_sys.current_local_solution = *_aux1;
      *aux_sys.old_local_solution = *_aux1;
      *aux_sys.older_local_solution = *_aux_saved;

      nl_sys.current_local_solution->close();
      nl_sys.old_local_solution->close();
      nl_sys.older_local_solution->close();
      aux_sys.current_local_solution->close();
      aux_sys.old_local_solution->close();
      aux_sys.older_local_solution->close();
    }
  }
}

Real
DT2::computeDT()
{
  if (_t_step < 2)
    return _current_dt;

  Real new_dt = _current_dt * std::pow(_e_tol / _error, 1.0 / _fe_problem.getNonlinearSystem().getTimeSteppingOrder());
  if (new_dt / _current_dt > _max_increase)
    new_dt = _current_dt * _max_increase;

  return new_dt;
}

void
DT2::rejectStep()
{
  if (_error >= _e_max)
    std::cout << "DT2Transient: Marking last solve not converged since |U2-U1|/max(|U2|,|U1|) = "
              << _error << " >= e_max." << std::endl;

  TransientNonlinearImplicitSystem & nl_sys = _fe_problem.getNonlinearSystem().sys();
  TransientExplicitSystem & aux_sys = _fe_problem.getAuxiliarySystem().sys();

  // recover initial state
  *nl_sys.current_local_solution = *_u_saved;
  *nl_sys.old_local_solution = *_u_saved;
  *nl_sys.older_local_solution = *_u_older_saved;

  *aux_sys.current_local_solution = *_aux_saved;
  *aux_sys.old_local_solution = *_aux_saved;
  *aux_sys.older_local_solution = *_aux_older_saved;

  nl_sys.solution->close();
  nl_sys.old_local_solution->close();
  nl_sys.older_local_solution->close();
  aux_sys.solution->close();
  aux_sys.old_local_solution->close();
  aux_sys.older_local_solution->close();

  // and cut the time step in a half
  _current_dt = _current_dt / 2.;
}

bool
DT2::converged()
{
  if (!_converged)
    return false;

  if (_error < _e_max)
    return true;
  else
    return false;
}
