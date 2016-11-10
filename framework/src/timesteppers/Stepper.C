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

#include "Stepper.h"

InstrumentedBlock::InstrumentedBlock(Real * dt_store)
  : _stepper(nullptr), _dt_store(dt_store), _own(!dt_store)
{
  if (!_dt_store)
    _dt_store = new Real(0);
}

InstrumentedBlock::~InstrumentedBlock()
{
  if (_own)
    delete _dt_store;
}

Real
InstrumentedBlock::next(const StepperInfo & si, StepperFeedback & sf)
{
  if (!_stepper)
    mooseError("InstrumentedStepper's inner stepper not set");

  *_dt_store = _stepper->next(si, sf);
  return *_dt_store;
}

void
InstrumentedBlock::setStepper(StepperBlock * s)
{
  _stepper.reset(s);
}

Real *
InstrumentedBlock::dtPtr()
{
  return _dt_store;
}

RetryUnusedBlock::RetryUnusedBlock(StepperBlock * s, Real tol, bool prev_prev)
  : _stepper(s), _tol(tol), _prev_prev(prev_prev), _prev_dt(0), _prev_time(0)
{
}

Real
RetryUnusedBlock::next(const StepperInfo & si, StepperFeedback & sf)
{
  if (_prev_dt != 0 && std::abs(si.prev_dt - _prev_dt) > _tol && std::abs(si.time - _prev_time) > _tol)
  {
    if (_prev_prev)
    {
      _prev_dt = si.prev_prev_dt;
      return si.prev_prev_dt;
    }
    else
      return _prev_dt;
  }
  _prev_time = si.time;
  _prev_dt = _stepper->next(si, sf);
  return _prev_dt;
}

ConstrFuncBlock::ConstrFuncBlock(StepperBlock * s, std::function<Real(Real)> func,
                                 Real max_diff)
  : _stepper(s), _func(func), _max_diff(max_diff)
{
}

Real
ConstrFuncBlock::next(const StepperInfo & si, StepperFeedback & sf)
{
  Real dt = _stepper->next(si, sf);
  Real f_curr = _func(si.time);
  Real df = std::abs(_func(si.time + dt) - f_curr);
  while (_max_diff > 0 && df > _max_diff)
  {
    dt /= 2.0;
    df = std::abs(_func(si.time + dt) - f_curr);
  }
  return dt;
}

PiecewiseBlock::PiecewiseBlock(std::vector<Real> times, std::vector<Real> dts,
                               bool interpolate)
  : _times(times), _dts(dts), _interp(interpolate), _lin(times, dts)
{
}

Real
PiecewiseBlock::next(const StepperInfo & si, StepperFeedback &)
{
  if (_interp)
    return _lin.sample(si.time);

  if (MooseUtils::relativeFuzzyGreaterEqual(si.time, _times.back()))
    return _times.back();

  for (int i = 0; i < _times.size() - 1; i++)
    if (MooseUtils::relativeFuzzyLessThan(si.time, _times[i + 1]))
      return _dts[i];
  return _dts.back();
}

MinOfBlock::MinOfBlock(StepperBlock * a, StepperBlock * b, Real tol)
  : _a(a), _b(b), _tol(tol)
{
}

Real
MinOfBlock::next(const StepperInfo & si, StepperFeedback & sf)
{
  Real dta = _a->next(si, sf);
  Real dtb = _b->next(si, sf);
  if (dta - _tol < dtb)
    return dta;
  return dtb;
}

AdaptiveBlock::AdaptiveBlock(unsigned int optimal_iters, unsigned int iter_window,
                             Real lin_iter_ratio, Real shrink_factor, Real growth_factor)
  : _optimal_iters(optimal_iters),
    _iter_window(iter_window),
    _lin_iter_ratio(lin_iter_ratio),
    _shrink_factor(shrink_factor),
    _growth_factor(growth_factor)
{
}

