//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

class RayTracingAngularQuadratureErrorTest : public GeneralUserObject
{
public:
  RayTracingAngularQuadratureErrorTest(const InputParameters & parameters);

  static InputParameters validParams();

  void initialize(){};
  void finalize(){};
  void execute(){};
};
