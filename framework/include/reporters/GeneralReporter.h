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
};
