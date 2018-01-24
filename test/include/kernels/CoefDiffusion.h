//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COEFDIFFUSION_H
#define COEFDIFFUSION_H

#include "Kernel.h"

// Forward Declarations
class CoefDiffusion;

template <>
InputParameters validParams<CoefDiffusion>();

class CoefDiffusion : public Kernel
{
public:
  CoefDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const Real & _coef;
};

#endif // COEFDIFFUSION_H
