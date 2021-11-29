//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalKernel.h"

class Function;

/**
 * Represents a simple ODE of du/dt - rate = 0
 */
class TimeNodalKernel : public NodalKernel
{
public:
  /**
   * Constructor initializes the rate
   */
  static InputParameters validParams();

  TimeNodalKernel(const InputParameters & parameters);

protected:
  virtual void computeResidual() override;

  /// Time derivative of u
  const VariableValue & _u_dot;

  /// Derivative of u_dot with respect to u
  const VariableValue & _du_dot_du;
};
