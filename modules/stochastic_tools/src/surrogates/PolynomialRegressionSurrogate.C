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
    _coeff(getModelData<std::vector<Real>>("_coeff")),
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
  unsigned int n_terms(_power_matrix.size());

  for (unsigned int i = 0; i < n_terms; ++i)
  {
    std::vector<unsigned int> i_powers(_power_matrix[i]);

    Real tmp_val(1.0);
    for (unsigned int j = 0; j < i_powers.size(); ++j)
    {
      tmp_val *= pow(x[j], i_powers[j]);
    }
    val += _coeff[i] * tmp_val;
  }

  return val;
}
