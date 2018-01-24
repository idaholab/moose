//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MMSREACTION_H_
#define MMSREACTION_H_

#include "Kernel.h"

class MMSReaction;

template <>
InputParameters validParams<MMSReaction>();

class MMSReaction : public Kernel
{
public:
  MMSReaction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  unsigned int _mesh_dimension;
};

#endif // MMSREACTION_H_
