//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Copies one variable onto an auxiliary variable
 */
class CopyValueAux : public AuxKernel
{
public:
  static InputParameters validParams();

  CopyValueAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// The variable to project from
  const VariableValue & _v;

  /// A reference to the variable to project from
  const MooseVariable & _source_variable;
};
