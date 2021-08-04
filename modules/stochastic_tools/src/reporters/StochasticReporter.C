//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "StochasticReporter.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", StochasticReporter);

InputParameters
StochasticReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "Storage container for stochastic simulation results coming from Reporters.");
  MooseEnum parallel_type("DISTRIBUTED=0 ROOT=1", "DISTRIBUTED");
  params.addParam<MooseEnum>(
      "parallel_type",
      parallel_type,
      "This parameter will determine how the stochastic data is gathered. It is common for "
      "outputting purposes that this parameter be set to ROOT, otherwise, many files will be "
      "produced showing the values on each processor. However, if there are lot of samples, "
      "gathering on root may be memory restrictive.");
  return params;
}

StochasticReporter::StochasticReporter(const InputParameters & parameters)
  : GeneralReporter(parameters), _parallel_type(getParam<MooseEnum>("parallel_type"))
{
}

ReporterName
StochasticReporter::declareStochasticReporterClone(const Sampler & sampler,
                                                   const ReporterData & from_data,
                                                   const ReporterName & from_reporter,
                                                   std::string prefix)
{
  std::string value_name = (prefix.empty() ? "" : prefix + ":") + from_reporter.getObjectName() +
                           ":" + from_reporter.getValueName();

  if (!from_data.hasReporterValue(from_reporter))
    mooseError("Reporter value ", from_reporter, " has not been declared.");

  // Single quantities
  if (from_data.hasReporterValue<bool>(from_reporter))
    declareStochasticReporter<bool>(value_name, sampler);
  else if (from_data.hasReporterValue<int>(from_reporter))
    declareStochasticReporter<int>(value_name, sampler);
  else if (from_data.hasReporterValue<Real>(from_reporter))
    declareStochasticReporter<Real>(value_name, sampler);
  else if (from_data.hasReporterValue<std::string>(from_reporter))
    declareStochasticReporter<std::string>(value_name, sampler);
  // Vector quantities
  else if (from_data.hasReporterValue<std::vector<bool>>(from_reporter))
    declareStochasticReporter<std::vector<bool>>(value_name, sampler);
  else if (from_data.hasReporterValue<std::vector<int>>(from_reporter))
    declareStochasticReporter<std::vector<int>>(value_name, sampler);
  else if (from_data.hasReporterValue<std::vector<Real>>(from_reporter))
    declareStochasticReporter<std::vector<Real>>(value_name, sampler);
  else if (from_data.hasReporterValue<std::vector<std::string>>(from_reporter))
    declareStochasticReporter<std::vector<std::string>>(value_name, sampler);
  // Vector of vector quantities
  else if (from_data.hasReporterValue<std::vector<std::vector<bool>>>(from_reporter))
    declareStochasticReporter<std::vector<std::vector<bool>>>(value_name, sampler);
  else if (from_data.hasReporterValue<std::vector<std::vector<int>>>(from_reporter))
    declareStochasticReporter<std::vector<std::vector<int>>>(value_name, sampler);
  else if (from_data.hasReporterValue<std::vector<std::vector<Real>>>(from_reporter))
    declareStochasticReporter<std::vector<std::vector<Real>>>(value_name, sampler);
  else if (from_data.hasReporterValue<std::vector<std::vector<std::string>>>(from_reporter))
    declareStochasticReporter<std::vector<std::vector<std::string>>>(value_name, sampler);
  else
    return {}; // Empty ReporterName to show that reporter value is unsupported type

  return {name(), value_name};
}
