//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "PeriodicBCHelper.h"

class MooseMesh;
namespace libMesh
{
class PeriodicBoundaryBase;
}

/**
 * This Action adds a periodic boundary to the problem. Note that Periodic Boundaries
 * are not MooseObjects so you need not specify a type for these boundaries.  If you
 * do, it will currently be ignored by this Action.
 */
class AddPeriodicBCAction : public Action, public Moose::PeriodicBCHelper
{
public:
  static InputParameters validParams();

  AddPeriodicBCAction(const InputParameters & params);

  virtual void act() override;

protected:
  using Action::paramError;

  /**
   * Overriden method from PeriodicBCHelper that is called on every periodic
   * boundary, in which we will setup the algebraic ghosting via the DofMap
   * and the MooseMesh context if possible.
   */
  virtual void onSetupPeriodicBoundary(libMesh::PeriodicBoundaryBase & p) override;

  /**
   * Helper for getting the variables to which periodic boundary
   * conditions will be applied
   */
  std::vector<const MooseVariableFieldBase *> getVariables() const;

  /// The variables to apply periodic boundary conditions to
  std::vector<const MooseVariableFieldBase *> _vars;

  MooseMesh * _mesh;
};
