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

#include "ODETimeKernel.h"
#include "SystemBase.h"
#include "Assembly.h"

template <>
InputParameters
validParams<ODETimeKernel>()
{
  InputParameters params = validParams<ODEKernel>();
  return params;
}

ODETimeKernel::ODETimeKernel(const InputParameters & parameters) : ODEKernel(parameters) {}

void
ODETimeKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number(), Moose::KT_TIME);
  for (_i = 0; _i < _var.order(); _i++)
    re(_i) += computeQpResidual();
}
