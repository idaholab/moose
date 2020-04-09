//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Control.h"

/**
 * Base class for controls that enable/disable object(s) based on some condition
 */
class ConditionalEnableControl : public Control
{
public:
  static InputParameters validParams();

  ConditionalEnableControl(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /**
   * Condition that must be true for an entry of the "enable" list to be enabled
   * and/or an entry of the "disable" list to be disabled
   *
   * @param[in] i   Index of entry within enable and/or disable list for which the condition applies
   */
  virtual bool conditionMet(const unsigned int & i) = 0;

  /// List of objects to enable if condition is met
  const std::vector<std::string> & _enable;

  /// List of objects to disable if condition is met
  const std::vector<std::string> & _disable;

  /// When true, the disable/enable lists are set to opposite values when the specified condition is false
  const bool & _reverse_on_false;
};
