//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorReceiver.h"

registerMooseObject("MooseApp", VectorPostprocessorReceiver);

defineLegacyParams(VectorPostprocessorReceiver);

InputParameters
VectorPostprocessorReceiver::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription(
      "Reports the value stored in this processor, which is usually filled "
      "in by another object. The VectorPostprocessorReceiver does not compute its own value.");
  return params;
}

VectorPostprocessorReceiver::VectorPostprocessorReceiver(const InputParameters & params)
  : GeneralVectorPostprocessor(params)
{
}

VectorPostprocessorValue &
VectorPostprocessorReceiver::addVector(std::string name)
{
  _name_vector_map[name] = &declareVector(name);
  return *_name_vector_map.at(name);
}
