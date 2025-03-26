//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADNodalKernel.h"
#include "MooseFunctorForward.h"

/**
 * Represents the rate in a simple ODE of du/dt = f
 */
class UserForcingFunctorNodalKernel : public ADNodalKernel
{
public:
  /**
   * Constructor grabs the Function
   */
  static InputParameters validParams();

  UserForcingFunctorNodalKernel(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  const Moose::Functor<ADReal> & _functor;
};
