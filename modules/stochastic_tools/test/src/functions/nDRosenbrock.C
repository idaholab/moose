//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "nDRosenbrock.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsTestApp", nDRosenbrock);

InputParameters
nDRosenbrock::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addRequiredParam<SamplerName>(
      "sampler", "The Sampler object to use to perform g-function evaluations.");
  params.addParam<bool>("classify", false, "Flag to turn return binary values.");
  params.addParam<Real>("limiting_value", 0.0, "True if value exceeds limiting value.");
  return params;
}

nDRosenbrock::nDRosenbrock(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _sampler(getSampler("sampler")),
    _classify(getParam<bool>("classify")),
    _limiting_value(getParam<Real>("limiting_value")),
    _values(declareVector("g_values"))
{
}

void
nDRosenbrock::execute()
{
  _values.reserve(_sampler.getNumberOfLocalRows());
  for (dof_id_type r = _sampler.getLocalRowBegin(); r < _sampler.getLocalRowEnd(); ++r)
  {
    std::vector<Real> x = _sampler.getNextLocalRow();
    Real y = 0.0;
    for (std::size_t i = 0; i < x.size() - 1; ++i)
      y -= (10.0 * Utility::pow<2>(x[i + 1] - Utility::pow<2>(x[i])) + Utility::pow<2>(1 - x[i])) / 20.0;
    if (!_classify)
      _values.push_back(std::exp(y));
    else
    {
      if (std::exp(y) > _limiting_value)
        _values.push_back(1.0);
      else
        _values.push_back(0.0);
    }
  }
}

void
nDRosenbrock::finalize()
{
  if (_parallel_type == "REPLICATED")
    _communicator.gather(0, _values);
}
