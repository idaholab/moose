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
#include "DoNotCopyParametersKernel.h"

template <>
InputParameters
validParams<DoNotCopyParametersKernel>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

// This is the wrong constructor, don't to this!
DoNotCopyParametersKernel::DoNotCopyParametersKernel(InputParameters parameters)
  : Kernel(parameters)
{
}

Real
DoNotCopyParametersKernel::computeQpResidual()
{
  getParam<std::string>("name"); // This will cause a segmentation fault
  return 0.0;
}

Real
DoNotCopyParametersKernel::computeQpJacobian()
{
  return 0.0;
}
