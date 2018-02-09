//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RENAMEDKERNEL_H
#define RENAMEDKERNEL_H

#include "Reaction.h"

// Forward Declarations
class RenamedKernel;

template <>
InputParameters validParams<RenamedKernel>();

class RenamedKernel : public Reaction
{
public:
  RenamedKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _coef;
};

#endif // RENAMEDKERNEL_H