Real
AdaptiveBlock::next(const StepperInfo & si, StepperFeedback &)
{
  bool can_shrink = true;
  bool can_grow = si.converged && si.prev_converged;

  unsigned int growth_nl_its = 0;
  unsigned int growth_l_its = 0;
  unsigned int shrink_nl_its = _optimal_iters + _iter_window;
  unsigned int shrink_l_its = _lin_iter_ratio * (_optimal_iters + _iter_window);
  if (_optimal_iters > _iter_window)
  {
    growth_nl_its = _optimal_iters - _iter_window;
    growth_l_its = _lin_iter_ratio * (_optimal_iters - _iter_window);
  }

  if (can_grow && (si.nonlin_iters < growth_nl_its && si.lin_iters < growth_l_its))
    return si.prev_dt * _growth_factor;
  else if (can_shrink &&
           (!si.converged || si.nonlin_iters > shrink_nl_its || si.lin_iters > shrink_l_its))
    return si.prev_dt * _shrink_factor;
  else
    return si.prev_dt;
};

SolveTimeAdaptiveBlock::SolveTimeAdaptiveBlock(int initial_direc, Real percent_change)
  : _percent_change(percent_change), _direc(initial_direc), _n_steps(0)
{
}

Real
SolveTimeAdaptiveBlock::next(const StepperInfo & si, StepperFeedback &)
{
  Real ratio = si.prev_solve_time_secs / si.prev_dt;
  Real prev_ratio = si.prev_prev_solve_time_secs / si.prev_prev_dt;
  Real prev_prev_ratio = si.prev_prev_prev_solve_time_secs / si.prev_prev_prev_dt;

  _n_steps++;
  // this is this way in order to mirror original SolutionTimeAdaptiveDT
  // stepper behavior.  However, it might be better to compare prev_ratio to
  // prev_prev ratio instead.
  if (ratio > prev_ratio && ratio > prev_prev_ratio && _n_steps > 1)
  {
    _direc *= -1;
    _n_steps = 0;
  }

  return si.prev_dt + si.prev_dt * _percent_change * _direc;
}

PredictorCorrectorBlock::PredictorCorrectorBlock(int start_adapting, Real e_tol,
                                                 Real scaling_param,
                                                 std::string time_integrator)
  : _start_adapting(start_adapting),
    _e_tol(e_tol),
    _scale_param(scaling_param),
    _time_integrator(time_integrator)
{
}

Real
PredictorCorrectorBlock::next(const StepperInfo & si, StepperFeedback &)
{
  if (!si.converged)
    return si.prev_dt;
  // original Predictor stepper actually used the Real valued time step
  // instead of step_count which is used here - which doesn't make sense for
  // this check since if t_0 != 0 or dt != 1 would make it so the predictor
  // might not have a prediction by the time _start_adapting was called -
  // this could be changed back like the original, but then, start_adapting
  // needs to be carefully documented.
  if (si.step_count < _start_adapting)
    return si.prev_dt;
  if (si.soln_nonlin == nullptr || si.soln_predicted == nullptr)
    mooseError("no predicted solution available");

  Real error = estimateTimeError(si);
  Real infnorm = si.soln_nonlin->linfty_norm();
  Real e_max = 1.1 * _e_tol * infnorm;

  if (error > e_max)
    return si.prev_dt * 0.5;
  return si.prev_dt * _scale_param * std::pow(infnorm * _e_tol / error, 1.0 / 3.0);
}

Real
PredictorCorrectorBlock::estimateTimeError(const StepperInfo & si)
{
  NumericVector<Number> & soln = *si.soln_nonlin;
  NumericVector<Number> & predicted = *si.soln_predicted;
  soln.close();
  predicted.close();

  Real dtprev = si.prev_prev_dt;
  Real dt = si.prev_dt;
  if (_time_integrator == "CrankNicolson")
  {
    predicted -= soln;
    predicted *= (dt) / (3.0 * (dt + dtprev));
    return predicted.l2_norm();
  }
  else if (_time_integrator == "BDF2")
  {
    predicted *= -1.0;
    predicted += soln;
    Real topcalc = 2.0 * (dt + dtprev) * (dt + dtprev);
    Real bottomcalc = 6.0 * dt * dt + 12.0 * dt * dtprev + 5.0 * dtprev * dtprev;
    predicted *= topcalc / bottomcalc;
    return predicted.l2_norm();
  }

  mooseError("unsupported time integration scheme '" + _time_integrator + "'");
}

