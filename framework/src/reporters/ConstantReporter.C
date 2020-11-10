//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantReporter.h"

registerMooseObject("MooseApp", ConstantReporter);

InputParameters
ConstantReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reporter with constant values to be accessed by other objects, can "
                             "be modified using transfers.");

  // Scalar data
  params.addParam<std::vector<ReporterValueName>>("integer_names", "Names for each integer value.");
  params.addParam<std::vector<int>>("integer_values", "Values for integers.");

  params.addParam<std::vector<ReporterValueName>>("real_names", "Names for each real value.");
  params.addParam<std::vector<Real>>("real_values", "Values for reals.");

  params.addParam<std::vector<ReporterValueName>>("string_names", "Names for each string value.");
  params.addParam<std::vector<std::string>>("string_values", "Values for strings.");

  // Vector data
  params.addParam<std::vector<ReporterValueName>>("integer_vector_names",
                                                  "Names for each vector of integers value.");
  params.addParam<std::vector<std::vector<int>>>("integer_vector_values",
                                                 "Values for vectors of integers.");

  params.addParam<std::vector<ReporterValueName>>("real_vector_names",
                                                  "Names for each vector of reals value.");
  params.addParam<std::vector<std::vector<Real>>>("real_vector_values",
                                                  "Values for vectors of reals.");

  params.addParam<std::vector<ReporterValueName>>("string_vector_names",
                                                  "Names for each vector of strings value.");
  params.addParam<std::vector<std::vector<std::string>>>("string_vector_values",
                                                         "Values for vectors of strings.");

  return params;
}

ConstantReporter::ConstantReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _int(declareConstantReporterValues<int>("integer")),
    _real(declareConstantReporterValues<Real>("real")),
    _string(declareConstantReporterValues<std::string>("string")),
    _int_vec(declareConstantVectorReporterValues<int>("integer")),
    _real_vec(declareConstantVectorReporterValues<Real>("real")),
    _string_vec(declareConstantVectorReporterValues<std::string>("string"))
{
}
