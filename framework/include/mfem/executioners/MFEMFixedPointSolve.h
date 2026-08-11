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

// System includes
#include <string>

class MFEMFixedPointSolve : public FixedPointSolve
{
public:
  MFEMFixedPointSolve(Executioner & ex) : FixedPointSolve(ex){};

  virtual ~MFEMFixedPointSolve() = default;

  virtual void initialSetup() override{};

  /**
   * Allocate storage for the fixed point algorithm.
   * This creates the system vector of old (older, pre/post solve) variable values and the
   * array of old (older, pre/post solve) postprocessor values.
   *
   * @param primary Whether this routine is to allocate storage for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void allocateStorage(const bool /*primary*/) override{};

  /// Print the convergence history of the coupling, at every fixed point iteration
  virtual void printFixedPointConvergenceHistory(
      Real /*initial_norm*/,
      const std::vector<Real> & /*timestep_begin_norms*/,
      const std::vector<Real> & /*timestep_end_norms*/) const override{};

protected:
  virtual void
  updateVariableDoFsForTransform(const std::vector<std::string> & /*transformed_var_names*/,
                                 const bool /*primary*/) override{};

  /**
   * Saves the current values of the variables, and update the old(er) vectors.
   *
   * @param primary Whether this routine is to save the variables for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void saveVariableValues(const bool /*primary*/) override{};

  /**
   * Saves the current values of the postprocessors, and update the old(er) vectors.
   *
   * @param primary Whether this routine is to save the variables for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void savePostprocessorValues(const bool /*primary*/) override{};

  /// Save both the variable and postprocessor values
  virtual void saveAllValues(const bool /*primary*/) override{};

  /**
   * Use the fixed point algorithm transform instead of simply using the Picard update
   * This routine can be used to alternate Picard iterations and fixed point algorithm
   * updates based on the values of the variables before and after a solve / a Picard iteration.
   *
   * @param primary Whether this routine is used for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual bool useFixedPointAlgorithmUpdateInsteadOfPicard(const bool /*primary*/) override
  {
    return false;
  }

  virtual void copyPreviousFixedPointSolutions() override{};

  /**
   * Use the fixed point algorithm to transform the postprocessors.
   * If this routine is not called, the next value of the postprocessors will just be from
   * the unrelaxed Picard fixed point algorithm.
   *
   * @param primary Whether this routine is to save the variables for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void transformPostprocessors(const bool /*primary*/) override{};

  /**
   * Use the fixed point algorithm to transform the variables.
   * If this routine is not called, the next value of the variables will just be from
   * the unrelaxed Picard fixed point algorithm.
   *
   * @param primary Whether this routine is to save the variables for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void transformVariables(const bool /*primary*/) override{};
};
