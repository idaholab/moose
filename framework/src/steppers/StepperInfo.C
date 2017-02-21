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

#include "StepperInfo.h"

StepperInfo::StepperInfo()
  : _step_count(1),
    _time(0),
    _nonlin_iters(0),
    _lin_iters(0),
    _converged(),
    _solve_time_secs(),
    _soln_nonlin(nullptr),
    _soln_aux(nullptr),
    _soln_predicted(nullptr),
    _backup(false),
    _restore(false),
    _restore_time(-1),
    _dummy_comm()
{
  const unsigned int max_history = 3;
  for (unsigned int i = 0; i < max_history; i++)
  {
    _dt.push_front(0);
    _converged.push_front(true);
    _solve_time_secs.push_front(0);
  }
  _soln_nonlin = std::move(NumericVector<Number>::build(_dummy_comm));
  _soln_nonlin->init(0, 0, false, SERIAL);
  _soln_aux = std::move(NumericVector<Number>::build(_dummy_comm));
  _soln_aux->init(0, 0, false, SERIAL);
  _soln_predicted = std::move(NumericVector<Number>::build(_dummy_comm));
  _soln_predicted->init(0, 0, false, SERIAL);
}

StepperInfo::StepperInfo(const StepperInfo& si)
{
  _step_count = si._step_count;
  _time = si._time;
  _dt = si._dt;
  _nonlin_iters = si._nonlin_iters;
  _lin_iters = si._lin_iters;
  _converged = si._converged;
  _solve_time_secs = si._solve_time_secs;

  _soln_nonlin.reset(si._soln_nonlin->clone().release());
  _soln_aux.reset(si._soln_aux->clone().release());
  _soln_predicted.reset(si._soln_predicted->clone().release());

  _backup = false;
  _restore = false;
  _restore_time = -1;
}

StepperInfo&
StepperInfo::operator=(const StepperInfo& si)
{
  _step_count = si._step_count;
  _time = si._time;
  _dt = si._dt;
  _nonlin_iters = si._nonlin_iters;
  _lin_iters = si._lin_iters;
  _converged = si._converged;
  _solve_time_secs = si._solve_time_secs;

  _soln_nonlin.reset(si._soln_nonlin->clone().release());
  _soln_aux.reset(si._soln_aux->clone().release());
  _soln_predicted.reset(si._soln_predicted->clone().release());

  _backup = false;
  _restore = false;
  _restore_time = -1;
  return *this;
}

void
StepperInfo::pushHistory(Real dt, bool converged, Real solve_time)
{
  _dt.push_front(dt);
  _dt.pop_back();
  _converged.push_front(converged);
  _converged.pop_back();
  _solve_time_secs.push_front(solve_time);
  _solve_time_secs.pop_back();
}

void
StepperInfo::update(
    int step_count,
    Real time,
    Real dt,
    unsigned int nonlin_iters,
    unsigned int lin_iters,
    bool converged,
    Real solve_time_secs,
    std::vector<Real> soln_nonlin,
    std::vector<Real> soln_aux,
    std::vector<Real> soln_predicted
    )
{
  _step_count = step_count;
  _time = time;
  _dt.push_front(dt);
  _dt.pop_back();
  _nonlin_iters = nonlin_iters;
  _lin_iters = lin_iters;
  _converged.push_front(converged);
  _converged.pop_back();
  _solve_time_secs.push_front(solve_time_secs);
  _solve_time_secs.pop_back();

  _backup = false;
  _restore = false;
  _restore_time = -1;

  if (_soln_nonlin->size() != soln_nonlin.size())
  {
    _soln_nonlin = std::move(NumericVector<Number>::build(_dummy_comm));
    _soln_nonlin->init(soln_nonlin.size(), soln_nonlin.size(), false, SERIAL);
  }
  if (_soln_aux->size() != soln_aux.size())
  {
    _soln_aux = std::move(NumericVector<Number>::build(_dummy_comm));
    _soln_aux->init(soln_aux.size(), soln_aux.size(), false, SERIAL);
  }
  if (_soln_predicted->size() != soln_predicted.size())
  {
    _soln_predicted = std::move(NumericVector<Number>::build(_dummy_comm));
    _soln_predicted->init(soln_predicted.size(), soln_predicted.size(), false, SERIAL);
  }

  *_soln_nonlin = soln_nonlin;
  *_soln_aux = soln_aux;
  *_soln_predicted = soln_predicted;
}

int
StepperInfo::stepCount()
{
  return _step_count;
}

Real
StepperInfo::time()
{
  return _time;
}

Real
StepperInfo::dt(int n)
{
  return _dt[n];
}

bool
StepperInfo::converged(int n)
{
  return _converged[n];
}

Real
StepperInfo::solveTimeSecs(int n)
{
  return _solve_time_secs[n];
}

int
StepperInfo::nonlinIters()
{
  return _nonlin_iters;
}

int
StepperInfo::linIters()
{
  return _lin_iters;
}

NumericVector<Number>*
StepperInfo::solnNonlin()
{
  return _soln_nonlin.get();
}

NumericVector<Number>*
StepperInfo::solnAux()
{
  return _soln_aux.get();
}

NumericVector<Number>*
StepperInfo::solnPredicted()
{
  return _soln_predicted.get();
}

void
StepperInfo::backup()
{
  _backup = true;
}

bool
StepperInfo::wantBackup()
{
  return _backup;
}

void
StepperInfo::restore(Real target_time)
{
  _restore = true;
  _restore_time = target_time;
}

Real
StepperInfo::restoreTime()
{
  return _restore_time;
}
