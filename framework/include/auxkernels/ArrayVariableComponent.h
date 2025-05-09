//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class ArrayVariableComponent : public AuxKernel
{
public:
  static InputParameters validParams();

  ArrayVariableComponent(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Holds the solution at current quadrature points
  const ArrayVariableValue & _u;
  /// The component
  const unsigned int _component;
};
