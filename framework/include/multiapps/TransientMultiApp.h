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
   * @param var_name The name of the variable you are going to be transferring to.
   * @return The vector to fill.
   */
  virtual NumericVector<Number> & appTransferVector(unsigned int app, std::string var_name);

  virtual void init();

  /**
   * Advance all of the apps one timestep.
   */
  void solveStep(Real dt, Real target_time, bool auto_advance=true);

  /**
   * Actually advances time and causes output.
   */
  virtual void advanceStep();

  /**
   * Finds the smallest dt from among any of the apps.
   */
  Real computeDT();

  /**
   * "Reset" the App corresponding to the global App number
   * passed in.  "Reset" means that the App will be deleted
   * and recreated.  The time for the new App will be set
   * to the current simulation time.  This might be handy
   * if some sub-app in your simulation needs to get replaced
   * by a "new" piece of material.
   *
   * @param global_app The global app number to reset.
   * @param time - The time to reset the app to.
   */
  virtual void resetApp(unsigned int global_app, Real time);

private:
  /**
   * Setup the executioner for the local app.
   *
   * @param i The local app number for the app that needs to be setup.
   * @param time The time to set as the current time for the App
   * @param output_initial Whether or not the app should be allowed to output its initial condition
   */
  void setupApp(unsigned int i, Real time = 0.0, bool output_initial = true);

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

  /// Is it our first time through the execution loop?
  bool & _first;

  /// The variables that have been transferred to.  Used when doing transfer interpolation.  This will be cleared after each solve.
  std::vector<std::string> _transferred_vars;

  /// The DoFs associated with all of the currently transferred variables.
  std::set<dof_id_type> _transferred_dofs;

  std::vector<std::map<std::string, unsigned int> > _output_file_numbers;

  bool _auto_advance;
};

#endif // TRANSIENTMULTIAPP_H
