
#include "Stepper.h"

int Logger::level = 0;
bool Logger::on = true;

ConstStepper::ConstStepper(double dt) : _dt(dt)
{
}

double
ConstStepper::advance(const StepperInfo *, StepperFeedback *)
{
  Logger l("Const");
  return l.val(_dt);
}

DTLimitStepper::DTLimitStepper(Stepper * s, double dt_min, double dt_max, bool throw_err)
    : _stepper(s), _min(dt_min), _max(dt_max), _err(throw_err)
{
}

double
DTLimitStepper::advance(const StepperInfo * si, StepperFeedback * sf)
{
  Logger l("DTLimit");
  double dt = _stepper->advance(si, sf);
  if (_err && (dt < _min || dt > _max))
    throw "time step is out of bounds";
  else if (dt < _min)
    return l.val(_min);
  else if (dt > _max)
    return l.val(_max);
  return l.val(dt);
}

BoundsStepper::BoundsStepper(Stepper * s, double t_min, double t_max, bool throw_err)
    : _stepper(s), _min(t_min), _max(t_max), _err(throw_err)
{
}

double
BoundsStepper::advance(const StepperInfo * si, StepperFeedback * sf)
{
  Logger l("Bounds");
  double dt = _stepper->advance(si, sf);
  double t = si->time + dt;
  if (_err && (t < _min || t > _max))
    throw "time step is out of bounds";
  else if (t < _min)
    return l.val(_min - si->time);
  else if (t > _max)
    return l.val(_max - si->time);
  return l.val(dt);
}

FixedPointStepper::FixedPointStepper(std::vector<double> times, double tol)
    : _times(times), _time_tol(tol)
{
}

double
FixedPointStepper::advance(const StepperInfo * si, StepperFeedback *)
{
  Logger l("FixedPoint");
  if (_times.size() == 0)
    return l.val(std::numeric_limits<double>::infinity());

  for (int i = 0; i < _times.size(); i++)
  {
    double t0 = _times[i];
    if (si->time < t0 - _time_tol)
      return l.val(t0 - si->time);
  }
  return l.val(std::numeric_limits<double>::infinity());
}

MaxRatioStepper::MaxRatioStepper(Stepper * s, double max_ratio) : _stepper(s), _max_ratio(max_ratio)
{
}

double
MaxRatioStepper::advance(const StepperInfo * si, StepperFeedback * sf)
{
  Logger l("MaxRatio");
  double dt = _stepper->advance(si, sf);
  if (si->prev_dt > 0 && dt / si->prev_dt > _max_ratio)
    dt = si->prev_dt * _max_ratio;
  return l.val(dt);
}

ReturnPtrStepper::ReturnPtrStepper(const double * dt_store) : _dt_store(dt_store)
{
}

double
ReturnPtrStepper::advance(const StepperInfo * si, StepperFeedback *)
{
  Logger l("ReturnPtr");
  return l.val(*_dt_store);
}

InstrumentedStepper::InstrumentedStepper(double * dt_store)
    : _stepper(nullptr), _dt_store(dt_store), _own(!dt_store)
{
  if (!_dt_store)
    _dt_store = new double(0);
}

InstrumentedStepper::~InstrumentedStepper()
{
  if (_own)
    delete _dt_store;
}

double
InstrumentedStepper::advance(const StepperInfo * si, StepperFeedback * sf)
{
  Logger l("Instrumented");
  if (!_stepper)
    throw "InstrumentedStepper's inner stepper not set";

  *_dt_store = _stepper->advance(si, sf);
  return l.val(*_dt_store);
}

void
InstrumentedStepper::setStepper(Stepper * s)
{
  _stepper.reset(s);
}

double *
InstrumentedStepper::dtPtr()
{
  return _dt_store;
}

RetryUnusedStepper::RetryUnusedStepper(Stepper * s, double tol, bool prev_prev)
    : _stepper(s), _tol(tol), _prev_prev(prev_prev), _prev_dt(0), _prev_time(0)
{
}

double
RetryUnusedStepper::advance(const StepperInfo * si, StepperFeedback * sf)
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

ConstrFuncStepper::ConstrFuncStepper(Stepper * s, std::function<double(double)> func,
                                     double max_diff)
    : _stepper(s), _func(func), _max_diff(max_diff)
{
}

double
ConstrFuncStepper::advance(const StepperInfo * si, StepperFeedback * sf)
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

PiecewiseStepper::PiecewiseStepper(std::vector<double> times, std::vector<double> dts,
                                   bool interpolate)
    : _times(times), _dts(dts), _interp(interpolate), _lin(times, dts)
{
}

