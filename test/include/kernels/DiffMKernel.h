//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DIFFMKERNEL_H
#define DIFFMKERNEL_H

#include "Kernel.h"
#include "MaterialProperty.h"

// Forward Declaration
class DiffMKernel;

template <>
InputParameters validParams<DiffMKernel>();

class DiffMKernel : public Kernel
{
public:
  DiffMKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const MaterialProperty<Real> & _diff;
  Real _offset;
};
#endif // DIFFMKERNEL_H
