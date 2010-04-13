#ifndef TRANSIENTEXECUTIONER_H
#define TRANSIENTEXECUTIONER_H

#include "Moose.h"
#include "Executioner.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

// Forward Declarations
class TransientExecutioner;
template<>
InputParameters validParams<TransientExecutioner>();

/**
 * TransientExecutioner executioners usually loop through a number of timesteps... calling solve()
 * for each timestep.
 */
class TransientExecutioner: public Executioner
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the Executioner in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  TransientExecutioner(std::string name, MooseSystem & moose_system, InputParameters parameters);

  /**
   * This will call solve() on the NonlinearSystem.
   */
  virtual bool execute();

protected:

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

  /**
   * Current timestep.
   *
   * Please don't modify this directly!
   */
  int & _t_step;

  /**
   * Current time.
   *
   * Please don't modify this directly!
   */
  double & _time;

  /**
   * Current delta t... or timestep size.
   *
   * Please don't modify this directly!
   */
  double & _dt;

  double _end_time;
  double _dtmin;
  double _dtmax;
  int _num_steps;
  int _n_startup_steps;

  /**
   * Steady state detection variables:
   */
  bool _trans_ss_check;
  double _ss_check_tol;
  double _ss_tmin;
  Real _old_time_solution_norm;
  
private:
  /**
   * Whether or not the previous solve converged.
   */
  bool _converged;
};

#endif //TRANSIENTEXECUTIONER_H