DT2Block::DT2Block(Real time_tol, Real e_tol, Real e_max, int integrator_order)
  : _tol(time_tol),
    _e_tol(e_tol),
    _e_max(e_max),
    _order(integrator_order),
    _start_time(-1),
    _end_time(-1),
    _big_soln(nullptr)
{
}

Real
DT2Block::resetWindow(Real start, Real dt)
{
  _start_time = start;
  _end_time = _start_time + dt;
  _big_soln.reset(nullptr);
  return dt;
}

Real
DT2Block::dt()
{
  return _end_time - _start_time;
}

Real
DT2Block::next(const StepperInfo & si, StepperFeedback & sf)
{
  if (std::abs(si.time - _end_time) < _tol && _big_soln && si.converged)
  {
    // we just finished the second of the two smaller dt steps and are ready for error calc
    Real err = calcErr(si);
    if (err > _e_max)
    {
      sf.rewind = true;
      sf.rewind_time = _start_time;
      return resetWindow(sf.rewind_time, dt() / 2);
    }

    Real new_dt = dt() * std::pow(_e_tol / err, 1.0 / _order);
    sf.snapshot = true;
    return resetWindow(si.time, new_dt);
  }
  else if (std::abs(si.time - _end_time) < _tol && !_big_soln && si.converged)
  {
    // collect big dt soln and rewind to collect small dt solns
    _big_soln.reset(si.soln_nonlin->clone().release());
    _big_soln->close();
    sf.rewind = true;
    sf.rewind_time = _start_time;
    return dt() / 2.0; // doesn't actually matter what we return here because rewind
  }
  else if (std::abs(si.time - _start_time) < _tol && _big_soln && si.converged)
  {
    // we just rewound and need to do small steps
    return dt() / 2;
  }
  else if (std::abs(_start_time + dt() / 2 - si.time) < _tol && _big_soln && si.converged)
  {
    // we just finished the first of the smaller dt steps
    return dt() / 2;
  }
  else
  {
    // something went wrong or this is initial call of simulation - start over
    sf.snapshot = true;
    Real ddt = dt();
    if (ddt == 0)
      ddt = si.prev_dt;
    return resetWindow(si.time, ddt);
  }
}

Real
DT2Block::calcErr(const StepperInfo & si)
{
  std::unique_ptr<NumericVector<Number>> small_soln(si.soln_nonlin->clone().release());
  std::unique_ptr<NumericVector<Number>> diff(si.soln_nonlin->clone().release());
  small_soln->close();
  diff->close();
  *diff -= *_big_soln;
  Real err = (diff->l2_norm() / std::max(_big_soln->l2_norm(), small_soln->l2_norm())) / dt();
  return err;
}

IfBlock::IfBlock(StepperBlock * on_true, StepperBlock * on_false,
                 std::function<bool(const StepperInfo &)> func)
  : _ontrue(on_true), _onfalse(on_false), _func(func)
{
}

Real
IfBlock::next(const StepperInfo & si, StepperFeedback & sf)
{
  bool val = _func(si);
  if (val)
    return _ontrue->next(si, sf);
  return _onfalse->next(si, sf);
}

ModBlock::ModBlock(StepperBlock * s,
                   std::function<
                       Real(const StepperInfo & si, Real dt)> func)
  : _stepper(s), _func(func)
{
}

Real
ModBlock::next(const StepperInfo & si, StepperFeedback & sf)
{
  return _func(si, _stepper->next(si, sf));
}

RootBlock::RootBlock(std::function<Real(const StepperInfo & si)> func)
  : _func(func)
{
}

