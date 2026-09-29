//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KLExpansionUserObject.h"
#include "KLCovarianceBase.h"
#include "Normal.h"

#include <Eigen/Dense>
#include "MooseRandom.h"

registerMooseObject("StochasticToolsApp", KLExpansionUserObject);

InputParameters
KLExpansionUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Builds a separable Karhunen-Loeve expansion (1-3 dimensions) via marginal "
      "eigenproblems on a uniform reference grid, and evaluates it at arbitrary "
      "points using Nystrom extension. Truncation is controlled by specifying "
      "EXACTLY ONE of 'n_terms' or 'variance_fraction'.");
  params.addRequiredParam<std::vector<Real>>(
      "lower_bounds", "Domain lower bound, one entry per active dimension (1-3)");
  params.addRequiredParam<std::vector<Real>>("upper_bounds",
                                             "Domain upper bound, one entry per active dimension");
  params.addRequiredRangeCheckedParam<std::vector<unsigned int>>(
      "n_grid", "n_grid > 0", "Reference grid size, one entry per active dimension");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "covariance_functions", "One 1D covariance marginal UserObject name per active dimension");
  params.addRangeCheckedParam<unsigned int>(
      "n_terms",
      "n_terms > 0",
      "Number of joint KL terms to retain. Specify this OR variance_fraction, not both.");
  params.addRangeCheckedParam<Real>(
      "variance_fraction",
      "variance_fraction > 0 & variance_fraction <= 1",
      "Fraction (0,1] of total spectral trace to retain (the fewest joint modes whose cumulative "
      "eigenvalue sum reaches this fraction are kept). Specify this OR n_terms, not both.");
  params.addParam<unsigned int>("seed", 0, "Seed for sampling the random KL coefficients");
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  return params;
}

KLExpansionUserObject::KLExpansionUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _dim(getParam<std::vector<Real>>("lower_bounds").size()),
    _n_terms_param(isParamValid("n_terms") ? getParam<unsigned int>("n_terms") : 0),
    _variance_fraction(isParamValid("variance_fraction") ? getParam<Real>("variance_fraction") : 0),
    _seed(getParam<unsigned int>("seed"))
{
  if (_dim < 1 || _dim > 3)
    paramError("lower_bounds", "Must have 1 to 3 entries (separable dimensions), got ", _dim, ".");

  if (isParamValid("n_terms") == isParamValid("variance_fraction"))
    paramError("n_terms", "Specify exactly one of 'n_terms' or 'variance_fraction'.");

  const auto & low = getParam<std::vector<Real>>("lower_bounds");
  const auto & high = getParam<std::vector<Real>>("upper_bounds");
  const auto & ng = getParam<std::vector<unsigned int>>("n_grid");
  const auto & cov_names = getParam<std::vector<UserObjectName>>("covariance_functions");

  if (high.size() != _dim || ng.size() != _dim || cov_names.size() != _dim)
    mooseError("lower_bounds, upper_bounds, n_grid, and covariance_functions must all have "
               "the same length, equal to the number of active dimensions (",
               _dim,
               ").");

  for (const auto d : make_range(_dim))
  {
    if (high[d] <= low[d])
      paramError("upper_bounds", "Each entry must be greater than the matching 'lower_bounds'.");
    _low[d] = low[d];
    _high[d] = high[d];
    _n_grid[d] = ng[d];
    _covariance[d] = &getUserObjectByName<KLCovarianceBase>(cov_names[d]);
  }

  buildBasis();
}

void
KLExpansionUserObject::buildBasis()
{
  for (const auto d : make_range(_dim))
  {
    _grid[d] = buildReferenceGrid(_low[d], _high[d], _n_grid[d]);
    solveMarginalEigenproblem(d);
  }
  sortJointModes();
  sampleRandomCoefficients();
}

std::vector<Real>
KLExpansionUserObject::buildReferenceGrid(Real low, Real high, unsigned int n) const
{
  std::vector<Real> grid(n);
  if (n == 1)
  {
    grid[0] = 0.5 * (low + high);
    return grid;
  }
  const Real step = (high - low) / (n - 1);
  for (const auto i : make_range(n))
    grid[i] = low + i * step;
  return grid;
}

