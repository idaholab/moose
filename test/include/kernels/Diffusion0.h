//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DIFFUSION0_H
#define DIFFUSION0_H

#include "Kernel.h"
#include "Material.h"

// Forward Declarations
class Diffusion0;

template <>
InputParameters validParams<Diffusion0>();

class Diffusion0 : public Kernel
{
public:
  Diffusion0(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  /// Parameters for spatially linearly varying diffusivity.
  Real _Ak, _Bk, _Ck;
};

#endif // DIFFUSION0_H
