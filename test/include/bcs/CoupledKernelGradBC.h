//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COUPLEDKERNELGRADBC_H
#define COUPLEDKERNELGRADBC_H

#include "IntegratedBC.h"

class CoupledKernelGradBC;
class Function;

template <>
InputParameters validParams<CoupledKernelGradBC>();

class CoupledKernelGradBC : public IntegratedBC
{
public:
  CoupledKernelGradBC(const InputParameters & parameters);

  virtual ~CoupledKernelGradBC();

protected:
  virtual Real computeQpResidual();

  RealVectorValue _beta;
  const VariableValue & _var2;
};

#endif /* COUPLEDKERNELGRADBC_H */
