//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

class PiecewiseBase;

/**
 * Tabulate the function nodes of a piecewise function, such as PiecewiseLinear or
 * PiecewiseConstant
 */
class PiecewiseFunctionTabulate : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  PiecewiseFunctionTabulate(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /// The piecewise function to tabulate
  const PiecewiseBase * const _piecewise_function;

  /// function argument (x) column
  VectorPostprocessorValue & _x_col;

  /// function value (x) column
  VectorPostprocessorValue & _y_col;
};
