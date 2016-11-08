
#include "Stepper.h"

class Logger
{
public:
  Logger(std::string loc) : _loc(loc), _dt(-1)
  {
    if (on)
    {
      DBG << "[LOG] " << std::string(level * 4, ' ') << "- entering " << _loc << "\n";
      level++;
    }
  }
  ~Logger()
  {
    if (on)
    {
      level--;
      DBG << "[LOG] " << std::string(level * 4, ' ') << "- leaving " << _loc << " (dt=" << _dt
          << ")\n";
    }
  }
  double val(double dt)
  {
    _dt = dt;
    return dt;
  }
  static int level;
  static bool on;
  std::string _loc;
  double _dt;
};

int Logger::level = 0;
bool Logger::on = true;

InstrumentedBlock::InstrumentedBlock(double * dt_store)
    : _stepper(nullptr), _dt_store(dt_store), _own(!dt_store)
{
  if (!_dt_store)
    _dt_store = new double(0);
}

InstrumentedBlock::~InstrumentedBlock()
{
  if (_own)
    delete _dt_store;
}

double
InstrumentedBlock::advance(const StepperInfo * si, StepperFeedback * sf)
{
  Logger l("Instrumented");
  if (!_stepper)
    throw "InstrumentedStepper's inner stepper not set";

  *_dt_store = _stepper->advance(si, sf);
  return l.val(*_dt_store);
}

void
InstrumentedBlock::setStepper(StepperBlock * s)
{
  _stepper.reset(s);
}

double *
InstrumentedBlock::dtPtr()
{
  return _dt_store;
}

RetryUnusedBlock::RetryUnusedBlock(StepperBlock * s, double tol, bool prev_prev)
    : _stepper(s), _tol(tol), _prev_prev(prev_prev), _prev_dt(0), _prev_time(0)
{
}

double
RetryUnusedBlock::advance(const StepperInfo * si, StepperFeedback * sf)
{
  Logger l("RetryUnused");
  if (_prev_dt != 0 && std::abs(si->prev_dt - _prev_dt) > _tol && std::abs(si->time - _prev_time) > _tol)
  {
    if (_prev_prev)
    {
      _prev_dt = si->prev_prev_dt;
      return l.val(si->prev_prev_dt);
    }
    else
      return l.val(_prev_dt);
  }
  _prev_time = si->time;
  _prev_dt = _stepper->advance(si, sf);
  return l.val(_prev_dt);
}

ConstrFuncBlock::ConstrFuncBlock(StepperBlock * s, std::function<double(double)> func,
                                     double max_diff)
    : _stepper(s), _func(func), _max_diff(max_diff)
{
}

double
ConstrFuncBlock::advance(const StepperInfo * si, StepperFeedback * sf)
{
  Logger l("ConstrFunc");
  double dt = _stepper->advance(si, sf);
  double f_curr = _func(si->time);
  double df = std::abs(_func(si->time + dt) - f_curr);
  while (_max_diff > 0 && df > _max_diff)
  {
    dt /= 2.0;
    df = std::abs(_func(si->time + dt) - f_curr);
  }
  return l.val(dt);
}

PiecewiseBlock::PiecewiseBlock(std::vector<double> times, std::vector<double> dts,
                                   bool interpolate)
    : _times(times), _dts(dts), _interp(interpolate), _lin(times, dts)
{
}

double
PiecewiseBlock::advance(const StepperInfo * si, StepperFeedback *)
{
  Logger l("Piecewise");
  if (_interp)
    return l.val(_lin.sample(si->time));

  if (MooseUtils::relativeFuzzyGreaterEqual(si->time, _times.back()))
    return l.val(_times.back());

  for (int i = 0; i < _times.size() - 1; i++)
    if (MooseUtils::relativeFuzzyLessThan(si->time, _times[i + 1]))
      return l.val(_dts[i]);
  return l.val(_dts.back());
}

MinOfBlock::MinOfBlock(StepperBlock * a, StepperBlock * b, double tol) : _a(a), _b(b), _tol(tol)
{
}

double
MinOfBlock::advance(const StepperInfo * si, StepperFeedback * sf)
{
  Logger l("MinOf");
  double dta = _a->advance(si, sf);
  double dtb = _b->advance(si, sf);
  if (dta - _tol < dtb)
    return l.val(dta);
  return l.val(dtb);
}

