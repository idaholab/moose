//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FDDIFFUSION_H
#define FDDIFFUSION_H

#include "FDKernel.h"

class FDDiffusion;

template <>
InputParameters validParams<FDDiffusion>();

class FDDiffusion : public FDKernel
{
public:
  FDDiffusion(const InputParameters & parameters);
  virtual ~FDDiffusion();

protected:
  virtual Real computeQpResidual();
};

#endif /* FDDIFFUSION_H */
