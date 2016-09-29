#pragma once

struct StepperInfo {
  Real prev_dt;
  unsigned int nonlin_iters;
  unsigned int lin_iters;
  unsigned int time;
  bool converged;
};

class Stepper
{
public:
  // Implementations of advance should strive to be idempotent.  Return the
  // next value of dt.
  virtual double advance(StepperInfo* si) = 0;
};

class FixedPointStepper : public Stepper {
public:
  FixedPointStepper(std::vector<double> times, double tol) : _times(times), _time_tol(tol) { }

  virtual double advance(StepperInfo* si) {
    if (_times.size() == 0)
      return si->prev_dt;
    if (si->time < _times[0])
      return _times[0] - si->time;

    for (int i = 1; i < _times.size(); i++)
    {
      double t1 = _times[i-1];
      double t2 = _times[i];
      if (t1 <= si->time && si->time <= t2)
        return t2 - si->time;
    }
    return si->prev_dt;
  }

private:
  std::vector<double> _times;
  double _time_tol;
};

class MinOfSteper : public Stepper
{
public:
  MinOfStepper(Stepper* a, Stepper* b) : _a(a), _b(b) { }

  virtual double advance(StepperInfo* si)
  {
    return std::min(_a->advance(si), _b->advance());
  }

private:
  Stepper* _a;
  Stepper* _b;
};

class AdaptiveStepper : public Stepper
{
public:
  virtual double advance(StepperInfo* si) {
    bool can_shrink = true;
    bool can_grow = si->converged;

    unsigned int shrink_nl_its = _optimal_iters + _iter_window;
    unsigned int shrink_l_its = _lin_iter_ratio*(_optimal_iters + _iter_window);
    unsigned int growth_nl_its = 0;
    unsigned int growth_l_its = 0;
    if (_optimal_iters > _iter_window)
    {
       growth_nl_its = _optimal_iters - _iter_window;
       growth_l_its = _lin_iter_ratio * (_optimal_iters - _iter_window);
    }

    if (can_grow && (si.nonlin_iters < growth_nl_its && si.lin_iters < growth_l_its))
      return si->prev_dt * _growth_factor;
    else if (can_shrink && (_nl_its > shrink_nl_its || _l_its > shrink_l_its))
      return si->prev_dt * _shrink_factor;
    else
      return si->prev_dt;
  };

  double _growth_factor;
  double _shrink_factor;
  unsigned int _optimal_iters;
  unsigned int _iter_window;
  unsigned int _lin_iter_ratio;
};

// *
