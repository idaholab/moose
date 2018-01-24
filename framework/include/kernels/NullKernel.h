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

#ifndef NULLKERNEL_H
#define NULLKERNEL_H

#include "Kernel.h"

class NullKernel;

template <>
InputParameters validParams<NullKernel>();

/**
 *
 */
class NullKernel : public Kernel
{
public:
  NullKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// filler value to put on the on-diagonal Jacobian
  const Real _jacobian_fill;
};

#endif // NULLKERNEL_H
