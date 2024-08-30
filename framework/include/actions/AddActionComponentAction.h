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
#include <string>

/**
 * Action for creating component actions.
 */
class AddActionComponentAction : public Action
{
public:
  static InputParameters validParams();

  AddActionComponentAction(const InputParameters & params);

  virtual void act() override;

  /// Adds relationship managers, in case there are known mesh complexities to handle
  /// in parallel
  using Action::addRelationshipManagers;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType when_type) override;

  /// Return the parameters of the component
  InputParameters & getComponentParams() { return _component_params; }

private:
  /// The Component type that is being created
  std::string _component_type;

  /// The parameters for the component to be created
  InputParameters _component_params;
};
