//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "GaussianProcessTester.h"

#include "Sampler.h"

registerMooseObject("StochasticToolsTestApp", GaussianProcessTester);

InputParameters
GaussianProcessTester::validParams()
{
  InputParameters params = EvaluateSurrogate::validParams();
  return params;
}

GaussianProcessTester::GaussianProcessTester(const InputParameters & parameters)
  : EvaluateSurrogate(parameters)
{
  const auto & model_names = getParam<std::vector<UserObjectName>>("model");
  _GP_model.resize(_model.size());
  _std_vector.reserve(model_names.size());
  for (unsigned int ii = 0; ii < _model.size(); ++ii)
  {
    _GP_model[ii] = dynamic_cast<const GaussianProcess *>(_model[ii]);
    _std_vector.push_back(&declareVector(model_names[ii] + "_std"));
  }
}

void
GaussianProcessTester::initialize()
{
  EvaluateSurrogate::initialize();
  for (auto & vec : _std_vector)
    vec->resize(_sampler.getNumberOfLocalRows(), 0);
}

void
GaussianProcessTester::execute()
{
  // Loop over samples
  for (dof_id_type p = _sampler.getLocalRowBegin(); p < _sampler.getLocalRowEnd(); ++p)
  {
    std::vector<Real> data = _sampler.getNextLocalRow();
    for (unsigned int m = 0; m < _GP_model.size(); ++m)
    {
      Real std;
      (*_value_vector[m])[p - _sampler.getLocalRowBegin()] = _GP_model[m]->evaluate(data, std);
      (*_std_vector[m])[p - _sampler.getLocalRowBegin()] = std;
    }
    if (_output_samples)
      for (unsigned int d = 0; d < _sampler.getNumberOfCols(); ++d)
      {
        (*_sample_vector[d])[p - _sampler.getLocalRowBegin()] = data[d];
      }
  }
}

void
GaussianProcessTester::finalize()
{
  EvaluateSurrogate::finalize();
  for (auto & vec : _std_vector)
    _communicator.gather(0, *vec);
}
