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
#ifndef DONOTCOPYPARAMETESKERNEL_H
#define DONOTCOPYPARAMETESKERNEL_H

#include "Kernel.h"

// Forward Declarations
class DoNotCopyParametersKernel;

template <>
InputParameters validParams<DoNotCopyParametersKernel>();

class DoNotCopyParametersKernel : public Kernel
{
public:
  // This is the wrong constructor, don't to this!
  DoNotCopyParametersKernel(InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif // DONOTCOPYPARAMETESKERNEL_H
