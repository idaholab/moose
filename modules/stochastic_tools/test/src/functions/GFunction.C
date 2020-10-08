//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GFunction.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsTestApp", GFunction);

InputParameters
GFunction::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();

  params.addRequiredParam<std::vector<Real>>("q_vector", "q values for g-function");
  params.addRequiredParam<SamplerName>(
      "sampler", "The Sampler object to use to perform g-function evaluations.");
  return params;
}

GFunction::GFunction(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _sampler(getSampler("sampler")),
    _q_vector(getParam<std::vector<Real>>("q_vector")),
    _values(declareVector("g_values"))
{
  if (_q_vector.size() != _sampler.getNumberOfCols())
    paramError("q_vector", "The 'q_vector' size must match the number of columns in the Sampler.");
  for (const auto & q : _q_vector)
    if (q < 0)
      paramError("q_vector", "The 'q_vector' entries must be zero or positive.");
}

void
GFunction::execute()
{
  _values.reserve(_sampler.getNumberOfLocalRows());
  for (dof_id_type r = _sampler.getLocalRowBegin(); r < _sampler.getLocalRowEnd(); ++r)
  {
    std::vector<Real> x = _sampler.getNextLocalRow();
    Real y = 1;
    for (std::size_t i = 0; i < _q_vector.size(); ++i)
      y *= (std::abs(4 * x[i] - 2) + _q_vector[i]) / (1 + _q_vector[i]);
    _values.push_back(y);
  }
}

void
GFunction::finalize()
{
  if (_parallel_type == "REPLICATED")
    _communicator.gather(0, _values);
}
