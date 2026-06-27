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

protected:
  virtual void
  updateVariableDoFsForTransform(const std::vector<std::string> & transformed_var_names,
                                 const bool primary) override;

  /// Save both the variable and postprocessor values
  virtual void saveAllValues(const bool primary) override;

  virtual void copyPreviousFixedPointSolutions() override;

  /// Find the system holding the variables to be transformed (accelerated or relaxed)
  /// @param primary whether we are looking at transformations as the parent or child app
  void findTransformedSystem(const bool primary);

  std::set<dof_id_type> _transformed_dofs;
  std::set<dof_id_type> _secondary_transformed_dofs;

  /// System holding the transformed variables
  SystemBase * _transformed_sys{nullptr};
  /// All the systems that should save their previous solutions
  std::set<SystemBase *> _systems_to_copy_previous_solutions_for;
};
