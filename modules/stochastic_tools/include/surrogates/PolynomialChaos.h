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
#include "nlohmann/json.h"

class PolynomialChaos : public SurrogateModel
{
public:
  static InputParameters validParams();
  PolynomialChaos(const InputParameters & parameters);
  using SurrogateModel::evaluate;
  virtual Real evaluate(const std::vector<Real> & x) const override;

  /// Access number of dimensions/parameters
  std::size_t getNumberOfParameters() const { return _poly.size(); }

  /// Number of terms in expansion
  std::size_t getNumberofCoefficients() const { return _tuple.size(); }

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
  Real powerExpectation(const unsigned int n) const;

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

  void store(nlohmann::json & json) const;

private:
  /// Variables calculation and for looping over the computed coefficients in parallel
  ///
  /// The various utility methods in this class require the coefficients be partitioned in parallel,
  /// but the data being partitioned is loaded from the trainer so it might not be available. Thus,
  /// the partitioning is done on demand, if needed.
  ///
  /// The methods are marked const because they do not modify the loaded data, to keep this interface
  /// the partitioning uses mutable variables.
  ///@{
  mutable dof_id_type _n_local_coeff = std::numeric_limits<dof_id_type>::max();
  mutable dof_id_type _local_coeff_begin = 0;
  mutable dof_id_type _local_coeff_end = 0;
  void linearPartitionCoefficients() const;
  ///@}

  // The following items are loaded from a SurrogateTrainer using getModelData

  /// Maximum polynomial order. The sum of 1D polynomial orders does not go above this value.
  const unsigned int & _order;

  /// Total number of parameters/dimensions
  const unsigned int & _ndim;

  /// Total number of coefficient (defined by size of _tuple)
  const std::size_t & _ncoeff;

  /// A _ndim-by-_ncoeff matrix containing the appropriate one-dimensional polynomial order
  const std::vector<std::vector<unsigned int>> & _tuple;

  /// These are the coefficients we are after in the PC expansion
  const std::vector<Real> & _coeff;

  /// The distributions used for sampling
  const std::vector<std::unique_ptr<const PolynomialQuadrature::Polynomial>> & _poly;

  friend void to_json(nlohmann::json & json, const PolynomialChaos * const & pc);
};
