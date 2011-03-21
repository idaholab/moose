#ifndef TRANSIENT_H
#define TRANSIENT_H

#include "Moose.h"
#include "Executioner.h"
#include "LinearInterpolation.h"
#include "MProblem.h"

#include "mesh_function.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

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

  virtual Moose::Problem & problem() { return _problem; }

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

  /**
   * Transient loop will continue as long as this keeps returning true.
   */
  virtual bool keepGoing();

  /**
   * Whether or not the last solve converged.
   */
  virtual bool lastSolveConverged();

protected:
  virtual void preExecute();

  Moose::MProblem _problem;

  int & _t_step;                        /// Current timestep.
  Real & _time;                         /// Current time
  Real _time_old;
  Real _input_dt;                       /// The dt from the input file.
  Real & _dt;                           /// Current delta t... or timestep size.
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
  
  /**
   * Whether or not the previous solve converged.
   */
  bool _converged;

  std::vector<Real> _sync_times;
  std::vector<Real>::iterator _curr_sync_time_iter;
  bool _remaining_sync_time;

  LinearInterpolation _time_ipol;               /// Piecewise linear definition of time stepping
  bool _use_time_ipol;                          /// true if we want to use piecewise-defined time stepping
};

#endif //TRANSIENTEXECUTIONER_H
