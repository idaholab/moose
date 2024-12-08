//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Action.h"
#include "libmesh/fe_type.h"

class ConservedAction : public Action
{
public:
  static InputParameters validParams();

  ConservedAction(const InputParameters & params);

  virtual void act() override;

protected:
  /// Type of solve
  enum class SolveType
  {
    DIRECT,
    REVERSE_SPLIT,
    FORWARD_SPLIT
  };
  /// Name of chemical potential variable for split solves
  std::string _chempot_name;
  /// Type of solve to use used in the action
  const SolveType _solve_type;
  /// Name of the variable being created
  const NonlinearVariableName _var_name;
  /// FEType for the variable being created
  libMesh::FEType _fe_type;
  /// Scaling parameter
  const Real _scaling;
};
