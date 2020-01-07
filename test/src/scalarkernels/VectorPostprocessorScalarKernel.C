//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorScalarKernel.h"

registerMooseObject("MooseTestApp", VectorPostprocessorScalarKernel);

InputParameters
VectorPostprocessorScalarKernel::validParams()
{
  InputParameters params = ODEKernel::validParams();

  params.addRequiredParam<VectorPostprocessorName>("vpp",
                                                   "VectorPostprocessor to pull the values out of");
  params.addRequiredParam<std::string>("vector",
                                       "The vector to use from the VectorPostprocessor.  "
                                       "This vector MUST be the same size as the scalar "
                                       "variable's ORDER.");

  return params;
}

VectorPostprocessorScalarKernel::VectorPostprocessorScalarKernel(const InputParameters & parameters)
  : ODEKernel(parameters),
    _vpp(getVectorPostprocessorValue("vpp", getParam<std::string>("vector"), true))
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
