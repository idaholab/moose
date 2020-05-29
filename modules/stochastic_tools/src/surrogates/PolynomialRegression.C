//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolynomialRegression.h"

registerMooseObject("StochasticToolsApp", PolynomialRegression);

InputParameters
PolynomialRegression::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Evaluates linear regression model with coefficients computed from PolynomialRegressionTrainer.");
  return params;
}

PolynomialRegression::PolynomialRegression(const InputParameters & parameters)
  : SurrogateModel(parameters),
    _coeff(getModelData<std::vector<Real>>("_coeff"))
{
}

Real
PolynomialRegression::evaluate(const std::vector<Real> & x) const
{
  // Check whether input point has same dimensionality as training data
  mooseAssert((_coeff.size() - 1) == x.size(),
              "Input point does not match dimensionality of training data.");

  Real val = _coeff[0];
  for (unsigned int i = 0; i < x.size(); ++i)
    val += _coeff[i + 1] * x[i];

  return val;
}
