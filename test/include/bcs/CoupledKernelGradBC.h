/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
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
