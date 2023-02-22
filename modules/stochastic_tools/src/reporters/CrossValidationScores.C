//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrossValidationScores.h"

registerMooseObject("StochasticToolsApp", CrossValidationScores);

InputParameters
CrossValidationScores::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "Tool for extracting cross-validation scores and storing them in a reporter for output.");

  params.addRequiredParam<std::vector<UserObjectName>>("models", "Names of surrogate models.");

  return params;
}

CrossValidationScores::CrossValidationScores(const InputParameters & parameters)
  : GeneralReporter(parameters), SurrogateModelInterface(this)
{
  for (const auto & mn : getParam<std::vector<UserObjectName>>("models"))
  {
    _models.push_back(&getSurrogateModelByName(mn));
    _cv_scores.push_back(&declareValueByName<std::vector<std::vector<Real>>>(mn));
  }
}

void
CrossValidationScores::execute()
{
  for (unsigned int m = 0; m < _models.size(); ++m)
    (*_cv_scores[m]) = _models[m]->getModelData<std::vector<std::vector<Real>>>("cv_scores");
}
