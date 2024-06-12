//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GaussianVector.h"
#include "Normal.h"
#include "DelimitedFileReader.h"

registerMooseObject("StochasticToolsApp", GaussianVector);

InputParameters
GaussianVector::validParams()
{
  InputParameters params = LikelihoodFunctionBaseVector::validParams();
  params.addClassDescription(
      "Gaussian likelihood function evaluating the model goodness against experiments.");
  params.addParam<bool>("log_likelihood", true, "Compute log-likelihood or likelihood.");
  params.addRequiredParam<ReporterName>(
      "noise", "Experimental noise plus model deviations against experiments.");
  params.addParam<FileName>("file_name", "Name of the CSV file with experimental values.");
  params.addParam<std::string>(
      "file_column_name", "Name of column in CSV file to use, by default first column is used.");
  params.addParam<std::vector<Real>>(
      "exp_values", "User-specified experimental values when CSV file is not provided.");
  return params;
}

GaussianVector::GaussianVector(const InputParameters & parameters)
  : LikelihoodFunctionBaseVector(parameters),
    ReporterInterface(this),
    _log_likelihood(getParam<bool>("log_likelihood")),
    _noise(getReporterValue<Real>("noise"))
{
  if (isParamValid("exp_values") && isParamValid("file_name"))
    paramError("exp_values", "exp_values and file_name both cannot be set at the same time.");
  else if (isParamValid("file_name"))
  {
    MooseUtils::DelimitedFileReader reader(getParam<FileName>("file_name"));
    reader.read();
    if (isParamValid("file_column_name"))
      _exp_values.push_back(reader.getData(getParam<std::string>("file_column_name")));
    else
      _exp_values.push_back(reader.getData(0));
  }
  else if (isParamValid("exp_values"))
    _exp_values.push_back(getParam<std::vector<Real>>("exp_values"));
  else
    mooseError("Either 'exp_values' or 'file_name' parameters must be specified to represent "
               "experimental data.");
}

Real
GaussianVector::function(const std::vector<std::vector<Real>> & exp,
                   const std::vector<std::vector<Real>> & model,
                   const Real & noise,
                   const bool & log_likelihood)
{
  Real result = 0.0;
  Real val1;
  for (unsigned j = 0; j < exp.size(); ++j){
    for (unsigned i = 0; i < exp[j].size(); ++i)
    {
        val1 = Normal::pdf(exp[j][i], model[j][i], noise);
        val1 = std::log(val1);
        result += val1;
    }
  }
  if (!log_likelihood)
    result = std::exp(result);
  return result;
}
Real
GaussianVector::function(const std::vector<Real> & exp,
                   const std::vector<Real> & model,
                   const Real & noise,
                   const bool & log_likelihood)
{
  Real result = 0.0;
  Real val1;
  for (unsigned i = 0; i < exp.size(); ++i)
    {
        val1 = Normal::pdf(exp[i], model[i], noise);
        val1 = std::log(val1);
        result += val1;
    }
  if (!log_likelihood)
    result = std::exp(result);
  return result;
}

Real
GaussianVector::function(const std::vector<std::vector<Real>> & exp,
                   const std::vector<Real> & model,
                   const Real & noise,
                   const bool & log_likelihood)
{
  Real result = 0.0;
  Real val1;
  std::vector<Real> _exp_values_flat;
  std::vector<Real> m2;
  
  for(const auto& sub: exp)
    _exp_values_flat.insert(end(_exp_values_flat),begin(sub), end(sub));
  for (unsigned i = 0; i < exp.size(); ++i){
    m2.insert(m2.end(),model.begin(),model.end());
  }
  for (unsigned i = 0; i < exp.size(); ++i)
    {
        val1 = Normal::pdf(_exp_values_flat[i], model[i], noise);
        val1 = std::log(val1);
        result += val1;
    }
  if (!log_likelihood)
    result = std::exp(result);
  return result;
}

//Real
//GaussianVector::function(const std::vector<std::vector<Real>> & x) const
//{
//  std::vector<Real> x_flat;
//  for (auto & sub: x){
//    x_flat.insert(std::end(x_flat),std::begin(sub), std::end(sub));
//  }
//  std::vector<Real> _exp_values_flat;
//  for (auto & sub: _exp_values){
//    _exp_values_flat.insert(std::end(_exp_values_flat), std::begin(sub), std::end(sub));
//  }
  

//  return function(_exp_values_flat, x_flat, _noise, _log_likelihood);
//}
Real
GaussianVector::function(const std::vector<std::vector<Real>> & x) const
{
  return function(_exp_values, x, _noise, _log_likelihood);
}
Real
GaussianVector::function(const std::vector<Real> & x) const
{
  return function(_exp_values, x, _noise, _log_likelihood);
}
