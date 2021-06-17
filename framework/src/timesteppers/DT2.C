//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "DT2.h"

#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "NonlinearSystemBase.h"
#include "TimeIntegrator.h"

#include "libmesh/implicit_system.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/transient_system.h"
#include "libmesh/numeric_vector.h"

// C++ Includes
#include <iomanip>

registerMooseObject("MooseApp", DT2);

defineLegacyParams(DT2);

InputParameters
DT2::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addClassDescription(
      "An adaptive timestepper that compares the solution obtained from a single step of size dt "
      "with two steps of size dt/2 and adjusts the next timestep accordingly.");
  params.addParam<Real>("dt", 1., "The initial time step size.");
  params.addRequiredParam<Real>("e_tol", "Target error tolerance.");
  params.addRequiredParam<Real>("e_max", "Maximum acceptable error.");
  params.addParam<Real>("max_increase", 1.0e9, "Maximum ratio that the time step can increase.");

  return params;
}

DT2::DT2(const InputParameters & parameters)
  : TimeStepper(parameters),
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
}

void
DT2::preExecute()
{
  TimeStepper::preExecute();
  System & nl_sys = _fe_problem.getNonlinearSystemBase().system();
  _u1 = &nl_sys.add_vector("u1", true, GHOSTED);
  _u2 = &nl_sys.add_vector("u2", false, GHOSTED);
  _u_diff = &nl_sys.add_vector("u_diff", false, GHOSTED);

  _u_saved = &nl_sys.add_vector("u_saved", false, GHOSTED);
  _u_older_saved = &nl_sys.add_vector("u_older_saved", false, GHOSTED);

  auto & aux_sys = _fe_problem.getAuxiliarySystem().sys();
  _aux1 = &aux_sys.add_vector("aux1", true, GHOSTED);
  _aux_saved = &aux_sys.add_vector("aux_saved", false, GHOSTED);
  _aux_older_saved = &aux_sys.add_vector("aux_older_saved", false, GHOSTED);
}

void
DT2::preSolve()
{
  NonlinearSystemBase & nl_sys = _fe_problem.getNonlinearSystemBase();
  auto & aux = _fe_problem.getAuxiliarySystem();
  auto & aux_sys = aux.sys();

  // save solution vectors
  *_u_saved = *nl_sys.currentSolution();
  *_u_older_saved = nl_sys.solutionOlder();

  *_aux_saved = *aux_sys.current_local_solution;
  *_aux_older_saved = aux.solutionOlder();

  _u_saved->close();
  _u_older_saved->close();
  _aux_saved->close();
  _aux_older_saved->close();
}

void
DT2::step()
{
  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();
  System & nl_sys = nl.system();
  auto & aux = _fe_problem.getAuxiliarySystem();
  auto & aux_sys = aux.sys();

  // solve the problem with full dt
  _fe_problem.solve();
  _converged = nl.converged();
  if (_converged)
  {
    // save the solution (for time step with dt)
    *_u1 = *nl.currentSolution();
    _u1->close();
    *_aux1 = *aux_sys.current_local_solution;
    _aux1->close();

    // take two steps with dt/2
    _console << "Taking two dt/2 time steps" << std::endl;

    // restore solutions to the original state
    *nl_sys.current_local_solution = *_u_saved;
    *aux_sys.current_local_solution = *_aux_saved;
    nl_sys.current_local_solution->close();
    aux_sys.current_local_solution->close();

    _dt = getCurrentDT() / 2; // cut the time step in half
    _time = _time_old + _dt;

    // 1. step
    _fe_problem.onTimestepBegin();
    _fe_problem.execute(EXEC_TIMESTEP_BEGIN);

    _console << "  - 1. step" << std::endl;
    Moose::setSolverDefaults(_fe_problem);
    nl.solve();
    _converged = nl.converged();

    if (_converged)
    {
      nl_sys.update();

      _fe_problem.execute(EXEC_TIMESTEP_END);
      _fe_problem.advanceState();

      _time += _dt;
      // 2. step
      _fe_problem.onTimestepBegin();
      _fe_problem.execute(EXEC_TIMESTEP_BEGIN);

      _console << "  - 2. step" << std::endl;
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

        _error = (_u_diff->l2_norm() / std::max(_u1->l2_norm(), _u2->l2_norm())) / getCurrentDT();

        // restore _dt to its original value
        _dt = getCurrentDT();
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
      *nl_sys.current_local_solution = *_u1;
      nl.solutionOld() = *_u1;
      nl.solutionOlder() = *_u_saved;

      *aux_sys.current_local_solution = *_aux1;
      aux.solutionOld() = *_aux1;
      aux.solutionOlder() = *_aux_saved;

      nl_sys.current_local_solution->close();
      nl.solutionOld().close();
      nl.solutionOlder().close();
      aux_sys.current_local_solution->close();
      aux.solutionOld().close();
      aux.solutionOlder().close();
    }
  }
}

Real
DT2::computeInitialDT()
{
  return getParam<Real>("dt");
}

Real
DT2::computeDT()
{
  Real curr_dt = getCurrentDT();
  Real new_dt =
      curr_dt * std::pow(_e_tol / _error,
                         1.0 / _fe_problem.getNonlinearSystemBase().getTimeIntegrator()->order());
  if (new_dt / curr_dt > _max_increase)
    new_dt = curr_dt * _max_increase;

  return new_dt;
}

void
DT2::rejectStep()
{
  if (_error >= _e_max)
    _console << "DT2Transient: Marking last solve not converged since |U2-U1|/max(|U2|,|U1|) = "
             << _error << " >= e_max." << std::endl;

  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();
  System & nl_sys = nl.system();
  auto & aux = _fe_problem.getAuxiliarySystem();
  auto & aux_sys = aux.sys();

  // recover initial state
  *nl_sys.current_local_solution = *_u_saved;
  nl.solutionOld() = *_u_saved;
  nl.solutionOlder() = *_u_older_saved;

  *aux_sys.current_local_solution = *_aux_saved;
  aux.solutionOld() = *_aux_saved;
  aux.solutionOlder() = *_aux_older_saved;

  nl_sys.solution->close();
  nl.solutionOld().close();
  nl.solutionOlder().close();
  aux_sys.solution->close();
  aux.solutionOld().close();
  aux.solutionOlder().close();
}

bool
DT2::converged() const
{
  if (!_converged)
    return false;

  if (_error < _e_max)
    return true;
  else
    return false;
}