AdaptiveBlock::AdaptiveBlock(unsigned int optimal_iters, unsigned int iter_window,
                                 double lin_iter_ratio, double shrink_factor, double growth_factor)
    : _optimal_iters(optimal_iters),
      _iter_window(iter_window),
      _lin_iter_ratio(lin_iter_ratio),
      _shrink_factor(shrink_factor),
      _growth_factor(growth_factor)
{
}

double
AdaptiveBlock::advance(const StepperInfo * si, StepperFeedback *)
{
  Logger l("Adaptive");
  bool can_shrink = true;
  bool can_grow = si->converged && si->prev_converged;

  unsigned int growth_nl_its = 0;
  unsigned int growth_l_its = 0;
  unsigned int shrink_nl_its = _optimal_iters + _iter_window;
  unsigned int shrink_l_its = _lin_iter_ratio * (_optimal_iters + _iter_window);
  if (_optimal_iters > _iter_window)
  {
    growth_nl_its = _optimal_iters - _iter_window;
    growth_l_its = _lin_iter_ratio * (_optimal_iters - _iter_window);
  }

  if (can_grow && (si->nonlin_iters < growth_nl_its && si->lin_iters < growth_l_its))
    return l.val(si->prev_dt * _growth_factor);
  else if (can_shrink &&
           (!si->converged || si->nonlin_iters > shrink_nl_its || si->lin_iters > shrink_l_its))
    return l.val(si->prev_dt * _shrink_factor);
  else
    return l.val(si->prev_dt);
};

SolveTimeAdaptiveBlock::SolveTimeAdaptiveBlock(int initial_direc, double percent_change)
    : _percent_change(percent_change), _direc(initial_direc), _n_steps(0)
{
}

double
SolveTimeAdaptiveBlock::advance(const StepperInfo * si, StepperFeedback *)
{
  Logger l("SolveTimeAdaptive");
  double ratio = si->prev_solve_time_secs / si->prev_dt;
  double prev_ratio = si->prev_prev_solve_time_secs / si->prev_prev_dt;
  double prev_prev_ratio = si->prev_prev_prev_solve_time_secs / si->prev_prev_prev_dt;

  _n_steps++;
  // this is this way in order to mirror original SolutionTimeAdaptiveDT
  // stepper behavior.  However, it might be better to compare prev_ratio to
  // prev_prev ratio instead.
  if (ratio > prev_ratio && ratio > prev_prev_ratio && _n_steps > 1)
  {
    _direc *= -1;
    _n_steps = 0;
  }

  return l.val(si->prev_dt + si->prev_dt * _percent_change * _direc);
}

PredictorCorrectorBlock::PredictorCorrectorBlock(int start_adapting, double e_tol,
                                                     double scaling_param,
                                                     std::string time_integrator)
    : _start_adapting(start_adapting),
      _e_tol(e_tol),
      _scale_param(scaling_param),
      _time_integrator(time_integrator)
{
}

double
PredictorCorrectorBlock::advance(const StepperInfo * si, StepperFeedback *)
{
  Logger l("PredictorCorrector");
  if (!si->converged)
    return l.val(si->prev_dt);
  // original Predictor stepper actually used the Real valued time step
  // instead of step_count which is used here - which doesn't make sense for
  // this check since if t_0 != 0 or dt != 1 would make it so the predictor
  // might not have a prediction by the time _start_adapting was called -
  // this could be changed back like the original, but then, start_adapting
  // needs to be carefully documented.
  if (si->step_count < _start_adapting)
    return l.val(si->prev_dt);
  if (si->soln_nonlin == nullptr || si->soln_predicted == nullptr)
    mooseError("no predicted solution available");

  double error = estimateTimeError(si);
  double infnorm = si->soln_nonlin->linfty_norm();
  double e_max = 1.1 * _e_tol * infnorm;

  if (error > e_max)
    return l.val(si->prev_dt * 0.5);
  return l.val(si->prev_dt * _scale_param * std::pow(infnorm * _e_tol / error, 1.0 / 3.0));
}

