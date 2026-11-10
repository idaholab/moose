//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADScalarKernel.h"

/**
 * Test ADScalarKernel that subtracts a functor from the residual, demonstrating the use of
 * functors in a scalar kernel. A scalar variable has no spatial location, so the functor is
 * evaluated at a point given by the user.
 */
class ADFunctorScalarTestKernel : public ADScalarKernel
{
public:
  static InputParameters validParams();

  ADFunctorScalarTestKernel(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// The functor to subtract from the residual
  const Moose::Functor<ADReal> & _functor;

  /// The point at which to evaluate the functor
  const Point _point;
};
