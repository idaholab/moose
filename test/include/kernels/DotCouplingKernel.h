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

#ifndef DOTCOUPLINGKERNEL_H
#define DOTCOUPLINGKERNEL_H

#include "Kernel.h"

class DotCouplingKernel;

template <>
InputParameters validParams<DotCouplingKernel>();

/**
 * Kernel that is calling coupledDot
 */
class DotCouplingKernel : public Kernel
{
public:
  DotCouplingKernel(const InputParameters & parameters);
  virtual ~DotCouplingKernel(){};

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const VariableValue & _v_dot;
  const VariableValue & _dv_dot_dv;
};

#endif /* DOTCOUPLINGKERNEL_H */
