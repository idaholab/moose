//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NULLKERNEL_H
#define NULLKERNEL_H

#include "Kernel.h"

class NullKernel;

template <>
InputParameters validParams<NullKernel>();

/**
 *
 */
class NullKernel : public Kernel
{
public:
  NullKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// filler value to put on the on-diagonal Jacobian
  const Real _jacobian_fill;
};

#endif // NULLKERNEL_H
