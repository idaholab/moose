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
#include "Calculators.h"

typedef StochasticTools::Calculator<std::vector<Real>, Real> RealCalculator;

class PolynomialRegressionTrainer : public SurrogateTrainer
{
public:
  static InputParameters validParams();

  PolynomialRegressionTrainer(const InputParameters & parameters);

  virtual void preTrain() override;

  virtual void train() override;

  virtual void postTrain() override;

private:
  /// Data from the current predictor row
  const std::vector<Real> & _predictor_row;

  /// Types for the polynomial regression
  const MooseEnum & _regression_type;

  /// The penalty parameter for Ridge regularization
  const Real & _penalty;

  /// Coefficients of regression model
  std::vector<std::vector<Real>> & _coeff;

  /// Maximum polynomial degree, limiting the sum of constituent polynomial degrees.
  const unsigned int & _max_degree;

  /// Matirx co containing the touples of the powers for each term
  const std::vector<std::vector<unsigned int>> & _power_matrix;

  /// Number of terms in the polynomial expression.
  const unsigned int _n_poly_terms;

  ///@{
  /// Matrix and rhs for the regression problem
  DenseMatrix<Real> _matrix;
  std::vector<DenseVector<Real>> _rhs;
  ///@}

  /// Calculators used in standardizing polynomial features.
  std::vector<std::unique_ptr<RealCalculator>> _calculators;

  /// Calculator used to sum response values.
  std::vector<Real> _r_sum;
};
