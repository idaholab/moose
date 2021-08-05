//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolynomialRegressionSurrogate.h"

registerMooseObject("StochasticToolsApp", PolynomialRegressionSurrogate);

InputParameters
PolynomialRegressionSurrogate::validParams()
{
  InputParameters params = SurrogateModel::validParams();
  params.addClassDescription("Evaluates polynomial regression model with coefficients computed "
                             "from PolynomialRegressionTrainer.");
  return params;
}

PolynomialRegressionSurrogate::PolynomialRegressionSurrogate(const InputParameters & parameters)
  : SurrogateModel(parameters),
    _coeff(getModelData<std::vector<std::vector<Real>>>("_coeff")),
    _power_matrix(getModelData<std::vector<std::vector<unsigned int>>>("_power_matrix")),
    _max_degree(getModelData<unsigned int>("_max_degree"))
{
}

Real
PolynomialRegressionSurrogate::evaluate(const std::vector<Real> & x) const
{
  // Check whether input point has same dimensionality as training data
  mooseAssert(_power_matrix[0].size() == x.size(),
              "Input point does not match dimensionality of training data.");

  Real val(0.0);
  for (unsigned int i = 0; i < _power_matrix.size(); ++i)
  {
    Real tmp_val(1.0);
    for (unsigned int j = 0; j < _power_matrix[i].size(); ++j)
      tmp_val *= MathUtils::pow(x[j], _power_matrix[i][j]);
    val += _coeff[0][i] * tmp_val;
  }

  return val;
}

void
PolynomialRegressionSurrogate::evaluate(const std::vector<Real> & x, std::vector<Real> & y) const
{
  // Check whether input point has same dimensionality as training data
  mooseAssert(_power_matrix[0].size() == x.size(),
              "Input point does not match dimensionality of training data.");

  y.assign(_coeff.size(), 0.0);
  for (unsigned int i = 0; i < _power_matrix.size(); ++i)
  {
    Real tmp_val(1.0);
    for (unsigned int j = 0; j < _power_matrix[i].size(); ++j)
      tmp_val *= MathUtils::pow(x[j], _power_matrix[i][j]);
    for (unsigned int r = 0; r < _coeff.size(); ++r)
      y[r] += _coeff[r][i] * tmp_val;
  }
}