void
KLExpansionUserObject::solveMarginalEigenproblem(unsigned int d)
{
  const auto & grid = _grid[d];
  const unsigned int n = grid.size();
  const auto * cov = _covariance[d];

  Eigen::MatrixXd C(n, n);
  for (const auto k : make_range(n))
    for (const auto l : make_range(n))
      C(k, l) = cov->computeCovariance(grid[k], grid[l]);

  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(C);
  if (solver.info() != Eigen::Success)
    mooseError("KLExpansionUserObject: eigen decomposition failed for dimension ", d);

  const auto & eigenvalues = solver.eigenvalues(); // ascending order, guaranteed real
  const auto & eigenvectors = solver.eigenvectors();

  // Eigenvalues at or below the symmetric eigensolver's backward error bound, n * epsilon *
  // lambda_max, are roundoff and their eigenvectors carry no information. They are discarded
  // because the Nystrom extension divides by sqrt(lambda).
  const Real tol = n * std::numeric_limits<Real>::epsilon() * eigenvalues(n - 1);

  _lambda[d].clear();
  _eigvec[d].clear();

  // eigenvalues() is ascending -> walk backwards from the end for descending order
  for (const auto m : make_range(n))
  {
    const unsigned int idx = n - 1 - m;
    if (eigenvalues(idx) <= tol)
      break;

    // Eigenvectors are defined only up to sign, and the solver's choice can differ between
    // platforms, which would flip the sign of the corresponding term in the sampled field. Make
    // the first entry whose magnitude is at least half the largest magnitude positive; entries
    // that large are far from roundoff, so the choice does not depend on rounding.
    const auto v = eigenvectors.col(idx);
    const Real half_max = 0.5 * v.cwiseAbs().maxCoeff();
    unsigned int l0 = 0;
    while (std::abs(v(l0)) < half_max)
      ++l0;
    const Real sign = v(l0) > 0 ? 1.0 : -1.0;

    _lambda[d].push_back(eigenvalues(idx));
    _eigvec[d].emplace_back(n);
    for (const auto l : make_range(n))
      _eigvec[d].back()[l] = sign * v(l);
  }
}

void
KLExpansionUserObject::sortJointModes()
{
  const unsigned int n0 = _lambda[0].size();
  const unsigned int n1 = (_dim >= 2) ? _lambda[1].size() : 1;
  const unsigned int n2 = (_dim >= 3) ? _lambda[2].size() : 1;

  std::vector<std::pair<Real, std::array<unsigned int, 3>>> candidates;
  candidates.reserve(static_cast<std::size_t>(n0) * n1 * n2);

  Real total_trace = 0.0;
  for (const auto i : make_range(n0))
    for (const auto j : make_range(n1))
      for (const auto k : make_range(n2))
      {
        const std::array<unsigned int, 3> modes{i, j, k};
        // Multiply the marginal eigenvalues in ascending order so that permutations of the same
        // factors, which occur when dimensions share a covariance and reference grid, give
        // exactly equal joint eigenvalues and are ordered by the tie break below.
        std::array<Real, 3> factors{};
        for (const auto d : make_range(_dim))
          factors[d] = _lambda[d][modes[d]];
        std::sort(factors.begin(), factors.begin() + _dim);
        Real eig = 1.0;
        for (const auto d : make_range(_dim))
          eig *= factors[d];
        candidates.push_back({eig, modes});
        total_trace += eig;
      }

  // Descending eigenvalue order. Ties are broken by the marginal mode indices because the order
  // of equal elements from std::sort differs between standard library implementations, and the
  // order determines which random coefficient multiplies each mode.
  std::sort(candidates.begin(),
            candidates.end(),
            [](const auto & a, const auto & b)
            { return a.first > b.first || (a.first == b.first && a.second < b.second); });

  unsigned int keep = 0;
  if (_n_terms_param > 0)
  {
    if (_n_terms_param > candidates.size())
      mooseWarning("KLExpansionUserObject: requested n_terms = ",
                   _n_terms_param,
                   " exceeds the total number of available joint modes (",
                   candidates.size(),
                   " = product of the numbers of marginal eigenvalues above roundoff). "
                   "Retaining all ",
                   candidates.size(),
                   " available modes instead. "
                   "Increase n_grid in one or more dimensions if more modes are needed.");
    keep = std::min<unsigned int>(_n_terms_param, candidates.size());
  }
  else
  {
    // variance_fraction requested: keep the fewest modes whose cumulative
    // eigenvalue sum reaches the requested fraction of the total joint trace.
    const Real target = _variance_fraction * total_trace;
    Real cumulative = 0.0;
    for (; keep < candidates.size(); ++keep)
    {
      cumulative += candidates[keep].first;
      if (cumulative >= target)
      {
        ++keep; // include the mode that just crossed the threshold
        break;
      }
    }
    if (keep == 0)
      keep = 1; // always keep at least one mode
  }

  _joint_index.resize(keep);
  _n_used_modes.fill(0);
  for (const auto m : make_range(keep))
  {
    _joint_index[m] = candidates[m].second;
    for (const auto d : make_range(_dim))
      _n_used_modes[d] = std::max(_n_used_modes[d], _joint_index[m][d] + 1);
  }
}

