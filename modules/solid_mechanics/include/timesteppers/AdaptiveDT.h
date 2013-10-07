#ifndef ADAPTIVESTEPPER_H
#define ADAPTIVESTEPPER_H

#include "TimeStepper.h"

#include "LinearInterpolation.h"

class Function;

/**
 *
 */
class AdaptiveDT : public TimeStepper
{
public:
  AdaptiveDT(const std::string & name, InputParameters parameters);
  virtual ~AdaptiveDT();

  virtual void init();

  virtual void rejectStep();

  virtual void acceptStep();

protected:
  virtual Real computeInitialDT();
  virtual Real computeDT();
  virtual Real computeFailedDT();
  void computeAdaptiveDT(Real & dt, bool allowToGrow=true, bool allowToShrink=true);
  void computeInterpolationDT(Real & dt);
  void limitDTByFunction(Real & limitedDT);

  Real & _dt_old;

  const Real _input_dt;                       ///< The dt from the input file.
  bool _synced_last_step;

  int _optimal_iterations;
  int _iteration_window;
  const int _linear_iteration_ratio;
  bool _adaptive_timestepping;

  Function * _timestep_limiting_function;
  Real _max_function_change;

  const std::vector<Real> _sync_times;
  std::vector<Real>::const_iterator _sync_times_iter;
  bool _remaining_sync_time;

  LinearInterpolation _time_ipol; ///< Piecewise linear definition of time stepping
  const bool _use_time_ipol;      ///< true if we want to use piecewise-defined time stepping
  const Real _growth_factor;
  const Real _cutback_factor;

  unsigned int & _nl_its;  /// Number of nonlinear iterations in previous solve
  unsigned int & _l_its;   /// Number of linear iterations in previous solve

};

template<>
InputParameters validParams<AdaptiveDT>();

#endif /* ADAPTIVESTEPPER_H */
