//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Linear.h"
#include "DelimitedFileReader.h"
#include "libmesh/utility.h"

registerMooseObject("StochasticToolsApp", Linear);

InputParameters
Linear::validParams()
{
  InputParameters params = LikelihoodFunctionBase::validParams();
  params.addClassDescription(
      "Linear function evaluating the model goodness against experiments.");
  params.addRequiredParam<ReporterName>(
      "noise", "Experimental noise plus model deviations against experiments.");
  params.addParam<FileName>("file_name", "Name of the CSV file with experimental values.");
  params.addParam<std::string>(
      "file_column_name", "Name of column in CSV file to use, by default first column is used.");
  params.addParam<std::vector<Real>>(
      "exp_values", "User-specified experimental values when CSV file is not provided.");
  return params;
}

Linear::Linear(const InputParameters & parameters)
  : LikelihoodFunctionBase(parameters),
    ReporterInterface(this),
    _noise(getReporterValue<Real>("noise"))
{
  if (isParamValid("exp_values") && isParamValid("file_name"))
    paramError("exp_values", "exp_values and file_name both cannot be set at the same time.");
  else if (isParamValid("file_name"))
  {
    MooseUtils::DelimitedFileReader reader(getParam<FileName>("file_name"));
    reader.read();
    if (isParamValid("file_column_name"))
      _exp_values = reader.getData(getParam<std::string>("file_column_name"));
    else
      _exp_values = reader.getData(0);
  }
  else if (isParamValid("exp_values"))
    _exp_values = getParam<std::vector<Real>>("exp_values");
  else
    mooseError("Either 'exp_values' or 'file_name' parameters must be specified to represent "
               "experimental data.");
}

Real
Linear::function(const std::vector<Real> & exp,
                   const std::vector<Real> & model,
                   const Real & noise,
                   const bool & /*log_likelihood*/)
{
  Real result = 0.0;
  Real val1;
  for (unsigned i = 0; i < exp.size(); ++i)
  {
    val1 = Utility::pow<2>((exp[i] - model[i]) / noise);
    result += val1;
  }
  return result;
}

Real
Linear::function(const std::vector<Real> & x) const
{
  return function(_exp_values, x, _noise, false);
}
