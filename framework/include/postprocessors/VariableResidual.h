//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VARIABLERESIDUAL_H
#define VARIABLERESIDUAL_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class VariableResidual;

template <>
InputParameters validParams<VariableResidual>();

class VariableResidual : public GeneralPostprocessor
{
public:
  VariableResidual(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  virtual PostprocessorValue getValue() override;

protected:
  /// MOOSE variable we compute the residual for
  MooseVariable & _var;
  /// The residual of the variable
  Real _var_residual;
};

#endif // VARIABLERESIDUAL_H
