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

template<>
InputParameters validParams<DotCouplingKernel>();

/**
 * Kernel that is calling coupledDot
 */
class DotCouplingKernel : public Kernel
{
public:
  DotCouplingKernel(const std::string & name, InputParameters parameters);
  virtual ~DotCouplingKernel();

protected:
  virtual Real computeQpResidual();

  VariableValue & _v_dot;
};


#endif /* DOTCOUPLINGKERNEL_H */
