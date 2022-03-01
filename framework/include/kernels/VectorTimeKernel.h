//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorKernel.h"

/**
 * All vector time kernels should inherit from this class
 *
 */
class VectorTimeKernel : public VectorKernel
{
public:
  static InputParameters validParams();

  VectorTimeKernel(const InputParameters & parameters);

  virtual void computeResidual() override;

protected:
  /// Time derivative of u
  const VectorVariableValue & _u_dot;

  /// Derivative of u_dot with respect to u
  const VariableValue & _du_dot_du;
};