double
PiecewiseStepper::advance(const StepperInfo * si, StepperFeedback *)
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

MinOfStepper::MinOfStepper(Stepper * a, Stepper * b, double tol) : _a(a), _b(b), _tol(tol)
{
}

double
MinOfStepper::advance(const StepperInfo * si, StepperFeedback * sf)
{
  Logger l("MinOf");
  double dta = _a->advance(si, sf);
  double dtb = _b->advance(si, sf);
  if (dta - _tol < dtb)
    return l.val(dta);
  return l.val(dtb);
}

AdaptiveStepper::AdaptiveStepper(unsigned int optimal_iters, unsigned int iter_window,
                                 double lin_iter_ratio, double shrink_factor, double growth_factor)
    : _optimal_iters(optimal_iters),
      _iter_window(iter_window),
      _lin_iter_ratio(lin_iter_ratio),
      _shrink_factor(shrink_factor),
      _growth_factor(growth_factor)
{
}

double
AdaptiveStepper::advance(const StepperInfo * si, StepperFeedback *)
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

SolveTimeAdaptiveStepper::SolveTimeAdaptiveStepper(int initial_direc, double percent_change)
    : _percent_change(percent_change), _direc(initial_direc), _n_steps(0)
{
}

double
SolveTimeAdaptiveStepper::advance(const StepperInfo * si, StepperFeedback *)
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

PredictorCorrectorStepper::PredictorCorrectorStepper(int start_adapting, double e_tol,
                                                     double scaling_param,
                                                     std::string time_integrator)
    : _start_adapting(start_adapting),
      _e_tol(e_tol),
      _scale_param(scaling_param),
      _time_integrator(time_integrator)
{
}

double
PredictorCorrectorStepper::advance(const StepperInfo * si, StepperFeedback *)
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
PredictorCorrectorStepper::estimateTimeError(const StepperInfo * si)
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

DT2Stepper::DT2Stepper(double time_tol, double e_tol, double e_max, int integrator_order)
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
DT2Stepper::resetWindow(double start, double dt)
{
  _start_time = start;
  _end_time = _start_time + dt;
  _big_soln.release();
  return dt;
}

double DT2Stepper::dt()
{
  return _end_time - _start_time;
}

double
DT2Stepper::advance(const StepperInfo * si, StepperFeedback * sf)
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
DT2Stepper::calcErr(const StepperInfo * si)
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

double
PrevDTStepper::advance(const StepperInfo * si, StepperFeedback *)
{
  Logger l("PrevDT");
  return l.val(si->prev_dt);
}

MultStepper::MultStepper(double mult, Stepper * s) : _stepper(s), _mult(mult)
{
  if (!_stepper)
    _stepper.reset(new PrevDTStepper());
}

double
MultStepper::advance(const StepperInfo * si, StepperFeedback * sf)
{
  Logger l("Mult");
  return l.val(_stepper->advance(si, sf) * _mult);
}

StepperIf::StepperIf(Stepper * on_true, Stepper * on_false, std::function<bool(const StepperInfo *)> func) : _ontrue(on_true), _onfalse(on_false), _func(func)
{
}

double
StepperIf::advance(const StepperInfo * si, StepperFeedback * sf)
{
  Logger l("OnFunc");
  bool val = _func(si);
  if (val)
    return l.val(_ontrue->advance(si, sf));
  return l.val(_onfalse->advance(si, sf));
}

Stepper *
StepperIf::between(Stepper * on, Stepper * between, std::vector<double> times, double tol)
{
  return new StepperIf(on, between, [=](const StepperInfo * si){
    for (int i = 0; i < times.size(); i++)
    {
      if (std::abs(si->time - times[i]) < tol)
        return true;
    }
    return false;
  });
}

Stepper *
StepperIf::everyN(Stepper * nth, Stepper * between, int every_n, int offset)
{
  return new StepperIf(nth, between, [=](const StepperInfo * si) {
    return (si->step_count + (every_n - offset) - 1) % every_n == 0;
  });
}

Stepper *
StepperIf::initialN(Stepper * initial, Stepper * primary, int n)
{
  return new StepperIf(initial, primary, [=](const StepperInfo * si) {
    return si->step_count <= n;
  });
}

Stepper *
StepperIf::converged(Stepper  * converged, Stepper * not_converged, bool delay)
{
  return new StepperIf(converged, not_converged, [=](const StepperInfo * si) {
    return si->converged && (!delay || si->prev_converged);
  });
}