double
PredictorCorrectorBlock::estimateTimeError(const StepperInfo * si)
{
  NumericVector<Number> & soln = *si->soln_nonlin;
  NumericVector<Number> & predicted = *si->soln_predicted;
  soln.close();
  predicted.close();

  double dtprev = si->prev_prev_dt;
  double dt = si->prev_dt;
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
    double topcalc = 2.0 * (dt + dtprev) * (dt + dtprev);
    double bottomcalc = 6.0 * dt * dt + 12.0 * dt * dtprev + 5.0 * dtprev * dtprev;
    predicted *= topcalc / bottomcalc;
    return predicted.l2_norm();
  }

  mooseError("unsupported time integration scheme '" + _time_integrator + "'");
}

DT2Block::DT2Block(double time_tol, double e_tol, double e_max, int integrator_order)
    : _tol(time_tol),
      _e_tol(e_tol),
      _e_max(e_max),
      _order(integrator_order),
      _start_time(-1),
      _end_time(-1),
      _big_soln(nullptr)
{
}

double
DT2Block::resetWindow(double start, double dt)
{
  _start_time = start;
  _end_time = _start_time + dt;
  _big_soln.release();
  return dt;
}

double DT2Block::dt()
{
  return _end_time - _start_time;
}

double
DT2Block::advance(const StepperInfo * si, StepperFeedback * sf)
{
  Logger l("DT2");

  if (std::abs(si->time - _end_time) < _tol && _big_soln && si->converged)
  {
    //std::c out << "DT2:inner: set complete - calcing error\n";
    // we just finished the second of the two smaller dt steps and are ready for error calc
    double err = calcErr(si);
    if (err > _e_max)
    {
      //std::c out << "DT2:inner: error too large, rewinding and start over with dt/2\n";
      sf->rewind = true;
      sf->rewind_time = _start_time;
      return l.val(resetWindow(sf->rewind_time, dt() / 2));
    }

    double new_dt = dt() * std::pow(_e_tol / err, 1.0 / _order);
    sf->snapshot = true;
    return l.val(resetWindow(si->time, new_dt));
  }
  else if (std::abs(si->time - _end_time) < _tol && !_big_soln && si->converged)
  {
    //std::c out << "DT2:inner: got big soln, rewinding\n";
    // collect big dt soln and rewind to collect small dt solns
    _big_soln.reset(si->soln_nonlin->clone().release());
    _big_soln->close();
    sf->rewind = true;
    sf->rewind_time = _start_time;
    return l.val(dt() / 2.0); // doesn't actually matter what we return here because rewind
  }
  else if (std::abs(si->time - _start_time) < _tol && _big_soln && si->converged)
  {
    //std::c out << "DT2:inner: just rewound - starting small steps\n";
    // we just rewound and need to do small steps
    return l.val(dt() / 2);
  }
  else if (std::abs(_start_time + dt() / 2 - si->time) < _tol && _big_soln && si->converged)
  {
    //std::c out << "DT2:inner: finished 1st small step, doing second\n";
    // we just finished the first of the smaller dt steps
    return l.val(dt() / 2);
  }
  else
  {
    //std::c out << "DT2:inner: starting solve sequence over\n";
    // something went wrong or this is initial call of simulation - start over
    sf->snapshot = true;
    double ddt = dt();
    if (ddt == 0)
      ddt = si->prev_dt;
    return l.val(resetWindow(si->time, ddt));
  }
}

double
DT2Block::calcErr(const StepperInfo * si)
{
  std::unique_ptr<NumericVector<Number>> small_soln(si->soln_nonlin->clone().release());
  std::unique_ptr<NumericVector<Number>> diff(si->soln_nonlin->clone().release());
  small_soln->close();
  diff->close();
  *diff -= *_big_soln;
  double err = (diff->l2_norm() / std::max(_big_soln->l2_norm(), small_soln->l2_norm())) / dt();
  //std::c out << "DT2:inner: error=" << err << ", e_tol=" << _e_tol << "\n";
  return err;
}

IfBlock::IfBlock(StepperBlock * on_true, StepperBlock * on_false,
                     std::function<bool(const StepperInfo *)> func)
    : _ontrue(on_true), _onfalse(on_false), _func(func)
{
}

