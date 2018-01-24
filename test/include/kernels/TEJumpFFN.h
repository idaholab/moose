//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TEJUMPFFN_H
#define TEJUMPFFN_H

#include "Kernel.h"

class TEJumpFFN;

template <>
InputParameters validParams<TEJumpFFN>();

class TEJumpFFN : public Kernel
{
public:
  TEJumpFFN(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  Real _t_jump;
  Real _slope;
};

#endif // TEJUMPFFN_H