void
KLExpansionUserObject::sampleRandomCoefficients()
{
  MooseRandom rng;
  rng.seed(0, _seed);

  _xi.resize(_joint_index.size());
  for (auto & x : _xi)
    x = rng.randNormal(0);
}

Real
KLExpansionUserObject::nystromExtend(Real s_star, unsigned int d, unsigned int mode) const
{
  //   mode(s*) = (1/sqrt(lambda)) * sum_l C(s*, t_l) * v_l
  const auto & grid = _grid[d];
  const auto & vec = _eigvec[d][mode];
  const auto * cov = _covariance[d];

  Real sum = 0.0;
  for (const auto l : index_range(grid))
    sum += cov->computeCovariance(s_star, grid[l]) * vec[l];

  return sum / std::sqrt(_lambda[d][mode]);
}

void
KLExpansionUserObject::computeModeValues(const Point & p, std::vector<Real> & mode_vals) const
{
  // Joint modes share marginal modes, so extend each marginal mode once per dimension
  std::array<std::vector<Real>, 3> marginal_vals;
  for (const auto d : make_range(_dim))
  {
    marginal_vals[d].resize(_n_used_modes[d]);
    for (const auto mode : make_range(_n_used_modes[d]))
      marginal_vals[d][mode] = nystromExtend(p(d), d, mode);
  }

  mode_vals.resize(_joint_index.size());
  for (const auto m : index_range(_joint_index))
  {
    Real a = 1.0;
    for (const auto d : make_range(_dim))
      a *= marginal_vals[d][_joint_index[m][d]];
    mode_vals[m] = a;
  }
}

Real
KLExpansionUserObject::getValue(const Point & p) const
{
  std::vector<Real> a;
  computeModeValues(p, a);

  Real value = 0.0;
  for (const auto m : index_range(a))
    value += a[m] * _xi[m];
  return value;
}

Real
KLExpansionUserObject::getStandardizedValue(const Point & p) const
{
  std::vector<Real> a;
  computeModeValues(p, a);

  Real raw = 0.0, var = 0.0;
  for (const auto m : index_range(a))
  {
    raw += a[m] * _xi[m];
    var += a[m] * a[m];
  }

  // Nominal pointwise variance of the untruncated separable covariance, prod_d C_d(p_d, p_d)
  Real nominal_var = 1.0;
  for (const auto d : make_range(_dim))
    nominal_var *= _covariance[d]->computeCovariance(p(d), p(d));

  // Degenerate point (e.g. far outside reference-grid support), where the truncated expansion
  // retains a negligible part of the nominal standard deviation: return 0 to avoid 0/0. The
  // relative threshold makes the standardized field independent of the covariance variances.
  const Real sigma = std::sqrt(var);
  if (sigma < 1e-14 * std::sqrt(nominal_var))
    return 0.0;

  return raw / sigma;
}

Real
KLExpansionUserObject::getCopulaUniform(const Point & p) const
{
  Real u = Normal::cdf(getStandardizedValue(p), 0.0, 1.0);

  // eps <= u <= 1-eps for numerical reasons. If u =1, inverse transform for weibull will have
  // issues.
  const Real eps = 1e-12;
  u = std::min(std::max(u, eps), 1.0 - eps);
  return u;
}
