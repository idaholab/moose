//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Control.h"

class Function;

class InputMatrixControl : public Control
{
public:
  /**
   * Class constructor
   * @param parameters Input parameters for this Control object
   */
  static InputParameters validParams();

  InputMatrixControl(const InputParameters & parameters);

  virtual void execute() override;

private:
  /// Function controlling the number of rows
  const Function & _nrow_function;
  /// Function controlling the number of columns
  const Function & _ncol_function;
  /// Function controlling the sample value
  const Function & _sample_function;
};
