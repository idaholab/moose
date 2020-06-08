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

  virtual void initialSetup() override;

  virtual void initialize() override;

  virtual void execute() override;

  virtual void finalize() override;

protected:
  /// Coefficients of regression model
  std::vector<Real> & _coeff;

  /// Matirx co containing the touples of the powers for each term
  std::vector<std::vector<unsigned int>> & _power_matrix;

  /// Types for the polynomial regression
  const MooseEnum & _regression_type;

private:
  /// Maximum polynomial degree, limiting the sum of constituent polynomial degrees.
  const unsigned int & _max_degree;

  /// The penalty parameter for Ridge regularization
  const Real & _penalty;

  /// Number of dimensions.
  unsigned int _n_dims;

  /// Number of terms in the polynomial expression.
  unsigned int _n_poly_terms;

  ///@{
  /// Matrix and rhs for the regression problem
  DenseMatrix<Real> _matrix;
  DenseVector<Real> _rhs;
  ///@}

  /// Sampler from which the parameters were perturbed
  Sampler * _sampler = nullptr;

  /// Vector postprocessor of the results from perturbing the model with _sampler
  const VectorPostprocessorValue * _values_ptr = nullptr;

  /// True when _sampler data is distributed
  bool _values_distributed = false; // default to false; set in initialSetup
};
