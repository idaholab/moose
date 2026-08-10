// KLExpansion.C
#include "KLE_uo.h"

#include <Eigen/Dense>
#include "MooseRandom.h"

// KLCovarianceBase
InputParameters
KLCovarianceBase::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Base class for 1D covariance marginals used by KLExpansionUserObject.");
  return params;
}

KLCovarianceBase::KLCovarianceBase(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
}

// KLExponentialCovariance
registerMooseObject("MooseApp", KLExponentialCovariance);

InputParameters
KLExponentialCovariance::validParams()
{
  InputParameters params = KLCovarianceBase::validParams();
  params.addClassDescription(
      "Exponential covariance kernel: variance * exp(-|x1-x2|/length_scale).");
  params.addRequiredParam<Real>("variance", "Marginal variance");
  params.addRequiredParam<Real>("length_scale", "Correlation length");
  return params;
}

KLExponentialCovariance::KLExponentialCovariance(const InputParameters & parameters)
  : KLCovarianceBase(parameters),
    _variance(getParam<Real>("variance")),
    _length_scale(getParam<Real>("length_scale"))
{
}

Real
KLExponentialCovariance::computeCovariance(Real x1, Real x2) const
{
  return _variance * std::exp(-std::abs(x1 - x2) / _length_scale);
}

// KLSquaredExponentialCovariance
registerMooseObject("MooseApp", KLSquaredExponentialCovariance);

InputParameters
KLSquaredExponentialCovariance::validParams()
{
  InputParameters params = KLCovarianceBase::validParams();
  params.addClassDescription("Squared-exponential (Gaussian/RBF) covariance kernel: "
                             "variance * exp(-(x1-x2)^2 / (2*length_scale^2)).");
  params.addRequiredParam<Real>("variance", "Marginal variance");
  params.addRequiredParam<Real>("length_scale", "Correlation length");
  return params;
}

KLSquaredExponentialCovariance::KLSquaredExponentialCovariance(const InputParameters & parameters)
  : KLCovarianceBase(parameters),
    _variance(getParam<Real>("variance")),
    _length_scale(getParam<Real>("length_scale"))
{
}

Real
KLSquaredExponentialCovariance::computeCovariance(Real x1, Real x2) const
{
  const Real diff = x1 - x2;
  return _variance * std::exp(-(diff * diff) / (2.0 * _length_scale * _length_scale));
}

// KLExpansionUserObject
registerMooseObject("MooseApp", KLExpansionUserObject);

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
  params.addRequiredParam<std::vector<unsigned int>>(
      "n_grid", "Reference grid size, one entry per active dimension");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "covariance_functions", "One 1D covariance marginal UserObject name per active dimension");
  params.addParam<unsigned int>(
      "n_terms",
      0,
      "Number of joint KL terms to retain. Specify this OR variance_fraction, not both.");
  params.addParam<Real>(
      "variance_fraction",
      -1,
      "Fraction (0,1] of total spectral trace to retain (the fewest joint modes whose cumulative "
      "eigenvalue sum reaches this fraction are kept). Specify this OR n_terms, not both.");
  params.addParam<unsigned int>("seed", 0, "RNG seed - MUST be identical across all MPI ranks");
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  return params;
}

