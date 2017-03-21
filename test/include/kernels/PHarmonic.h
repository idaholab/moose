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
#ifndef PHARMONIC_H
#define PHARMONIC_H

#include "Kernel.h"

// Forward Declarations
class PHarmonic;

template <>
InputParameters validParams<PHarmonic>();

/**
 * This kernel implements (grad(v), |grad(u)|^(p-2) grad(u)), where u is the solution
 * and v is the test function. When p=2, this kernel is equivalent with Diffusion.
 */

class PHarmonic : public Kernel
{
public:
  PHarmonic(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _p;
};

#endif // PHARMONIC_H
