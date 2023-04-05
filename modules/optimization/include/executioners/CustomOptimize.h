//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Steady.h"
#include "CustomOptimizeSolve.h"

// System includes
#include <string>

// Forward declarations
class InputParameters;
class FEProblemBase;

class CustomOptimize : public Steady
{
public:
  static InputParameters validParams();

  CustomOptimize(const InputParameters & parameters);

  virtual void execute() override;

  CustomOptimizeSolve & getOptimizeSolve() { return _optim_solve; }

protected:
  CustomOptimizeSolve _optim_solve;
};