Real
RootBlock::next(const StepperInfo & si, StepperFeedback &)
{
  return _func(si);
}

StepperBlock *
BaseStepper::constant(Real dt)
{
  return new RootBlock([=](const StepperInfo & si)
                       {
                         return dt;
                       });
}

StepperBlock *
BaseStepper::prevdt()
{
  return new RootBlock([=](const StepperInfo & si)
                       {
                         return si.prev_dt;
                       });
}

StepperBlock *
BaseStepper::fixedTimes(std::vector<Real> times, Real tol)
{
  return new RootBlock([=](const StepperInfo & si)
                       {
                         if (times.size() == 0)
                           return std::numeric_limits<Real>::infinity();

                         for (int i = 0; i < times.size(); i++)
                         {
                           Real t0 = times[i];
                           if (si.time < t0 - tol)
                             return t0 - si.time;
                         }
                         return std::numeric_limits<Real>::infinity();
                       });
}

StepperBlock *
BaseStepper::ptr(const Real * dt_store)
{
  return new RootBlock([=](const StepperInfo & si)
                       {
                         return *dt_store;
                       });
}

StepperBlock *
BaseStepper::dtLimit(StepperBlock * s, Real dt_min, Real dt_max)
{
  return new ModBlock(s, [=](const StepperInfo & si, Real dt)
                      {
                        if (dt < dt_min)
                          return dt_min;
                        else if (dt > dt_max)
                          return dt_max;
                        return dt;
                      });
}

StepperBlock *
BaseStepper::bounds(StepperBlock * s, Real t_min, Real t_max)
{
  return new ModBlock(s, [=](const StepperInfo & si, Real dt)
                      {
                        Real t = si.time + dt;
                        if (t < t_min)
                          return t_min - si.time;
                        else if (t > t_max)
                          return t_max - si.time;
                        return dt;
                      });
}

StepperBlock *
BaseStepper::maxRatio(StepperBlock * s, Real max_ratio)
{
  return new ModBlock(s, [=](const StepperInfo & si, Real dt)
                      {
                        if (si.prev_dt > 0 && dt / si.prev_dt > max_ratio)
                          dt = si.prev_dt * max_ratio;
                        return dt;
                      });
}

StepperBlock *
BaseStepper::mult(Real mult, StepperBlock * s)
{
  if (!s)
    s = prevdt();
  return new ModBlock(
      s, [=](const StepperInfo & si, Real dt)
      {
        return dt * mult;
      });
}

StepperBlock *
BaseStepper::between(StepperBlock * on, StepperBlock * between, std::vector<Real> times,
                     Real tol)
{
  return new IfBlock(on, between, [=](const StepperInfo & si)
                     {
                       for (int i = 0; i < times.size(); i++)
                       {
                         if (std::abs(si.time - times[i]) < tol)
                           return true;
                       }
                       return false;
                     });
}

StepperBlock *
BaseStepper::everyN(StepperBlock * nth, int every_n, int offset, StepperBlock * between)
{
  if (!between)
    between = BaseStepper::prevdt();
  return new IfBlock(nth, between, [=](const StepperInfo & si)
                     {
                       return (si.step_count + (every_n - offset) - 1) % every_n == 0;
                     });
}

StepperBlock *
BaseStepper::initialN(StepperBlock * initial, StepperBlock * primary, int n)
{
  return new IfBlock(initial, primary, [=](const StepperInfo & si)
                     {
                       return si.step_count <= n;
                     });
}

StepperBlock *
BaseStepper::converged(StepperBlock * converged, StepperBlock * not_converged, bool delay)
{
  return new IfBlock(converged, not_converged, [=](const StepperInfo & si)
                     {
                       return si.converged && (!delay || si.prev_converged);
                     });
}

StepperBlock *
BaseStepper::min(StepperBlock * a, StepperBlock * b, Real tol)
{
  return new MinOfBlock(a, b, tol);
}
