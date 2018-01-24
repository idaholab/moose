//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PPSDIFFUSION_H
#define PPSDIFFUSION_H

#include "Kernel.h"

// Forward Declaration
class PPSDiffusion;

template <>
InputParameters validParams<PPSDiffusion>();

class PPSDiffusion : public Kernel
{
public:
  PPSDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  const PostprocessorValue & _pps_value;
};

#endif // PPSDIFFUSION_H
