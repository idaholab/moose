//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MMSCOUPLEDDIRICHLETBC_H_
#define MMSCOUPLEDDIRICHLETBC_H_

#include "NodalBC.h"

class MMSCoupledDirichletBC;

template <>
InputParameters validParams<MMSCoupledDirichletBC>();

class MMSCoupledDirichletBC : public NodalBC
{
public:
  MMSCoupledDirichletBC(const InputParameters & parameters);
  virtual ~MMSCoupledDirichletBC() {}

protected:
  virtual Real computeQpResidual();

  Real _value; // Multiplier on the boundary
  unsigned int _mesh_dimension;
};

#endif // MMSCOUPLEDDIRICHLETBC_H_
