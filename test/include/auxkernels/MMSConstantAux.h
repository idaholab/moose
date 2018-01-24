//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MMSCONSTANTAUX_H
#define MMSCONSTANTAUX_H

#include "AuxKernel.h"

class MMSConstantAux;

template <>
InputParameters validParams<MMSConstantAux>();

class MMSConstantAux : public AuxKernel
{
public:
  MMSConstantAux(const InputParameters & parameters);

  virtual ~MMSConstantAux() {}

protected:
  virtual Real computeValue();

  unsigned int _mesh_dimension;
};

#endif // MMSCONSTANTAUX_H
