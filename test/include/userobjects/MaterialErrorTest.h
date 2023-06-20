//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"

class MaterialErrorTest : public ElementUserObject
{
public:
  static InputParameters validParams();

  MaterialErrorTest(const InputParameters & params);

  void initialize() {}
  void execute() {}
  void finalize() {}
  void threadJoin(const UserObject &) {}
};
