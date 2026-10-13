//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

class KLCovarianceBase;

/**
 * Separable N-D (N <= 3) Karhunen-Loeve expansion of a Gaussian random field. The marginal
 * eigenproblems are solved once per dimension on a uniform reference grid, and the Nystrom
 * extension evaluates the expansion at arbitrary points (e.g. nodes of an irregular mesh).
 */
class KLExpansionUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();
  KLExpansionUserObject(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

  /// Raw (unstandardized) correlated Gaussian KL field value.
  Real getValue(const Point & p) const;

  /// Gaussian field standardized to unit pointwise variance: Z(p) / sigma(p),
  /// where sigma(p)^2 is the actual variance of the truncated expansion at p
  Real getStandardizedValue(const Point & p) const;

  /// Gaussian-copula uniform field: Phi(getStandardizedValue(p)), i.e. a
  /// spatially correlated Uniform(0,1) field with the KL expansion's
  /// correlation structure preserved through the copula.
  Real getCopulaUniform(const Point & p) const;

  /// Number of joint KL terms actually retained (useful if determined from variance_fraction).
  unsigned int numTerms() const { return _joint_index.size(); }

protected:
  void buildBasis();
  std::vector<Real> buildReferenceGrid(Real low, Real high, unsigned int n) const;
  void solveMarginalEigenproblem(unsigned int d);
  void sortJointModes();
  void sampleRandomCoefficients();

  /// Nystrom extension of marginal mode `mode` of dimension `d`, evaluated at s_star.
  /// Returns sqrt(lambda) times the eigenfunction, where lambda is the marginal eigenvalue.
  Real nystromExtend(Real s_star, unsigned int d, unsigned int mode) const;

  /// Computes a_m(p) = prod_d nystromExtend(p_d, d, mode) for every retained joint mode m,
  /// WITHOUT applying the random xi coefficients. Shared by getValue(), getStandardizedValue()
  void computeModeValues(const Point & p, std::vector<Real> & mode_vals) const;

  /// Number of active separable dimensions
  const unsigned int _dim;
  std::array<Real, 3> _low{};
  std::array<Real, 3> _high{};
  std::array<unsigned int, 3> _n_grid{};

  /// Number of joint modes to retain, or 0 if truncation is set by _variance_fraction
  const unsigned int _n_terms_param;
  /// Fraction of the spectral trace to retain, used when n_terms is not given
  const Real _variance_fraction;

  const unsigned int _seed;

  std::array<const KLCovarianceBase *, 3> _covariance{nullptr, nullptr, nullptr};

  /// Reference grid coordinates, _grid[d][l]
  std::array<std::vector<Real>, 3> _grid;
  /// Marginal eigenvalues above roundoff, _lambda[d][mode], in descending order
  std::array<std::vector<Real>, 3> _lambda;
  /// Marginal eigenvectors, _eigvec[d][mode][grid_pt]
  std::array<std::vector<std::vector<Real>>, 3> _eigvec;

  /// Marginal mode index per dim, per retained joint mode
  std::vector<std::array<unsigned int, 3>> _joint_index;

  /// Number of leading marginal modes per dim referenced by the retained joint modes
  std::array<unsigned int, 3> _n_used_modes{};

  /// Sampled N(0,1) coefficient per joint mode
  std::vector<Real> _xi;
};
