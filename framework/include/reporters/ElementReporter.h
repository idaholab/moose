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
#include "ElementUserObject.h"
#include "Reporter.h"

/**
 */
class ElementReporter : public ElementUserObject, public Reporter
{
public:
  static InputParameters validParams();

  ElementReporter(const InputParameters & parameters);

  /**
   * @returns Whether or not this Reporter should store its value at this specific time.
   *
   * If the private parameter '_always_store' is true, this will always return true.
   * Otherwise, it will return true if the current execute flag matches a flag
   * that this ElementReporter has in its 'execute_on' parameter. Otherwise, it will
   * return false.
   *
   * This enables ElementReporter objects that do not fill information ahead of time in
   * execute() but instead fill their information in the to_json implementation.
   * Without this, said ElementReporters would always output their information even though
   * the user requested that they do not execute on a specific flag.
   */
  bool shouldStore() const override final;

private:
  /// Whether or not this ElementReporter should always store its information; see shouldStore()
  const bool _always_store;
};
