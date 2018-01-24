//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef USERFORCINGFUNCTIONNODALKERNEL_H
#define USERFORCINGFUNCTIONNODALKERNEL_H

#include "NodalKernel.h"

// Forward Declarations
class UserForcingFunctionNodalKernel;
class Function;

template <>
InputParameters validParams<UserForcingFunctionNodalKernel>();

/**
 * Represents the rate in a simple ODE of du/dt = f
 */
class UserForcingFunctionNodalKernel : public NodalKernel
{
public:
  /**
   * Constructor grabs the Function
   */
  UserForcingFunctionNodalKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  Function & _func;
};

#endif
