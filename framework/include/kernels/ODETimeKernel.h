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

#ifndef ODETIMEKERNEL_H
#define ODETIMEKERNEL_H

#include "ODEKernel.h"

// Forward Declarations
class ODETimeKernel;

template <>
InputParameters validParams<ODETimeKernel>();

/**
 * Base class for ODEKernels that contribute to the time residual
 * vector.
 */
class ODETimeKernel : public ODEKernel
{
public:
  ODETimeKernel(const InputParameters & parameters);

  virtual void computeResidual() override;
};

#endif
