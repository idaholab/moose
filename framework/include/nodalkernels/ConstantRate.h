//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONSTANTRATE_H
#define CONSTANTRATE_H

#include "NodalKernel.h"

// Forward Declarations
class ConstantRate;

template <>
InputParameters validParams<ConstantRate>();

/**
 * Represents the rate in a simple ODE of du/dt = rate
 */
class ConstantRate : public NodalKernel
{
public:
  /**
   * Constructor initializes the rate
   */
  ConstantRate(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  /// The rate
  const Real & _rate;
};

#endif
