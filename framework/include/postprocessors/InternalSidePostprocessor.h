//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InternalSideUserObject.h"
#include "Postprocessor.h"

class InternalSidePostprocessor : public InternalSideUserObject, public Postprocessor
{
public:
  static InputParameters validParams();

  InternalSidePostprocessor(const InputParameters & parameters);
};
