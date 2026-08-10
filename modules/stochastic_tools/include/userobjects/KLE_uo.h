// Separable N-D (N <= 3) Karhunen-Loeve expansion for MOOSE, using Nystrom extension to evaluate
// the expansion at arbitrary points (irregular mesh nodes) off of a uniform reference grid
// solved once per dimension.

#pragma once

#include "GeneralUserObject.h"

// KLCovarianceBase
// Minimal pluggable interface for a 1D covariance marginal C(x1, x2).
// One instance must be supplied per separable dimension of the KL expansion.

class KLCovarianceBase : public GeneralUserObject
{
public:
  static InputParameters validParams();
  KLCovarianceBase(const InputParameters & parameters);

  virtual Real computeCovariance(Real x1, Real x2) const = 0;

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}
};

// Exponential
// C(x1, x2) = variance * exp( -|x1 - x2| / length_scale )
class KLExponentialCovariance : public KLCovarianceBase
{
public:
  static InputParameters validParams();
  KLExponentialCovariance(const InputParameters & parameters);

  virtual Real computeCovariance(Real x1, Real x2) const override;

protected:
  const Real _variance;
  const Real _length_scale;
};

// Squared Exponential
// C(x1, x2) = variance * exp( -(x1-x2)^2 / (2 * length_scale^2) )
class KLSquaredExponentialCovariance : public KLCovarianceBase
{
public:
  static InputParameters validParams();
  KLSquaredExponentialCovariance(const InputParameters & parameters);

  virtual Real computeCovariance(Real x1, Real x2) const override;

protected:
  const Real _variance;
  const Real _length_scale;
};

class KLExpansionUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();
  KLExpansionUserObject(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

  // Raw (unstandardized) correlated Gaussian KL field value.
  Real getValue(const Point & p) const;

  // Gaussian field standardized to unit pointwise variance: Z(p) / sigma(p),
  // where sigma(p) is the actual variance of the truncated expansion at p
  Real getStandardizedValue(const Point & p) const;

  // Gaussian-copula uniform field: Phi(getStandardizedValue(p)), i.e. a
  // spatially correlated Uniform(0,1) field with the KL expansion's
  // correlation structure preserved through the copula.
  Real getCopulaUniform(const Point & p) const;

  // Number of joint KL terms actually retained (useful if determined from variance_fraction).
  unsigned int numTerms() const { return _joint_index.size(); }

protected:
  void buildBasis();
  std::vector<Real> buildReferenceGrid(Real low, Real high, unsigned int n) const;
  void solveMarginalEigenproblem(unsigned int d);
  void sortJointModes();
  void sampleRandomCoefficients();

  // Nystrom extension of marginal mode `mode` of dimension `d`, evaluated at s_star.
  Real nystromExtend(Real s_star, unsigned int d, unsigned int mode) const;

  // Computes a_m(p) = joint_sqrt_eig[m] * prod_d nystromExtend(p_d, d, mode)
  // for every retained joint mode m, WITHOUT applying the random xi
  // coefficients. Shared by getValue(), getStandardizedValue()
  void computeModeValues(const Point & p, std::vector<Real> & mode_vals) const;

  const unsigned int _dim; // number of active separable dimensions
  std::array<Real, 3> _low{};
  std::array<Real, 3> _high{};
  std::array<unsigned int, 3> _n_grid{};

  const unsigned int _n_terms_param;
  const Real _variance_fraction;

  const unsigned int _seed;

  std::array<const KLCovarianceBase *, 3> _covariance{nullptr, nullptr, nullptr};

  std::array<std::vector<Real>, 3> _grid;   // _grid[d][l]
  std::array<std::vector<Real>, 3> _lambda; // _lambda[d][mode], descending order, full spectrum
  std::array<std::vector<std::vector<Real>>, 3> _eigvec; // _eigvec[d][mode][grid_pt]

  std::vector<std::array<unsigned int, 3>>
      _joint_index; // marginal mode index per dim, per joint mode

  std::vector<Real> _xi; // sampled N(0,1) coefficient per joint mode
};
