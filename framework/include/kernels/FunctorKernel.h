//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 * Adds a term from a functor.
 */
class FunctorKernel : public ADKernelValue
{
public:
  static InputParameters validParams();

  FunctorKernel(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

  /// Functor to add
  const Moose::Functor<ADReal> & _functor;
  /// Sign to apply to functor
  const Real _sign;
};
