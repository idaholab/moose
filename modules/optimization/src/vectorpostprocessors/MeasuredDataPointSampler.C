//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeasuredDataPointSampler.h"

registerMooseObject("MooseApp", MeasuredDataPointSampler);

InputParameters
MeasuredDataPointSampler::validParams()
{
  InputParameters params = PointValueSampler::validParams();
  params.addRequiredParam<std::vector<std::vector<Real>>>(
      "measured_values", "Measured data to compare sample points to.");

  params.set<MooseEnum>("sort_by") = "id";
  params.suppressParameter<MooseEnum>("sort_by");

  return params;
}

MeasuredDataPointSampler::MeasuredDataPointSampler(const InputParameters & parameters)
  : PointValueSampler(parameters)
{
  const std::vector<std::vector<Real>> & data =
      getParam<std::vector<std::vector<Real>>>("measured_values");

  if (_variable_names.size() != data.size())
    paramError("measured_values", "Need to have a set of values for each variable given.");

  _measured.resize(data.size());
  _diff.resize(data.size());
  for (unsigned int i = 0; i < data.size(); ++i)
  {
    if (data[i].size() != _points.size())
      paramError("measured_values",
                 "Number of measured values per variable needs to match the number of points.");

    _measured[i] = &declareVector(_variable_names[i] + "_measured");
    _measured[i]->assign(data[i].begin(), data[i].end());

    _diff[i] = &declareVector(_variable_names[i] + "_difference");
    _diff[i]->resize(_points.size());
  }
}

void
MeasuredDataPointSampler::finalize()
{
  PointValueSampler::finalize();

  if (_values.size() != _diff.size())
    mooseError("Internal error, inconsistent misfit and values size.");

  for (unsigned int i = 0; i < _values.size(); ++i)
    for (unsigned int j = 0; j < _points.size(); ++j)
      (*_diff[i])[j] = (*_values[i])[j] - (*_measured[i])[j];
}
