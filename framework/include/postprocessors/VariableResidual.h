//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class VariableResidual : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  VariableResidual(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  virtual PostprocessorValue getValue() override;

protected:
  /// MOOSE variable we compute the residual for
  MooseVariableFEBase & _var;
  /// The residual of the variable
  Real _var_residual;
};
