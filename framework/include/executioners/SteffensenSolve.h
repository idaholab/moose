//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FixedPointSolve.h"
#include "NonlinearSystemBase.h"

class SteffensenSolve : public FixedPointSolve
{
public:
  SteffensenSolve(Executioner & ex);

  static InputParameters validParams();

  virtual void initialSetup() override;

  /**
   * Allocate storage for the fixed point algorithm.
   * This creates the system vector of old (older, pre/post solve) variable values and the
   * array of old (older, pre/post solve) postprocessor values.
   *
   * @param primary Whether this routine is to allocate storage for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void allocateStorage(const bool primary) override final;

  virtual void printFixedPointConvergenceHistory(
      Real initial_norm,
      const std::vector<Real> & timestep_begin_norms,
      const std::vector<Real> & timestep_end_norms) const override final;

private:
  /**
   * Saves the current values of the variables, and update the old(er) vectors.
   *
   * @param primary Whether this routine is to save the variables for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void saveVariableValues(const bool primary) override final;

  /**
   * Saves the current values of the postprocessors, and update the old(er) vectors.
   *
   * @param primary Whether this routine is to save the variables for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void savePostprocessorValues(const bool primary) override final;

  /**
   * Use the fixed point algorithm transform instead of simply using the Picard update
   *
   * @param primary Whether this routine is used for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual bool useFixedPointAlgorithmUpdateInsteadOfPicard(const bool primary) override final;

  /**
   * Use the fixed point algorithm to transform the postprocessors.
   * If this routine is not called, the next value of the postprocessors will just be from
   * the unrelaxed Picard fixed point algorithm.
   *
   * @param primary Whether this routine is to save the variables for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void transformPostprocessors(const bool primary) override final;

  /**
   * Use the fixed point algorithm to transform the variables.
   * If this routine is not called, the next value of the variables will just be from
   * the unrelaxed Picard fixed point algorithm.
   *
   * @param transformed_dofs The dofs that will be affected by the algorithm
   * @param primary Whether this routine is to save the variables for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void transformVariables(const std::set<dof_id_type> & transformed_dofs,
                                  const bool primary) override final;

  /// Vector tag id for the most recent solution variable, pre-Steffensen transform, as a main app
  TagID _fxn_m1_tagid = Moose::INVALID_TAG_ID;

  /// Vector tag id for the solution variable before the latest solve, as a main app
  TagID _xn_m1_tagid = Moose::INVALID_TAG_ID;

  /// Vector tag id for the most recent solution variable, pre-Steffensen transform, as a sub app
  TagID _secondary_fxn_m1_tagid = Moose::INVALID_TAG_ID;

  /// Vector tag id for the solution variable before the latest solve, as a sub app
  TagID _secondary_xn_m1_tagid = Moose::INVALID_TAG_ID;
};
