//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorOfVectorTestReporter.h"

registerMooseObject("OptimizationTestApp", VectorOfVectorTestReporter);

InputParameters
VectorOfVectorTestReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "Test Reporter to create vector of vector for VectorOfVectorRowSum testing.");
  params.addRequiredParam<std::string>("name", "Reporter name to create.");
  params.addRequiredParam<std::vector<std::vector<Real>>>(
      "vector_of_vectors", "Data to put into vector of vector reporter.");
  return params;
}

VectorOfVectorTestReporter::VectorOfVectorTestReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _vectors(declareValueByName<std::vector<std::vector<Real>>>(getParam<std::string>("name"),
                                                                REPORTER_MODE_REPLICATED))
{
  _vectors = getParam<std::vector<std::vector<Real>>>("vector_of_vectors");
}
