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
 * Coupled auxiliary scalar value
 */
class CoupledScalarAux : public AuxKernel
{
public:
  static InputParameters validParams();

  CoupledScalarAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// The time level of the coupled variable
  MooseEnum _lag;
  /// Coupled variable
  const VariableValue & _coupled_val;

  /// The component of the scalar variable
  unsigned int _component;
};
