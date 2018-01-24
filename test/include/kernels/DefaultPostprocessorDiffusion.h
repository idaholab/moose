//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DEFAULTPOSTPROCESSORDIFFUSION_H
#define DEFAULTPOSTPROCESSORDIFFUSION_H

#include "Kernel.h"

// Forward Declaration
class DefaultPostprocessorDiffusion;

template <>
InputParameters validParams<DefaultPostprocessorDiffusion>();

class DefaultPostprocessorDiffusion : public Kernel
{
public:
  DefaultPostprocessorDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  const PostprocessorValue & _pps_value;
};

#endif // DEFAULTPOSTPROCESSORDIFFUSION_H
