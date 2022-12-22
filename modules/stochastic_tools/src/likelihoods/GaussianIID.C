//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GaussianIID.h"
#include "DelimitedFileReader.h"
#include "Normal.h"

registerMooseObject("StochasticToolsApp", GaussianIID);

InputParameters
GaussianIID::validParams()
{
  InputParameters params = Likelihood::validParams();
  params.addClassDescription(
      "GaussianIID likelihood function evaluating the model goodness against experiments.");
  params.addParam<bool>("log_likelihood", true, "Compute log-likelihood or likelihood.");
  params.addRequiredParam<Real>("noise",
                                "Experimental noise plus model deviations against experiments.");
  params.addParam<FileName>("file_name", "Name of the CSV file with experimental values.");
  params.addParam<std::string>(
      "file_column_name", "Name of column in CSV file to use, by default first column is used.");
  params.addParam<std::vector<Real>>(
      "exp_values", "User-specified experimental values when CSV file is not provided.");
  return params;
}

GaussianIID::GaussianIID(const InputParameters & parameters)
  : Likelihood(parameters),
    ReporterInterface(this),
    _log_likelihood(getParam<bool>("log_likelihood")),
    _noise(getParam<Real>("noise"))
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
GaussianIID::reqFunction(const std::vector<Real> & exp,
                         const std::vector<std::vector<Real>> & model,
                         const std::vector<Real> & weights,
                         const Real & noise,
                         const bool & log_likelihood) const
{
  Real result = 0.0;
  for (unsigned i = 0; i < exp.size(); ++i)
  {
    Real tmp = 0.0;
    for (unsigned j = 0; j < model.size(); ++j)
      tmp += weights[j] * Normal::pdf(exp[i], model[j][i], noise);
    result += std::log(tmp);
  }

  result = log_likelihood ? result : std::exp(result);
  return result;
}

Real
GaussianIID::function(const std::vector<std::vector<Real>> & x, const std::vector<Real> & w) const
{
  if (x.size() != w.size())
    mooseError("The number of models should be equal to the number of model weights.");

  if (x[0].size() != _exp_values.size())
    mooseError("The number of model evaluations should be equal to the number of experimental data "
               "points.");

  return reqFunction(_exp_values, x, w, _noise, _log_likelihood);
}
