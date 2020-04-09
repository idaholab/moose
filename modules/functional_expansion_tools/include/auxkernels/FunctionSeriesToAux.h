//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionAux.h"

/**
 * Specialization of FunctionAux that is designed to work specifically with FXs, namely that it is
 * always processed at timestep_begin
 */
class FunctionSeriesToAux : public FunctionAux
{
public:
  static InputParameters validParams();

  FunctionSeriesToAux(const InputParameters & parameters);
};