double
IfBlock::advance(const StepperInfo * si, StepperFeedback * sf)
{
  Logger l("OnFunc");
  bool val = _func(si);
  if (val)
    return l.val(_ontrue->advance(si, sf));
  return l.val(_onfalse->advance(si, sf));
}


ModBlock::ModBlock(StepperBlock * s,
                       std::function <
                           double(const StepperInfo * si, double dt)> func)
    : _stepper(s), _func(func)
{
}

double
ModBlock::advance(const StepperInfo * si, StepperFeedback * sf)
{
  Logger l("Constr");
  return l.val(_func(si, _stepper->advance(si, sf)));
}

RootBlock::RootBlock(std::function<double(const StepperInfo * si)> func)
    : _func(func)
{
}

double
RootBlock::advance(const StepperInfo * si, StepperFeedback *)
{
  return _func(si);
}

StepperBlock *
BaseStepper::constant(double dt)
{
  return new RootBlock([=](const StepperInfo * si) { return dt; });
}

StepperBlock *
BaseStepper::prevdt()
{
  return new RootBlock([=](const StepperInfo * si) { return si->prev_dt; });
}

StepperBlock *
BaseStepper::fixedTimes(std::vector<double> times, double tol)
{
  return new RootBlock([=](const StepperInfo * si) {
    if (times.size() == 0)
      return std::numeric_limits<double>::infinity();

    for (int i = 0; i < times.size(); i++)
    {
      double t0 = times[i];
      if (si->time < t0 - tol)
        return t0 - si->time;
    }
    return std::numeric_limits<double>::infinity();
  });
}

StepperBlock *
BaseStepper::ptr(const double * dt_store)
{
  return new RootBlock([=](const StepperInfo * si) { return *dt_store; });
}

StepperBlock *
BaseStepper::dtLimit(StepperBlock * s, double dt_min, double dt_max)
{
  return new ModBlock(s, [=](const StepperInfo * si, double dt) {
    if (dt < dt_min)
      return dt_min;
    else if (dt > dt_max)
      return dt_max;
    return dt;
  });
}

StepperBlock *
BaseStepper::bounds(StepperBlock * s, double t_min, double t_max)
{
  return new ModBlock(s, [=](const StepperInfo * si, double dt) {
    double t = si->time + dt;
    if (t < t_min)
      return t_min - si->time;
    else if (t > t_max)
      return t_max - si->time;
    return dt;
  });
}

StepperBlock *
BaseStepper::maxRatio(StepperBlock * s, double max_ratio)
{
  return new ModBlock(s, [=](const StepperInfo * si, double dt) {
    if (si->prev_dt > 0 && dt / si->prev_dt > max_ratio)
      dt = si->prev_dt * max_ratio;
    return dt;
  });
}

StepperBlock *
BaseStepper::mult(double mult, StepperBlock * s)
{
  if (!s)
    s = prevdt();
  return new ModBlock(
      s, [=](const StepperInfo * si, double dt) { return dt * mult; });
}

StepperBlock *
BaseStepper::between(StepperBlock * on, StepperBlock * between, std::vector<double> times,
                   double tol)
{
  return new IfBlock(on, between, [=](const StepperInfo * si) {
    for (int i = 0; i < times.size(); i++)
    {
      if (std::abs(si->time - times[i]) < tol)
        return true;
    }
    return false;
  });
}

StepperBlock *
BaseStepper::everyN(StepperBlock * nth, StepperBlock * between, int every_n, int offset)
{
  return new IfBlock(nth, between, [=](const StepperInfo * si) {
    return (si->step_count + (every_n - offset) - 1) % every_n == 0;
  });
}

StepperBlock *
BaseStepper::initialN(StepperBlock * initial, StepperBlock * primary, int n)
{
  return new IfBlock(initial, primary, [=](const StepperInfo * si) {
    return si->step_count <= n;
  });
}

StepperBlock *
BaseStepper::converged(StepperBlock * converged, StepperBlock * not_converged, bool delay)
{
  return new IfBlock(converged, not_converged, [=](const StepperInfo * si) {
    return si->converged && (!delay || si->prev_converged);
  });
}

StepperBlock *
BaseStepper::min(StepperBlock * a, StepperBlock * b, double tol)
{
  return new MinOfBlock(a, b, tol);
}
