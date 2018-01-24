//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ROBINBC_H
#define ROBINBC_H

#include "IntegratedBC.h"

class RobinBC;

template <>
InputParameters validParams<RobinBC>();

class RobinBC : public IntegratedBC
{
public:
  RobinBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif // ROBINBC_H
