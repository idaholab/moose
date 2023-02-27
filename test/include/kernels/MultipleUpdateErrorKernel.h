//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

/**
 * Kernel that tries to obtain a writable reference to a variable, but fails immediately because
 * only AuxKernels, ElementUserObjects, and NodalUserObjects are permitted to get a writable
 * reference.
 */
class MultipleUpdateErrorKernel : public Kernel
{
public:
  static InputParameters validParams();

  MultipleUpdateErrorKernel(const InputParameters & parameters);

protected:
  Real computeQpResidual() override;

  /// use deprecated API
  const bool _deprecated;

  /// current API
  MooseVariable * _var1;
  MooseVariable * _var2;

  /// deprectated API
  VariableValue * _dvar1;
  VariableValue * _dvar2;
};
