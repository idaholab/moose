//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GENERICDIFFUSION_H
#define GENERICDIFFUSION_H

#include "Kernel.h"

// Forward Declarations
class GenericDiffusion;

template <>
InputParameters validParams<GenericDiffusion>();

class GenericDiffusion : public Kernel
{
public:
  GenericDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const MaterialProperty<Real> & _diffusivity;
};

#endif // GENERICDIFFUSION_H
