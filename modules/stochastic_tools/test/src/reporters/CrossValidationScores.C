//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrossValidationScores.h"

registerMooseObject("StochasticToolsTestApp", CrossValidationScores);

InputParameters
CrossValidationScores::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "Tool for extracting cross-validation scores and storing them in a reporter for output.");

  params.addParam<std::vector<unsigned int>>("responses",
                                             std::vector<unsigned int>(1, 0),
                                             "Response indices for which to extract CV scores.");
  params.addRequiredParam<std::vector<UserObjectName>>("models", "Names of surrogate models.");

  return params;
}

CrossValidationScores::CrossValidationScores(const InputParameters & parameters)
  : GeneralReporter(parameters),
    SurrogateModelInterface(this),
    _response_indices(getParam<std::vector<unsigned int>>("responses"))
{
  for (const auto & mn : getParam<std::vector<UserObjectName>>("models"))
  {
    _models.push_back(&getSurrogateModelByName(mn));
    for (const auto & r : _response_indices)
      _cv_scores.push_back(&declareValueByName<std::vector<Real>>(mn + "_" + std::to_string(r)));
  }
}

void
CrossValidationScores::execute()
{
  unsigned int n_r = _response_indices.size();
  for (unsigned int m = 0; m < _models.size(); ++m)
    for (unsigned int r = 0; r < n_r; ++r)
      for (auto trial : _models[m]->getModelData<std::vector<std::vector<Real>>>("cv_scores"))
        (*_cv_scores[n_r * m + r]).push_back(trial[_response_indices[r]]);
}
