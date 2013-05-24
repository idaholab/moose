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
#ifndef TRANSIENTMULTIAPP_H
#define TRANSIENTMULTIAPP_H

#include "MultiApp.h"
#include "MooseApp.h"
#include "Transient.h"
#include "TransientInterface.h"

class TransientMultiApp;

template<>
InputParameters validParams<TransientMultiApp>();

/**
 * MultiApp Implementation for Transient Apps.
 * In particular, this is important because TransientMultiApps
 * will be taken into account in the time step selection process.
 */
class TransientMultiApp :
  public MultiApp
{
public:
  TransientMultiApp(const std::string & name, InputParameters parameters);

  virtual ~TransientMultiApp();

  /**
   * Get the vector to transfer to for this MultiApp.
   * In general this is the Auxiliary system solution vector.
   * But - in the case of doing transfer interpolation this might be different.
   *
   * @param app The global app number you want the transfer vector for.
   * @param The name of the variable you are going to be transferring to.
   * @return The vector to fill.
   */
  virtual NumericVector<Number> & appTransferVector(unsigned int app, std::string var_name);

  /**
   * Advance all of the apps one timestep.
   */
  void solveStep(Real dt, Real target_time);

  /**
   * Finds the smallest dt from among any of the apps.
   */
  Real computeDT();

private:
  std::vector<Transient *> _transient_executioners;

  bool _sub_cycling;
  bool _interpolate_transfers;
  bool _detect_steady_state;
  Real _steady_state_tol;
  bool _output_sub_cycles;

  unsigned int _max_failures;
  bool _tolerate_failure;

  unsigned int _failures;

  bool _catch_up;
  Real _max_catch_up_steps;

  /// The variables that have been transferred to.  Used when doing transfer interpolation.  This will be cleared after each solve.
  std::vector<std::string> _transferred_vars;

  /// The DoFs associated with all of the currently transferred variables.
  std::set<unsigned int> _transferred_dofs;
};

#endif // TRANSIENTMULTIAPP_H
