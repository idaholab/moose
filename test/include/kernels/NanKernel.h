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
#ifndef NANKERNEL_H
#define NANKERNEL_H

#include "Kernel.h"

// Forward Declaration
class NanKernel;

template <>
InputParameters validParams<NanKernel>();

/**
 * Kernel that generates NaN
 */
class NanKernel : public Kernel
{
public:
  NanKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  unsigned int _timestep_to_nan;

  unsigned int _deprecated_default;
  unsigned int _deprecated_no_default;
};

#endif // NANKERNEL_H
