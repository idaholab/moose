//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PREACTION_H
#define PREACTION_H

#include "Kernel.h"

class PReaction;

template <>
InputParameters validParams<PReaction>();

class PReaction : public Kernel
{
public:
  PReaction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _coef;
  Real _p;
};

#endif // PREACTION_H
