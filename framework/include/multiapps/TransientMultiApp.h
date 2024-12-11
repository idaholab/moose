//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiApp.h"

#include "libmesh/numeric_vector.h"

class TransientBase;

/**
 * MultiApp Implementation for Transient Apps.
 * In particular, this is important because TransientMultiApps
 * will be taken into account in the time step selection process.
 */
class TransientMultiApp : public MultiApp
{
public:
  static InputParameters validParams();

  TransientMultiApp(const InputParameters & parameters);

  virtual NumericVector<Number> & appTransferVector(unsigned int app,
                                                    std::string var_name) override;

  virtual void initialSetup() override;

  virtual bool solveStep(Real dt, Real target_time, bool auto_advance = true) override;

  virtual void incrementTStep(Real target_time) override;

  virtual void finishStep(bool recurse_through_multiapp_levels = false) override;

  virtual void resetApp(unsigned int global_app, Real time) override;

  /**
   * Finds the smallest dt from among any of the apps.
   */
  Real computeDT();

private:
  /**
   * Setup the executioner for the local app.
   *
   * @param i The local app number for the app that needs to be setup.
   * @param time The time to set as the current time for the App
   */
  void setupApp(unsigned int i, Real time = 0.0);

  std::vector<TransientBase *> _transient_executioners;

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

  std::vector<std::map<std::string, unsigned int>> _output_file_numbers;

  bool _auto_advance;

  std::set<unsigned int> _reset;

  /// Flag for toggling console output on sub cycles
  bool _print_sub_cycles;
};

/**
 * Utility class for catching solve failure errors so that MOOSE
 * can recover state before continuing.
 */
class MultiAppSolveFailure : public std::runtime_error
{
public:
  MultiAppSolveFailure(const std::string & error) throw() : runtime_error(error) {}

  MultiAppSolveFailure(const MultiAppSolveFailure & e) throw() : runtime_error(e) {}

  ~MultiAppSolveFailure() throw() {}
};
