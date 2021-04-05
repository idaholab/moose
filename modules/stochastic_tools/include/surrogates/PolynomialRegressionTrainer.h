//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/utility.h"
#include "SurrogateTrainer.h"
#include "MultiDimPolynomialGenerator.h"

class PolynomialRegressionTrainer : public SurrogateTrainer
{
public:
  static InputParameters validParams();

  PolynomialRegressionTrainer(const InputParameters & parameters);

  virtual void train() override;

  virtual void postTrain() override;

private:
  /// Data from the current sampler row
  const std::vector<Real> & _sampler_row;

  /// Response value
  const Real & _rval;

  /// Predictor values from reporters
  std::vector<const Real *> _pvals;

  /// Columns from sampler for predictors
  std::vector<unsigned int> _pcols;

  /// Number of dimensions.
  const unsigned int _n_dims;

  /// Types for the polynomial regression
  const MooseEnum & _regression_type;

  /// The penalty parameter for Ridge regularization
  const Real & _penalty;

  /// Coefficients of regression model
  std::vector<Real> & _coeff;

  /// Maximum polynomial degree, limiting the sum of constituent polynomial degrees.
  const unsigned int & _max_degree;

  /// Matirx co containing the touples of the powers for each term
  const std::vector<std::vector<unsigned int>> & _power_matrix;

  /// Number of terms in the polynomial expression.
  const unsigned int _n_poly_terms;

  ///@{
  /// Matrix and rhs for the regression problem
  DenseMatrix<Real> _matrix;
  DenseVector<Real> _rhs;
  ///@}
};
