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

#ifndef TRANSIENT_H
#define TRANSIENT_H

#include "Executioner.h"
#include "LinearInterpolation.h"
#include "FEProblem.h"

// LibMesh includes
#include "libmesh/mesh_function.h"
#include "libmesh/parameters.h"

// System includes
#include <string>
#include <fstream>

// Forward Declarations
class Transient;

template<>
InputParameters validParams<Transient>();

/**
 * Transient executioners usually loop through a number of timesteps... calling solve()
 * for each timestep.
 */
class Transient: public Executioner
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the Executioner in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  Transient(const std::string & name, InputParameters parameters);

  virtual ~Transient();

  virtual Problem & problem();

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
  virtual void estimateTimeError();
  /**
   * Optional override.
   *
   * @return The dt to use for this timestep.
   */
  virtual Real computeDT();

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

  /**
   * Can be used to set the next "target time" which is a time to nail perfectly.
   * Useful for driving MultiApps.
   */
  virtual void setTargetTime(Real target_time);

  /**
   * Get the current time.
   */
  virtual Real getTime() { return _time; };

protected:
  FEProblem & _problem;

  /// Current timestep.
  int & _t_step;
  /// Current time
  Real & _time;
  /// Previous time
  Real & _time_old;
  /// The dt from the input file.
  Real _input_dt;
  /// Current delta t... or timestep size.
  Real & _dt;
  Real & _dt_old;

  Real _prev_dt;
  bool _reset_dt;

  Real _end_time;
  Real _dtmin;
  Real _dtmax;
  Real _num_steps;
  int _n_startup_steps;

  /**
   * Steady state detection variables:
   */
  bool _trans_ss_check;
  Real _ss_check_tol;
  Real _ss_tmin;
  Real _old_time_solution_norm;

  /// Whether or not the previous solve converged.
  bool _converged;

  std::set<Real> _sync_times;
  Real _prev_sync_time;
  bool _remaining_sync_time;

  /// Piecewise linear definition of time stepping
  LinearInterpolation _time_ipol;
  /// true if we want to use piecewise-defined time stepping
  bool _use_time_ipol;
  Real _growth_factor;
  bool _cutback_occurred;
  bool _abort;
  bool _estimate_error;
  bool _time_error_out_to_file;
  Real _error;
  Real _cumulative_error;
  std::string _time_errors_filename;
  std::ofstream _time_error_file;
  ///if to use time interval output
  bool _time_interval;
  ///the output interval to use
  Real _time_interval_output_interval;
  Real start_time;
  Real _timestep_tolerance;
  Real _target_time;
  bool _use_multiapp_dt;
};

#endif //TRANSIENTEXECUTIONER_H