KLExpansionUserObject::KLExpansionUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _dim(getParam<std::vector<Real>>("lower_bounds").size()),
    _n_terms_param(getParam<unsigned int>("n_terms")),
    _variance_fraction(getParam<Real>("variance_fraction")),
    _seed(getParam<unsigned int>("seed"))
{
  if (_dim < 1 || _dim > 3)
    mooseError("KLExpansionUserObject supports 1 to 3 separable dimensions, got ", _dim);

  const bool have_n_terms = parameters.isParamSetByUser("n_terms");
  const bool have_fraction = parameters.isParamSetByUser("variance_fraction");

  if (have_n_terms == have_fraction) // both false or both true
    mooseError("KLExpansionUserObject: specify EXACTLY ONE of 'n_terms' or "
               "'variance_fraction', not both and not neither.");

  if (have_n_terms && _n_terms_param == 0)
    mooseError("KLExpansionUserObject: 'n_terms' must be greater than 0, got ", _n_terms_param);

  if (have_fraction && (_variance_fraction <= 0.0 || _variance_fraction > 1.0))
    mooseError("KLExpansionUserObject: 'variance_fraction' must be in the range (0, 1], got ",
               _variance_fraction);

  const auto & low = getParam<std::vector<Real>>("lower_bounds");
  const auto & high = getParam<std::vector<Real>>("upper_bounds");
  const auto & ng = getParam<std::vector<unsigned int>>("n_grid");
  const auto & cov_names = getParam<std::vector<UserObjectName>>("covariance_functions");

  if (high.size() != _dim || ng.size() != _dim || cov_names.size() != _dim)
    mooseError("lower_bounds, upper_bounds, n_grid, and covariance_functions must all have "
               "the same length, equal to the number of active dimensions (",
               _dim,
               ").");

  for (unsigned int d = 0; d < _dim; ++d)
  {
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
  for (unsigned int d = 0; d < _dim; ++d)
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
  for (unsigned int i = 0; i < n; ++i)
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
  for (unsigned int k = 0; k < n; ++k)
    for (unsigned int l = 0; l < n; ++l)
      C(k, l) = cov->computeCovariance(grid[k], grid[l]);

  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(C);
  if (solver.info() != Eigen::Success)
    mooseError("KLExpansionUserObject: eigen decomposition failed for dimension ", d);

  const auto & eigenvalues = solver.eigenvalues(); // ascending order, guaranteed real
  const auto & eigenvectors = solver.eigenvectors();

  _lambda[d].resize(n);
  _eigvec[d].resize(n);

  // eigenvalues() is ascending -> walk backwards from the end for descending order
  for (unsigned int m = 0; m < n; ++m)
  {
    const unsigned int idx = n - 1 - m;
    _lambda[d][m] = std::max(eigenvalues(idx), 0.0); // guard tiny negative roundoff
    _eigvec[d][m].resize(n);
    for (unsigned int l = 0; l < n; ++l)
      _eigvec[d][m][l] = eigenvectors(l, idx);
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
  for (unsigned int i = 0; i < n0; ++i)
    for (unsigned int j = 0; j < n1; ++j)
      for (unsigned int k = 0; k < n2; ++k)
      {
        Real eig = _lambda[0][i];
        if (_dim >= 2)
          eig *= _lambda[1][j];
        if (_dim >= 3)
          eig *= _lambda[2][k];
        candidates.push_back({eig, {i, j, k}});
        total_trace += eig;
      }

  std::sort(candidates.begin(),
            candidates.end(),
            [](const auto & a, const auto & b) { return a.first > b.first; });

  unsigned int keep = 0;
  if (_n_terms_param > 0)
  {
    if (_n_terms_param > candidates.size())
      mooseWarning("KLExpansionUserObject: requested n_terms = ",
                   _n_terms_param,
                   " exceeds the total number of available joint modes (",
                   candidates.size(),
                   " = product of marginal grid sizes). "
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
  for (unsigned int m = 0; m < keep; ++m)
  {
    _joint_index[m] = candidates[m].second;
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
  for (unsigned int l = 0; l < grid.size(); ++l)
    sum += cov->computeCovariance(s_star, grid[l]) * vec[l];

  return sum / std::sqrt(_lambda[d][mode]);
}

void
KLExpansionUserObject::computeModeValues(const Point & p, std::vector<Real> & mode_vals) const
{
  mode_vals.resize(_joint_index.size());
  for (unsigned int m = 0; m < _joint_index.size(); ++m)
  {
    Real a = 1.0;
    for (unsigned int d = 0; d < _dim; ++d)
      a *= nystromExtend(p(d), d, _joint_index[m][d]);
    mode_vals[m] = a;
  }
}

Real
KLExpansionUserObject::getValue(const Point & p) const
{
  std::vector<Real> a;
  computeModeValues(p, a);

  Real value = 0.0;
  for (unsigned int m = 0; m < a.size(); ++m)
    value += a[m] * _xi[m];
  return value;
}

Real
KLExpansionUserObject::getStandardizedValue(const Point & p) const
{
  std::vector<Real> a;
  computeModeValues(p, a);

  Real raw = 0.0, var = 0.0;
  for (unsigned int m = 0; m < a.size(); ++m)
  {
    raw += a[m] * _xi[m];
    var += a[m] * a[m];
  }

  const Real sigma = std::sqrt(var);
  if (sigma < 1e-14)
    return 0.0; // degenerate point (e.g. far outside reference-grid support). return 0 to avoid 0/0

  return raw / sigma;
}

Real
KLExpansionUserObject::getCopulaUniform(const Point & p) const
{
  const Real z = getStandardizedValue(p);
  // Standard normal CDF via erf: Phi(z) = 0.5*(1 + erf(z/sqrt(2)))
  Real u = 0.5 * (1.0 + std::erf(z / std::sqrt(2.0)));

  // eps <= u <= 1-eps for numerical reasons. If u =1, inverse transform for weibull will have
  // issues.
  const Real eps = 1e-12;
  u = std::min(std::max(u, eps), 1.0 - eps);
  return u;
}
