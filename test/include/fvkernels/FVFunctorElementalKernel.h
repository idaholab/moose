//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

class FVFunctorElementalKernel : public FVElementalKernel
{
public:
  static InputParameters validParams();
  FVFunctorElementalKernel(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// The functor that provides the residual for this kernel
  const Moose::Functor<ADReal> & _functor;
};
