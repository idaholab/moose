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
#include "PolynomialQuadrature.h"
#include "QuadratureSampler.h"

#include "Distribution.h"

class PolynomialChaos : public SurrogateModel
{
public:
  static InputParameters validParams();
  PolynomialChaos(const InputParameters & parameters);
  virtual void initialSetup() override;
  virtual void train() override;
  virtual void trainFinalize() override;
  virtual Real evaluate(const std::vector<Real> & x) const override;

  /// Access number of dimensions/parameters
  unsigned int getNumberOfParameters() const { return _ndim; }

  /// Number of terms in expansion
  unsigned int getNumberofCoefficients() const { return _ncoeff; }

  /// Access polynomial orders from tuple
  ////@{
  const std::vector<std::vector<unsigned int>> & getPolynomialOrders() const;
  unsigned int getPolynomialOrder(const unsigned int dim, const unsigned int i) const;
  ///@}

  /// Access computed expansion coefficients
  const std::vector<Real> & getCoefficients() const;

  /// Evaluate mean: \mu = E[u]
  virtual Real computeMean() const;

  /// Evaluate standard deviation: \sigma = sqrt(E[(u-\mu)^2])
  virtual Real computeStandardDeviation() const;

  /// Compute expectation of a certain power of the QoI: E[(u-\mu)^n]
  Real powerExpectation(const unsigned int n, const bool distributed = true) const;

  /// Evaluates partial derivative of expansion: du(x)/dx_dim
  Real computeDerivative(const unsigned int dim, const std::vector<Real> & x) const;
  /**
   * Evaluates sum of partial derivative of expansion. Example:
   * computeGradient({0, 2, 3}, x) = du(x)/dx_0dx_2dx_3
   */
  Real computePartialDerivative(const std::vector<unsigned int> & dim,
                                const std::vector<Real> & x) const;

  /// Computes Sobol sensitivities S_{i_1,i_2,...,i_s}, where ind = i_1,i_2,...,i_s
  Real computeSobolIndex(const std::set<unsigned int> & ind) const;
  Real computeSobolTotal(const unsigned int dim) const;

protected:
  /**
   * Function computing for computing _tuple
   * Example for ndim = 3, order = 4:
   * | 0 | 1 0 0 | 2 1 1 0 0 0 | 3 2 2 1 1 1 0 0 0 0 |
   * | 0 | 0 1 0 | 0 1 0 2 1 0 | 0 1 0 2 1 0 3 2 1 0 |
   * | 0 | 0 0 1 | 0 0 1 0 1 2 | 0 0 1 0 1 2 0 1 2 3 |
   */
  static std::vector<std::vector<unsigned int>> generateTuple(const unsigned int ndim,
                                                              const unsigned int order);
  /// Tuple sorter function
  static bool sortTuple(const std::vector<unsigned int> & first,
                        const std::vector<unsigned int> & second);

private:
  /// Utility for looping over coefficients in parallel
  ///@{
  dof_id_type _n_local_coeff;
  dof_id_type _local_coeff_begin;
  dof_id_type _local_coeff_end;
  ///@}

  /// The following items are used tor training only

  /// Sampler from which the parameters were perturbed
  Sampler * _sampler;

  /// QuadratureSampler pointer, necessary for applying quadrature weights
  QuadratureSampler * _quad_sampler;

  /// Vector postprocessor of the results from perturbing the model with _sampler
  const VectorPostprocessorValue * _values_ptr = nullptr;

  /// Total number of parameters/dimensions
  unsigned int _ndim;

  /// Total number of coefficient (defined by size of _tuple)
  std::size_t _ncoeff;

  /// True when _sampler data is distributed
  bool _values_distributed;

  // The following items are stored using declareModelData for use as a trained model.

  /// Maximum polynomial order. The sum of 1D polynomial orders does not go above this value.
  unsigned int & _order;

  /// A _ndim-by-_ncoeff matrix containing the appropriate one-dimensional polynomial order
  std::vector<std::vector<unsigned int>> & _tuple;

  /// These are the coefficients we are after in the PC expansion
  std::vector<Real> & _coeff;

  /// The distributions used for sampling
  std::vector<std::unique_ptr<const PolynomialQuadrature::Polynomial>> & _poly;
};
