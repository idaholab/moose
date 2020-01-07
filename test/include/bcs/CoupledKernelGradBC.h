//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

class Function;

class CoupledKernelGradBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  CoupledKernelGradBC(const InputParameters & parameters);

  virtual ~CoupledKernelGradBC();

protected:
  virtual Real computeQpResidual();

  RealVectorValue _beta;
  const VariableValue & _var2;
};
