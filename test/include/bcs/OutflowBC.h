//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef OUTFLOWBC_H
#define OUTFLOWBC_H

#include "IntegratedBC.h"

class OutflowBC;

template <>
InputParameters validParams<OutflowBC>();

class OutflowBC : public IntegratedBC
{
public:
  OutflowBC(const InputParameters & parameters);

protected:
  RealVectorValue _velocity;
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif // OUTFLOWBC_H
