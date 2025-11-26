//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActionComponent.h"
#include "THMProblem.h"

#define registerTHMActionComponentTasks(app_name, derived_name)                                    \
  registerMooseAction(app_name, derived_name, "THM:add_component");                                \
  registerMooseAction(app_name, derived_name, "THM:add_closures");                                 \
  registerMooseAction(app_name, derived_name, "THM:add_control_logic")

/**
 * Base class for ActionComponents that build THM components.
 */
class THMActionComponent : public ActionComponent
{
public:
  static InputParameters validParams();
  THMActionComponent(const InputParameters & params);

protected:
  virtual void actOnAdditionalTasks() override;
  virtual void addTHMComponents() {}
  virtual void addClosures() {}
  virtual void addControlLogic() {}

  /// Adds a THM component
  void addTHMComponent(const std::string & class_name,
                       const std::string & obj_name,
                       InputParameters & params);
  /// Adds a Closures object
  void addClosuresObject(const std::string & class_name,
                         const std::string & obj_name,
                         InputParameters & params);
  /// Adds a ControlLogic object
  void addControlLogicObject(const std::string & class_name,
                             const std::string & obj_name,
                             InputParameters & params);

  /// Gets the THM problem
  THMProblem & getTHMProblem();
};
