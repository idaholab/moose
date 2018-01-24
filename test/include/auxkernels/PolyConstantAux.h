//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POLYCONSTANTAUX_H
#define POLYCONSTANTAUX_H

#include "AuxKernel.h"

class PolyConstantAux;

template <>
InputParameters validParams<PolyConstantAux>();

class PolyConstantAux : public AuxKernel
{
public:
  PolyConstantAux(const InputParameters & parameters);

  virtual ~PolyConstantAux() {}

protected:
  virtual Real computeValue();
};

#endif // POLYCONSTANTAUX_H
