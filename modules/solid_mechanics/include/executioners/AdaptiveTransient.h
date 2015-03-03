/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef ADAPTIVETRANSIENT_H
#define ADAPTIVETRANSIENT_H

#include "Executioner.h"
#include "LinearInterpolation.h"
#include "FEProblem.h"

#include "libmesh/mesh_function.h"

// System includes
#include <string>
#include <sstream>
#include <iomanip>

// Forward Declarations
class AdaptiveTransient;

template<>
InputParameters validParams<AdaptiveTransient>();

/**
 * Transient executioners usually loop through a number of timesteps... calling solve()
 * for each timestep.
 */
class AdaptiveTransient: public Executioner
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the Executioner in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  AdaptiveTransient(const std::string & name, InputParameters parameters);

  virtual ~AdaptiveTransient();

  virtual Problem & problem() { return _problem; }

  virtual void init();

  /**
   * This will call solve() on the NonlinearSystem.
   */
  virtual void execute();

  /**
   * This should execute the solve for one timestep.
   */
  virtual void takeStep(Real input_dt = -1.0);

  /**
   * @return The fully constrained dt for this timestep
   */
  virtual Real computeConstrainedDT();

  /**
   * Optional override.
   *
   * @return The dt to use for this timestep.
   */
  virtual Real computeDT();

  void computeAdaptiveDT(Real &dt, bool allowToGrow=true, bool allowToShrink=true);

  Real limitDTByFunction(Real trialDt);

  /**
   * Transient loop will continue as long as this keeps returning true.
   */
  virtual bool keepGoing();

  /**
   * Whether or not the last solve converged.
   */
  virtual bool lastSolveConverged();

  virtual void preExecute();

  virtual void endStep();

  enum SyncType
  {
    SYNC,
    TIME_FUNC
  };

protected:
  FEProblem & _problem;

  MooseEnum _time_scheme;
  int & _t_step;                        ///< Current timestep.
  Real & _time;                         ///< Current time
  Real _time_old;
  Real _input_dt;                       ///< The dt from the input file.
  Real & _dt;                           ///< Current delta t... or timestep size.
  Real & _dt_old;

  Real _prev_dt;
  bool _synced_last_step;

  Real _end_time;
  Real _dtmin;
  Real _dtmax;
  Real _num_steps;
  int _n_startup_steps;

  int _optimal_iterations;
  int _iteration_window;
  int _linear_iteration_ratio;
  bool _adaptive_timestepping;

  Function * _timestep_limiting_function;
  std::string _timestep_limiting_function_name;
  Real _max_function_change;

  /**
   * Steady state detection variables:
   */
  bool _trans_ss_check;
  Real _ss_check_tol;
  Real _ss_tmin;
  Real _old_time_solution_norm;

  /**
   * Whether or not the previous solve converged.
   */
  bool _converged;
  bool _caught_exception;

  std::vector<std::pair<Real,SyncType> > _sync_times;
  std::vector<std::pair<Real,SyncType> >::iterator _curr_sync_time_iter;
  SyncType _last_sync_type;
  bool _remaining_sync_time;

  LinearInterpolation _time_ipol;               ///< Piecewise linear definition of time stepping
  bool _use_time_ipol;                          ///< true if we want to use piecewise-defined time stepping
  Real _growth_factor;
  Real _cutback_factor;
  bool _cutback_occurred;

  std::ostringstream _diag;

  void setupTimeIntegrator();
};

#endif //ADAPTIVETRANSIENT_H
