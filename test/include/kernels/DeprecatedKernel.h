//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DEPRECATEDKERNEL_H
#define DEPRECATEDKERNEL_H

#include "Reaction.h"

// Forward Declarations
class DeprecatedKernel;

template <>
InputParameters validParams<DeprecatedKernel>();

class DeprecatedKernel : public Reaction
{
public:
  DeprecatedKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _coef;
};

#endif // DEPRECATEDKERNEL_H
