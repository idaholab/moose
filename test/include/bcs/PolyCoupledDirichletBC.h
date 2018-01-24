//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POLYCOUPLEDDIRICHLETBC_H
#define POLYCOUPLEDDIRICHLETBC_H

#include "NodalBC.h"

class PolyCoupledDirichletBC;

template <>
InputParameters validParams<PolyCoupledDirichletBC>();

class PolyCoupledDirichletBC : public NodalBC
{
public:
  PolyCoupledDirichletBC(const InputParameters & parameters);

  virtual ~PolyCoupledDirichletBC() {}

protected:
  virtual Real computeQpResidual();

  Real _value; // Multiplier on the boundary
};

#endif // POLYCOUPLEDDIRICHLETBC_H
