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
#ifndef PMASSEIGENKERNEL_H
#define PMASSEIGENKERNEL_H

#include "EigenKernel.h"

// Forward Declarations
class PMassEigenKernel;

template <>
InputParameters validParams<PMassEigenKernel>();

/**
 * This kernel implements (v, |u|^(p-2) u)/k, where u is the variable, v is the test function
 * and k is the eigenvalue. When p=2, this kernel is equivalent with MassEigenKernel.
 */

class PMassEigenKernel : public EigenKernel
{
public:
  PMassEigenKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _p;
};

#endif // PMASSEIGENKERNEL_H
