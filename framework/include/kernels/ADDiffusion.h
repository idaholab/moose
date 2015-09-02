//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDIFFUSION_H
#define ADDIFFUSION_H

#include "ADKernel.h"

class ADDiffusion;

template <>
InputParameters validParams<ADDiffusion>();

class ADDiffusion : public ADKernel
{
public:
  ADDiffusion(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();
};

#endif /* ADDIFFUSION_H */
