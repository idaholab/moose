//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KernelDensity1D.h"
#include "math.h"
#include "libmesh/utility.h"
#include "DelimitedFileReader.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "Normal.h"
#include "Uniform.h"

registerMooseObject("StochasticToolsApp", KernelDensity1D);

InputParameters
KernelDensity1D::validParams()
{
  InputParameters params = Distribution::validParams();
  MooseEnum bandwidth_rule("silverman standarddeviation userdefined");
  MooseEnum kernel_function("gaussian uniform");
  params.addClassDescription("KernelDensity1D distribution");
  params.addRequiredParam<MooseEnum>(
      "bandwidth_rule", bandwidth_rule, "Bandwidth rule for evaluating the bandwith.");
  params.addRequiredParam<MooseEnum>(
      "kernel_function",
      kernel_function,
      "Kernel function determines the shape of the underlying kernel for the kernel density.");
  params.addRangeCheckedParam<Real>("bandwidth",
                                    1.0,
                                    "bandwidth > 0",
                                    "Bandwidth controls the smoothness of the kernel density.");
  params.addParam<std::vector<Real>>("data", "The data vector.");
  params.addParam<FileName>("file_name", "Name of the CSV file.");
  params.addParam<std::string>(
      "file_column_name", "Name of column in csv file to use, by default first column is used.");
  return params;
}

KernelDensity1D::KernelDensity1D(const InputParameters & parameters)
  : Distribution(parameters),
    _bandwidth_rule(getParam<MooseEnum>("bandwidth_rule")),
    _kernel_function(getParam<MooseEnum>("kernel_function")),
    _bandwidth(getParam<Real>("bandwidth"))
{
  if (isParamValid("data") && isParamValid("file_name"))
    paramError("data", "data and file_name both cannot be set at the same time.");
  else if (isParamValid("file_name"))
  {
    MooseUtils::DelimitedFileReader reader(getParam<FileName>("file_name"));
    reader.read();
    if (isParamValid("file_column_name"))
      _data = reader.getData(getParam<std::string>("file_column_name"));
    else
      _data = reader.getData(0);
  }
  else if (isParamValid("data"))
    _data = getParam<std::vector<Real>>("data");
  else
    mooseError("Either 'data' or 'file_name' parameters must be specified to represent input data");
  Real mu = 0;
  Real sd = 0;
  for (unsigned i = 0; i < _data.size(); ++i)
  {
    mu += _data[i] / _data.size();
  }
  for (unsigned i = 0; i < _data.size(); ++i)
  {
    sd += Utility::pow<2>((_data[i] - mu)) / _data.size();
  }
  sd = std::pow(sd, 0.5);
  if (_bandwidth_rule == "silverman")
  {
    _bandwidth = 1.06 * sd * std::pow(_data.size(), -0.2);
  }
  else if (_bandwidth_rule == "standarddeviation")
  {
    _bandwidth = sd;
  }
}

Real
KernelDensity1D::pdf(const Real & x,
                     const Real & bandwidth,
                     const std::vector<Real> & data,
                     const MooseEnum & kernel_function)
{
  Real value = 0;
  if (kernel_function == "gaussian")
  {
    for (unsigned i = 0; i < data.size(); ++i)
    {
      value += 1 / (data.size() * bandwidth) * Normal::pdf(((x - data[i]) / bandwidth), 0.0, 1.0);
    }
  }
  else if (kernel_function == "uniform")
  {
    for (unsigned i = 0; i < data.size(); ++i)
    {
      value += 1 / (data.size() * bandwidth) * Uniform::pdf(((x - data[i]) / bandwidth), -1.0, 1.0);
    }
  }
  else
    ::mooseError("Invalid kernel function type ", std::string(kernel_function));
  return value;
}

Real
KernelDensity1D::cdf(const Real & x,
                     const Real & bandwidth,
                     const std::vector<Real> & data,
                     const MooseEnum & kernel_function)
{
  Real value = 0;
  if (kernel_function == "gaussian")
  {
    for (unsigned i = 0; i < data.size(); ++i)
    {
      value += 1.0 / (data.size()) * Normal::cdf(x, data[i], bandwidth);
    }
  }
  else if (kernel_function == "uniform")
  {
    for (unsigned i = 0; i < data.size(); ++i)
    {
      value += 1.0 / (data.size()) * Uniform::cdf((x - data[i]) / bandwidth, -1.0, 1.0);
    }
  }
  else
    ::mooseError("Invalid kernel function type ", std::string(kernel_function));
  return value;
}

Real
KernelDensity1D::quantile(const Real & p,
                          const Real & bandwidth,
                          const std::vector<Real> & data,
                          const MooseEnum & kernel_function)
{
  Real value = 0;
  if (kernel_function == "gaussian")
  {
    int index = std::round(p * (data.size() - 1));
    value = (Normal::quantile(p, data[index], bandwidth));
  }
  else if (kernel_function == "uniform")
  {
    int index = std::round(p * (data.size() - 1));
    value = (Uniform::quantile(p, -1.0, 1.0) * bandwidth + data[index]);
  }
  else
    ::mooseError("Invalid kernel function type ", std::string(kernel_function));
  return value;
}

Real
KernelDensity1D::pdf(const Real & x) const
{
  return pdf(x, _bandwidth, _data, _kernel_function);
}

Real
KernelDensity1D::cdf(const Real & x) const
{
  return cdf(x, _bandwidth, _data, _kernel_function);
}

Real
KernelDensity1D::quantile(const Real & p) const
{
  return quantile(p, _bandwidth, _data, _kernel_function);
}
