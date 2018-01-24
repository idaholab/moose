//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EXAMPLECOEFDIFFUSION_H
#define EXAMPLECOEFDIFFUSION_H

#include "Kernel.h"

// Forward Declarations
class ExampleCoefDiffusion;

template <>
InputParameters validParams<ExampleCoefDiffusion>();

class ExampleCoefDiffusion : public Kernel
{
public:
  ExampleCoefDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

private:
  Real _coef;
};
#endif // EXAMPLECOEFDIFFUSION_H
