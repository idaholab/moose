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

class LibMeshFixedPointSolve : public FixedPointSolve
{
public:
  LibMeshFixedPointSolve(Executioner & ex);

  virtual ~LibMeshFixedPointSolve() = default;

  virtual void initialSetup() override;

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
                                  const bool primary) = 0;

protected:
  /**
   * Saves the current values of the variables, and update the old(er) vectors.
   *
   * @param primary Whether this routine is to save the variables for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void saveVariableValues(const bool primary) = 0;

  /**
   * Saves the current values of the postprocessors, and update the old(er) vectors.
   *
   * @param primary Whether this routine is to save the variables for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void savePostprocessorValues(const bool primary) = 0;

  /// Save both the variable and postprocessor values
  virtual void saveAllValues(const bool primary);

  /// Find the system holding the variables to be transformed (accelerated or relaxed)
  /// @param primary whether we are looking at transformations as the parent or child app
  void findTransformedSystem(const bool primary);

  /// System holding the transformed variables
  SystemBase * _transformed_sys;
  /// All the systems that should save their previous solutions
  std::set<SystemBase *> _systems_to_copy_previous_solutions_for;
};
