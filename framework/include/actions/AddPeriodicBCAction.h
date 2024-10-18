//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

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
class AddPeriodicBCAction : public Action
{
public:
  static InputParameters validParams();

  AddPeriodicBCAction(const InputParameters & params);

  virtual void act() override;

protected:
  /**
   * This function will automatically add the correct translation vectors for
   * each requested dimension when using GeneratedMesh
   * @returns a boolean indicating whether or not these boundaries were automatically added
   */
  bool autoTranslationBoundaries();

  void setPeriodicVars(libMesh::PeriodicBoundaryBase & p,
                       const std::vector<VariableName> & var_names);

  MooseMesh * _mesh;
};
