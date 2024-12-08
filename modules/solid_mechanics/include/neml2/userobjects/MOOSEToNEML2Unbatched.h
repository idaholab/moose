//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MOOSEToNEML2.h"
#include "GeneralUserObject.h"

class MOOSEToNEML2Unbatched : public MOOSEToNEML2, public GeneralUserObject
{
public:
  static InputParameters validParams();

  MOOSEToNEML2Unbatched(const InputParameters & params);

  void initialize() override {}
  void execute() override {}
  void finalize() override {}
};
