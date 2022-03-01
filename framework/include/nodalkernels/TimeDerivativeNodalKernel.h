//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeNodalKernel.h"

/**
 * Represents du/dt
 */
class TimeDerivativeNodalKernel : public TimeNodalKernel
{
public:
  /**
   * Constructor (Comment here for @aeslaughter :-)
   */
  static InputParameters validParams();

  TimeDerivativeNodalKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;
};
