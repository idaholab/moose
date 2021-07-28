//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurrogateModel.h"

class PolynomialRegressionSurrogate : public SurrogateModel
{
public:
  static InputParameters validParams();

  PolynomialRegressionSurrogate(const InputParameters & parameters);

  using SurrogateModel::evaluate;
  virtual Real evaluate(const std::vector<Real> & x) const override;
  virtual void evaluate(const std::vector<Real> & x, std::vector<Real> & y) const override;

protected:
  /// Coefficients of regression model
  const std::vector<std::vector<Real>> & _coeff;

  /// The power matrix for the terms in the polynomial expressions
  const std::vector<std::vector<unsigned int>> & _power_matrix;

private:
  /// Maximum polynomial degree, limiting the sum of constituent polynomial degrees.
  const unsigned int & _max_degree;
};
