//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StochasticMatrix.h"

registerMooseObject("StochasticToolsApp", StochasticMatrix);

InputParameters
StochasticMatrix::validParams()
{
  InputParameters params = StochasticReporter::validParams();
  params.addClassDescription(
      "Tool for extracting Sampler object data and storing data from stochastic simulations.");
  params.addRequiredParam<SamplerName>("sampler",
                                       "The sample from which to extract distribution data.");
  params.addParam<std::vector<ReporterValueName>>(
      "sampler_column_names",
      "Prescribed names of sampler columns, used to assign names of outputted vectors.");
  return params;
}

StochasticMatrix::StochasticMatrix(const InputParameters & parameters)
  : StochasticReporter(parameters), _sampler(getSampler("sampler"))
{
  std::vector<ReporterValueName> names;
  if (isParamValid("sampler_column_names"))
    names = getParam<std::vector<ReporterValueName>>("sampler_column_names");
  else
  {
    names.resize(_sampler.getNumberOfCols());
    const int padding = MooseUtils::numDigits(_sampler.getNumberOfCols());
    for (dof_id_type j = 0; j < _sampler.getNumberOfCols(); ++j)
    {
      std::stringstream nm;
      nm << getParam<SamplerName>("sampler") << "_" << std::setw(padding) << std::setfill('0') << j;
      names[j] = nm.str();
    }
  }

  if (names.size() != _sampler.getNumberOfCols())
    paramError("sampler_column_names",
               "The number of column names specified (",
               names.size(),
               ") does not match the number of sampler columns (",
               _sampler.getNumberOfCols(),
               ").");

  for (const auto & nm : names)
    _sample_vectors.push_back(&declareStochasticReporter<Real>(nm, _sampler));
}

void
StochasticMatrix::execute()
{
  for (dof_id_type i = 0; i < _sampler.getNumberOfLocalRows(); ++i)
  {
    std::vector<Real> data = _sampler.getNextLocalRow();
    for (std::size_t j = 0; j < data.size(); ++j)
      (*_sample_vectors[j])[i] = data[j];
  }
}

ReporterName
StochasticMatrix::declareStochasticReporterClone(const Sampler & sampler,
                                                 const ReporterData & from_data,
                                                 const ReporterName & from_reporter,
                                                 std::string prefix)
{
  if (sampler.name() != _sampler.name())
    paramError("sampler",
               "Attempting to create a stochastic vector with a different sampler (",
               sampler.name(),
               ") than the one specified at input (",
               _sampler.name(),
               ").");

  return StochasticReporter::declareStochasticReporterClone(
      sampler, from_data, from_reporter, prefix);
}
