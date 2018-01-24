//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PHARMONIC_H
#define PHARMONIC_H

#include "Kernel.h"

// Forward Declarations
class PHarmonic;

template <>
InputParameters validParams<PHarmonic>();

/**
 * This kernel implements (grad(v), |grad(u)|^(p-2) grad(u)), where u is the solution
 * and v is the test function. When p=2, this kernel is equivalent with Diffusion.
 */

class PHarmonic : public Kernel
{
public:
  PHarmonic(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _p;
};

#endif // PHARMONIC_H
