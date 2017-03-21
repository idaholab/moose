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
#ifndef NANATCOUNTKERNEL_H
#define NANATCOUNTKERNEL_H

#include "Kernel.h"

// Forward Declaration
class NanAtCountKernel;

template <>
InputParameters validParams<NanAtCountKernel>();

/**
 * Kernel that generates NaN
 */
class NanAtCountKernel : public Kernel
{
public:
  NanAtCountKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

private:
  /// The residual count to nan at
  unsigned int _count_to_nan;

  /// Whether or not to print out the count
  bool _print_count;

  /// The current count
  unsigned int _count;
};

#endif // NANATCOUNTKERNEL_H
