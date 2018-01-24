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
#ifndef PMASSKERNEL_H
#define PMASSKERNEL_H

#include "Kernel.h"

// Forward Declarations
class PMassKernel;

template <>
InputParameters validParams<PMassKernel>();

/**
 * This kernel implements (v, |u|^(p-2) u)/k, where u is the variable, v is the test function
 * and k is the eigenvalue. When p=2, this kernel is equivalent with MassEigenKernel.
 */
class PMassKernel : public Kernel
{
public:
  PMassKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _p;
};

#endif // PMASSKERNEL_H
