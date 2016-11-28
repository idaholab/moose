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

#include "DT2Stepper.h"
#include "FEProblem.h"
#include "Transient.h"
#include "MooseApp.h"

template<>
InputParameters validParams<DT2Stepper>()
{
  InputParameters params = validParams<Stepper>();

  params.addRequiredParam<Real>("dt", "The initial time step size.");
  params.addRequiredParam<Real>("e_tol", "Target error tolerance.");
  params.addRequiredParam<Real>("e_max", "Maximum acceptable error.");
  params.addParam<Real>("max_increase", 1.0e9, "Maximum ratio that the time step can increase.");

  return params;
}

DT2Stepper::DT2Stepper(const InputParameters & parameters) :
    Stepper(parameters),
    _input_dt(getParam<Real>("dt")),
    _e_tol(getParam<Real>("e_tol")),
    _e_max(getParam<Real>("e_max")),
    _max_increase(getParam<Real>("max_increase")),
    _tol(_executioner.timestepTol())
{
}

Real
DT2Stepper::computeInitialDT()
{
  // On the first step... this means that we are getting ready to start the first small step
  if (std::abs(_time - _start_time) < _tol && _big_soln && _converged[0])
    return computeDT();

  // Otherwise this really is the first timestep so set things up...
  backup();
  return resetWindow(_time, _input_dt);
}

Real
DT2Stepper::computeDT()
{
  if (MooseUtils::absoluteFuzzyEqual(_time, _end_time, _tol) && !_big_soln && _converged[0]) // Just finished initial big step
  {
    // collect big dt soln and rewind to collect small dt solns
    _big_soln.reset(_soln_nonlin->clone().release());
    _big_soln->close();

    restore(_start_time);

    return windowDT() / 2.0; // doesn't actually matter what we return here because rewind
  }
  else if (MooseUtils::absoluteFuzzyEqual(_time, _start_time, _tol) && _big_soln && _converged[0]) // Start of first small step
  {
    // we just rewound and need to do small steps
    return windowDT() / 2.0;
  }
  else if (MooseUtils::absoluteFuzzyEqual(_time, _start_time + windowDT() / 2., _tol) && _big_soln && _converged[0]) // Start of second small step
  {
    // we just finished the first of the smaller dt steps
    return windowDT() / 2.0;
  }
  else if (MooseUtils::absoluteFuzzyEqual(_time, _end_time, _tol) && _big_soln && _converged[0]) // Finished second small step
  {
    // we just finished the second of the two smaller dt steps and are ready for error calc
    Real err = calculateError();
    if (err > _e_max)
    {
      restore(_start_time);

      return resetWindow(_start_time, windowDT() / 2.0);
    }

    Real new_dt = windowDT() * std::pow(_e_tol / err, 1.0 / _order);

    backup();

    return resetWindow(_time, new_dt);
  }
  else
  {
    // something went wrong or this is initial call of simulation - start over
    backup();

    Real ddt = windowDT();

    if (ddt == 0)
      ddt = _dt[0];

    return resetWindow(_time, ddt);
  }
}

Real
DT2Stepper::computeFailedDT()
{
  // This isn't complete
  return _dt[0]*0.5;
}

Real
DT2Stepper::resetWindow(Real start, Real dt)
{
  _start_time = start;
  _end_time = _start_time + dt;
  _big_soln.reset(nullptr);
  return dt;
}

Real
DT2Stepper::windowDT()
{
  return _end_time - _start_time;
}

Real
DT2Stepper::calculateError()
{
  auto small_soln = _soln_nonlin->clone();

  auto diff = _soln_nonlin->clone();

  *diff -= *_big_soln;
  Real err = (diff->l2_norm() / std::max(_big_soln->l2_norm(), small_soln->l2_norm())) / _dt[0];
  return err;
}
