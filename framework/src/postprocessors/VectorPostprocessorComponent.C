//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorComponent.h"
#include "VectorPostprocessorInterface.h"

registerMooseObject("MooseApp", VectorPostprocessorComponent);

InputParameters
VectorPostprocessorComponent::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addRequiredParam<VectorPostprocessorName>(
      "vectorpostprocessor", "The vectorpostprocessor from which a value is extracted");
  params.addRequiredParam<std::string>("vector_name",
                                       "Name of the vector for which to report a value");
  params.addRequiredParam<unsigned int>(
      "index", "Index of the vectorpostprocessor for which to report a value");
  params.addClassDescription(
      "Returns the value of the specified component of a VectorPostprocessor");

  return params;
}

VectorPostprocessorComponent::VectorPostprocessorComponent(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _vpp_name(getParam<VectorPostprocessorName>("vectorpostprocessor")),
    _vector_name(getParam<std::string>("vector_name")),
    _vpp_values(getVectorPostprocessorValue("vectorpostprocessor", _vector_name)),
    _vpp_index(getParam<unsigned int>("index"))
{
}

Real
VectorPostprocessorComponent::getValue()
{
  if (_vpp_index >= _vpp_values.size())
    mooseError("In VectorPostprocessorComponent index greater than size of vector");
  return _vpp_values[_vpp_index];
}
