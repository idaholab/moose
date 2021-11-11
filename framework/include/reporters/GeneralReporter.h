//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Reporter.h"
#include "GeneralUserObject.h"

/**
 * Reporter object that has a single execution of the "execute" method for for each execute flag.
 */
class GeneralReporter : public GeneralUserObject, public Reporter
{
public:
  static InputParameters validParams();
  GeneralReporter(const InputParameters & parameters);

  // These objects are not threaded
  void threadJoin(const UserObject &) final {}

  /**
   * @returns Whether or not this Reporter should store its value at this specific time.
   *
   * If the private parameter '_always_store' is true, this will always return true.
   * Otherwise, it will return true if the current execute flag matches a flag
   * that this GeneralReporter has in its 'execute_on' parameter. Otherwise, it will
   * return false.
   *
   * This enables GeneralReporter objects that do not fill information ahead of time in
   * execute() but instead fill their information in the to_json implementation.
   * Without this, said GeneralReporters would always output their information even though
   * the user requested that they do not execute on a specific flag.
   */
  bool shouldStore() const override final;

private:
  /// Whether or not this GeneralReporter should always store its information; see shouldStore()
  const bool _always_store;
};
