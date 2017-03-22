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

#include "VectorPostprocessorScalarKernel.h"

template <>
InputParameters
validParams<VectorPostprocessorScalarKernel>()
{
  InputParameters params = validParams<ODEKernel>();

  params.addRequiredParam<VectorPostprocessorName>("vpp",
                                                   "VectorPostprocessor to pull the values out of");
  params.addRequiredParam<std::string>("vector",
                                       "The vector to use from the VectorPostprocessor.  "
                                       "This vector MUST be the same size as the scalar "
                                       "variable's ORDER.");

  return params;
}

VectorPostprocessorScalarKernel::VectorPostprocessorScalarKernel(const InputParameters & parameters)
  : ODEKernel(parameters), _vpp(getVectorPostprocessorValue("vpp", getParam<std::string>("vector")))
{
}

VectorPostprocessorScalarKernel::~VectorPostprocessorScalarKernel() {}

Real
VectorPostprocessorScalarKernel::computeQpResidual()
{
  return _u[_i] - _vpp[_i];
}

Real
VectorPostprocessorScalarKernel::computeQpJacobian()
{
  return 1.;
}
