//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorAux.h"

registerMooseObject("MooseTestApp", VectorPostprocessorAux);

InputParameters
VectorPostprocessorAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addRequiredParam<VectorPostprocessorName>("vpp",
                                                   "The VectorPostprocessor to pull values out of");
  params.addRequiredParam<std::string>("vector", "The vector to use from the VectorPostprocessor");
  params.addRequiredParam<unsigned int>("index", "The entry in the VectorPostprocessor to use");

  return params;
}

VectorPostprocessorAux::VectorPostprocessorAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _vpp(getVectorPostprocessorValue("vpp", getParam<std::string>("vector"), true)),
    _index(getParam<unsigned int>("index"))
{
}

Real
VectorPostprocessorAux::computeValue()
{
  return _vpp[_index];
}
