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
#include "GeneralPostprocessor.h"

/**
 * Testing object to make sure the maximum number of n dofs per element is computed properly
 */
class MaxVarNDofsPerElemPP : public GeneralPostprocessor
{
public:
  MaxVarNDofsPerElemPP(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void initialize() {}
  virtual void execute() {}

  virtual PostprocessorValue getValue();
};
