//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

/**
 * Base class for topological optimization constraints.
 * Override isConfigAllowed to implement custom constraint.
 */
class TopologicalConstraintBase : public GeneralUserObject
{
public:
  static InputParameters validParams();

  TopologicalConstraintBase(const InputParameters & params);

  void execute() override {}
  void initialize() override {}
  void finalize() override {}
  void initialSetup() override {}

  /// override isConfigAllowed to create custom constraint
  virtual bool isConfigAllowed(const std::vector<dof_id_type> config,
                               const MooseMesh * _subapp_mesh) const = 0;
};
