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

StepperInfo::StepperInfo()
  : _step_count(1),
    _time(0),
    _dt(),
    _nonlin_iters(0),
    _lin_iters(0),
    _converged(),
    _solve_time_secs(),
    _soln_nonlin(nullptr),
    _soln_aux(nullptr),
    _soln_predicted(nullptr),
    _snapshot(false),
    _rewind(false),
    _rewind_time(-1),
    _dummy_comm()
{
  for (int i = 0; i < 3; i++)
  {
    _dt.push_front(0);
    _converged.push_front(true);
    _solve_time_secs.push_front(0);
  }
  _soln_nonlin.reset(NumericVector<Number>::build(_dummy_comm).release());
  _soln_nonlin->init(0, 0, false, SERIAL);
  _soln_aux.reset(NumericVector<Number>::build(_dummy_comm).release());
  _soln_aux->init(0, 0, false, SERIAL);
  _soln_predicted.reset(NumericVector<Number>::build(_dummy_comm).release());
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

  _snapshot = false;
  _rewind = false;
  _rewind_time = -1;
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

  _snapshot = false;
  _rewind = false;
  _rewind_time = -1;
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

  _snapshot = false;
  _rewind = false;
  _rewind_time = -1;

  if (_soln_nonlin->size() != soln_nonlin.size())
  {
    _soln_nonlin.reset(NumericVector<Number>::build(_dummy_comm).release());
    _soln_nonlin->init(soln_nonlin.size(), soln_nonlin.size(), false, SERIAL);
  }
  if (_soln_aux->size() != soln_aux.size())
  {
    _soln_aux.reset(NumericVector<Number>::build(_dummy_comm).release());
    _soln_aux->init(soln_aux.size(), soln_aux.size(), false, SERIAL);
  }
  if (_soln_predicted->size() != soln_predicted.size())
  {
    _soln_predicted.reset(NumericVector<Number>::build(_dummy_comm).release());
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

template <typename T>
T nth_elem(std::list<T>& lst, int n)
{
  int i = 0;
  for (auto it = lst.begin(); it != lst.end(); ++it)
  {
    if (i == n)
      return *it;
    i++;
  }
  mooseError("too old history value requested from StepperInfo");
}

Real
StepperInfo::dt(int n)
{
  return nth_elem(_dt, n);
}

bool
StepperInfo::converged(int n)
{
  return nth_elem(_converged, n);
}

Real
StepperInfo::solveTimeSecs(int n)
{
  return nth_elem(_solve_time_secs, n);
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
StepperInfo::snapshot()
{
  _snapshot = true;
}

bool
StepperInfo::wantSnapshot()
{
  return _snapshot;
}

void
StepperInfo::rewind(Real target_time)
{
  _rewind = true;
  _rewind_time = target_time;
}

Real
StepperInfo::rewindTime()
{
  return _rewind_time;
}

StepperBlock *
BaseStepper::constant(Real dt)
{
  return new RootBlock([=](StepperInfo & si)
                       {
                         return dt;
                       });
}

StepperBlock *
BaseStepper::prevdt()
{
  return new RootBlock([=](StepperInfo & si)
                       {
                         return si.dt();
                       });
}

StepperBlock *
BaseStepper::fixedTimes(std::vector<Real> times, Real tol)
{
  return new RootBlock([=](StepperInfo & si)
                       {
                         if (times.size() == 0)
                           return std::numeric_limits<Real>::infinity();

                         for (int i = 0; i < times.size(); i++)
                         {
                           Real t0 = times[i];
                           if (si.time() < t0 - tol)
                             return t0 - si.time();
                         }
                         return std::numeric_limits<Real>::infinity();
                       });
}

StepperBlock *
BaseStepper::ptr(const Real * dt_store)
{
  return new RootBlock([=](StepperInfo & si)
                       {
                         return *dt_store;
                       });
}

StepperBlock *
BaseStepper::dtLimit(StepperBlock * s, Real dt_min, Real dt_max)
{
  return new ModBlock(s, [=](StepperInfo & si, Real dt)
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
  return new ModBlock(s, [=](StepperInfo & si, Real dt)
                      {
                        Real t = si.time() + dt;
                        if (t < t_min)
                          return t_min - si.time();
                        else if (t > t_max)
                          return t_max - si.time();
                        return dt;
                      });
}

StepperBlock *
BaseStepper::maxRatio(StepperBlock * s, Real max_ratio)
{
  return new ModBlock(s, [=](StepperInfo & si, Real dt)
                      {
                        if (si.dt() > 0 && dt / si.dt() > max_ratio)
                          dt = si.dt() * max_ratio;
                        return dt;
                      });
}

StepperBlock *
BaseStepper::mult(Real mult, StepperBlock * s)
{
  if (!s)
    s = prevdt();
  return new ModBlock(
      s, [=](StepperInfo & si, Real dt)
      {
        return dt * mult;
      });
}

StepperBlock *
BaseStepper::between(StepperBlock * on, StepperBlock * between, std::vector<Real> times,
                     Real tol)
{
  return new IfBlock(on, between, [=](StepperInfo & si)
                     {
                       for (int i = 0; i < times.size(); i++)
                       {
                         if (std::abs(si.time() - times[i]) < tol)
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
  return new IfBlock(nth, between, [=](StepperInfo & si)
                     {
                       return (si.stepCount() + (every_n - offset) - 1) % every_n == 0;
                     });
}

StepperBlock *
BaseStepper::initialN(StepperBlock * initial, StepperBlock * primary, int n)
{
  return new IfBlock(initial, primary, [=](StepperInfo & si)
                     {
                       return si.stepCount() <= n;
                     });
}

StepperBlock *
BaseStepper::converged(StepperBlock * converged, StepperBlock * not_converged, bool delay)
{
  return new IfBlock(converged, not_converged, [=](StepperInfo & si)
                     {
                       return si.converged() && (!delay || si.converged(1));
                     });
}

StepperBlock *
BaseStepper::min(StepperBlock * a, StepperBlock * b, Real tol)
{
  return new MinOfBlock(a, b, tol);
}

RootBlock::RootBlock(std::function<Real(StepperInfo & si)> func)
  : _func(func)
{
}

Real
RootBlock::next(StepperInfo & si)
{
  return _func(si);
}

ModBlock::ModBlock(StepperBlock * s,
                   std::function<
                       Real(StepperInfo & si, Real dt)> func)
  : _stepper(s), _func(func)
{
}

Real
ModBlock::next(StepperInfo & si)
{
  return _func(si, _stepper->next(si));
}

IfBlock::IfBlock(StepperBlock * on_true, StepperBlock * on_false,
                 std::function<bool(StepperInfo &)> func)
  : _ontrue(on_true), _onfalse(on_false), _func(func)
{
}

Real
IfBlock::next(StepperInfo & si)
{
  bool val = _func(si);
  if (val)
    return _ontrue->next(si);
  return _onfalse->next(si);
}


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
InstrumentedBlock::next(StepperInfo & si)
{
  if (!_stepper)
    mooseError("InstrumentedStepper's inner stepper not set");

  *_dt_store = _stepper->next(si);
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
RetryUnusedBlock::next(StepperInfo & si)
{
  if (_prev_dt != 0 && std::abs(si.dt() - _prev_dt) > _tol && std::abs(si.time() - _prev_time) > _tol)
  {
    if (_prev_prev)
    {
      _prev_dt = si.dt(1);
      return si.dt(1);
    }
    else
      return _prev_dt;
  }
  _prev_time = si.time();
  _prev_dt = _stepper->next(si);
  return _prev_dt;
}

ConstrFuncBlock::ConstrFuncBlock(StepperBlock * s, std::function<Real(Real)> func,
                                 Real max_diff)
  : _stepper(s), _func(func), _max_diff(max_diff)
{
}

Real
ConstrFuncBlock::next(StepperInfo & si)
{
  Real dt = _stepper->next(si);
  Real f_curr = _func(si.time());
  Real df = std::abs(_func(si.time() + dt) - f_curr);
  while (_max_diff > 0 && df > _max_diff)
  {
    dt /= 2.0;
    df = std::abs(_func(si.time() + dt) - f_curr);
  }
  return dt;
}

PiecewiseBlock::PiecewiseBlock(std::vector<Real> times, std::vector<Real> dts,
                               bool interpolate)
  : _times(times), _dts(dts), _interp(interpolate), _lin(times, dts)
{
}

Real
PiecewiseBlock::next(StepperInfo & si)
{
  if (_interp)
    return _lin.sample(si.time());

  if (MooseUtils::relativeFuzzyGreaterEqual(si.time(), _times.back()))
    return _times.back();

  for (int i = 0; i < _times.size() - 1; i++)
    if (MooseUtils::relativeFuzzyLessThan(si.time(), _times[i + 1]))
      return _dts[i];
  return _dts.back();
}

MinOfBlock::MinOfBlock(StepperBlock * a, StepperBlock * b, Real tol)
  : _a(a), _b(b), _tol(tol)
{
}

Real
MinOfBlock::next(StepperInfo & si)
{
  Real dta = _a->next(si);
  Real dtb = _b->next(si);
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
AdaptiveBlock::next(StepperInfo & si)
{
  bool can_shrink = true;
  bool can_grow = si.converged() && si.converged(1);

  unsigned int growth_nl_its = 0;
  unsigned int growth_l_its = 0;
  unsigned int shrink_nl_its = _optimal_iters + _iter_window;
  unsigned int shrink_l_its = _lin_iter_ratio * (_optimal_iters + _iter_window);
  if (_optimal_iters > _iter_window)
  {
    growth_nl_its = _optimal_iters - _iter_window;
    growth_l_its = _lin_iter_ratio * (_optimal_iters - _iter_window);
  }

  if (can_grow && (si.nonlinIters() < growth_nl_its && si.linIters() < growth_l_its))
    return si.dt() * _growth_factor;
  else if (can_shrink &&
           (!si.converged() || si.nonlinIters() > shrink_nl_its || si.linIters() > shrink_l_its))
    return si.dt() * _shrink_factor;
  else
    return si.dt();
};

SolveTimeAdaptiveBlock::SolveTimeAdaptiveBlock(int initial_direc, Real percent_change)
  : _percent_change(percent_change), _direc(initial_direc), _n_steps(0)
{
}

Real
SolveTimeAdaptiveBlock::next(StepperInfo & si)
{
  Real ratio = si.solveTimeSecs() / si.dt();
  Real prev_ratio = si.solveTimeSecs(1) / si.dt(1);
  Real prev_prev_ratio = si.solveTimeSecs(2) / si.dt(2);

  _n_steps++;
  // this is this way in order to mirror original SolutionTimeAdaptiveDT
  // stepper behavior.  However, it might be better to compare prev_ratio to
  // prev_prev ratio instead.
  if (ratio > prev_ratio && ratio > prev_prev_ratio && _n_steps > 1)
  {
    _direc *= -1;
    _n_steps = 0;
  }

  return si.dt() + si.dt() * _percent_change * _direc;
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
PredictorCorrectorBlock::next(StepperInfo & si)
{
  if (!si.converged())
    return si.dt();
  // original Predictor stepper actually used the Real valued time step
  // instead of step_count which is used here - which doesn't make sense for
  // this check since if t_0 != 0 or dt != 1 would make it so the predictor
  // might not have a prediction by the time _start_adapting was called -
  // this could be changed back like the original, but then, start_adapting
  // needs to be carefully documented.
  if (si.stepCount() < _start_adapting)
    return si.dt();
  if (si.solnNonlin() == nullptr || si.solnPredicted() == nullptr)
    mooseError("no predicted solution available");

  Real error = estimateTimeError(si);
  Real infnorm = si.solnNonlin()->linfty_norm();
  Real e_max = 1.1 * _e_tol * infnorm;

  if (error > e_max)
    return si.dt() * 0.5;
  return si.dt() * _scale_param * std::pow(infnorm * _e_tol / error, 1.0 / 3.0);
}

Real
PredictorCorrectorBlock::estimateTimeError(StepperInfo & si)
{
  NumericVector<Number> & soln = *si.solnNonlin();
  NumericVector<Number> & predicted = *si.solnPredicted();

  Real dtprev = si.dt(1);
  Real dt = si.dt();
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
DT2Block::next(StepperInfo & si)
{
  if (std::abs(si.time() - _end_time) < _tol && _big_soln && si.converged())
  {
    // we just finished the second of the two smaller dt steps and are ready for error calc
    Real err = calcErr(si);
    if (err > _e_max)
    {
      si.rewind(_start_time);
      return resetWindow(si.rewindTime(), dt() / 2);
    }

    Real new_dt = dt() * std::pow(_e_tol / err, 1.0 / _order);
    si.snapshot();
    return resetWindow(si.time(), new_dt);
  }
  else if (std::abs(si.time() - _end_time) < _tol && !_big_soln && si.converged())
  {
    // collect big dt soln and rewind to collect small dt solns
    _big_soln.reset(si.solnNonlin()->clone().release());
    _big_soln->close();
    si.rewind(_start_time);
    return dt() / 2.0; // doesn't actually matter what we return here because rewind
  }
  else if (std::abs(si.time() - _start_time) < _tol && _big_soln && si.converged())
  {
    // we just rewound and need to do small steps
    return dt() / 2;
  }
  else if (std::abs(_start_time + dt() / 2 - si.time()) < _tol && _big_soln && si.converged())
  {
    // we just finished the first of the smaller dt steps
    return dt() / 2;
  }
  else
  {
    // something went wrong or this is initial call of simulation - start over
    si.snapshot();
    Real ddt = dt();
    if (ddt == 0)
      ddt = si.dt();
    return resetWindow(si.time(), ddt);
  }
}

Real
DT2Block::calcErr(StepperInfo & si)
{
  std::unique_ptr<NumericVector<Number>> small_soln(si.solnNonlin()->clone().release());
  std::unique_ptr<NumericVector<Number>> diff(si.solnNonlin()->clone().release());
  *diff -= *_big_soln;
  Real err = (diff->l2_norm() / std::max(_big_soln->l2_norm(), small_soln->l2_norm())) / dt();
  return err;
}

