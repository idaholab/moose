//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterVectorSum.h"

registerMooseObject("OptimizationApp", ReporterVectorSum);

InputParameters
ReporterVectorSum::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Sum a vector of vectors returned by a StochasticReporter into a "
                             "single vector.");

  params.addParam<std::string>(
      "summed_vector_name",
      "summed_vector_name",
      "Name of reporter that will contain the sum of StochasticReporter vectors.");
  params.addRequiredParam<ReporterName>(
      "reporter_vector_of_vectors", "Name of StochasticReporter vector of vectors to be summed");
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

ReporterVectorSum::ReporterVectorSum(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _summed_data(
        declareValueByName<std::vector<Real>>("summed_vector_name", REPORTER_MODE_REPLICATED))
{
}

void
ReporterVectorSum::initialSetup()
{
  ReporterName rname = getParam<ReporterName>("reporter_vector_of_vectors");
  _reporter_data =
      &getReporterValueByName<std::vector<std::vector<Real>>>(rname, REPORTER_MODE_REPLICATED);
}

void
ReporterVectorSum::execute()
{
  _summed_data.clear();
  std::size_t entries(_reporter_data->at(0).size());
  _summed_data.resize(entries, 0.);
  for (auto & reporter_vector : *_reporter_data)
    for (std::size_t i = 0; i < entries; ++i)
      _summed_data[i] += reporter_vector[i];
}
