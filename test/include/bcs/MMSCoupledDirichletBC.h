//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalBC.h"

class MMSCoupledDirichletBC : public NodalBC
{
public:
  static InputParameters validParams();

  MMSCoupledDirichletBC(const InputParameters & parameters);
  virtual ~MMSCoupledDirichletBC() {}

protected:
  virtual Real computeQpResidual();

  Real _value; // Multiplier on the boundary
  unsigned int _mesh_dimension;
};
