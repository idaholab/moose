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

#include "NullKernel.h"

template<>
InputParameters validParams<NullKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Kernel that sets a zero residual.");
  return params;
}

NullKernel::NullKernel(const InputParameters & parameters) :
    Kernel(parameters)
{
}

Real
NullKernel::computeQpResidual()
{
  return 0.0;
}

Real
NullKernel::computeQpJacobian()
{
  return 1.0;
}
