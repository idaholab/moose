//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Creates multiple superellipsoids that are positioned randomly throughout the domain
// each semiaxis can be varied by a uniform or normal distribution

#include "MultiSmoothSuperellipsoidIC.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseUtils.h"
#include "MooseVariable.h"

registerMooseObject("PhaseFieldApp", MultiSmoothSuperellipsoidIC);

InputParameters
MultiSmoothSuperellipsoidIC::validParams()
{
  InputParameters params = SmoothSuperellipsoidBaseIC::validParams();
  params.addClassDescription("Random distribution of smooth ellipse with given minimum spacing");
  params.addRequiredParam<std::vector<unsigned int>>("numbub",
                                                     "Vector of the number of bubbles to place");
  params.addRequiredParam<std::vector<Real>>(
      "bubspac",
      "Vector of the minimum spacing of bubbles of one type, measured from center to center");
  params.addParam<unsigned int>("max_num_tries", 1000, "The number of tries");
  params.addRequiredParam<std::vector<Real>>(
      "semiaxis_a", "Vector of mean semiaxis values in the x direction for the ellipse");
  params.addRequiredParam<std::vector<Real>>(
      "semiaxis_b", "Vector of mean semiaxis values in the y direction for the ellipse");
  params.addRequiredParam<std::vector<Real>>("semiaxis_c",
                                             "Vector of mean semiaxis values in the z direction "
                                             "for the ellipse, must be greater than 0 even if 2D.");
  params.addParam<std::vector<Real>>(
      "exponent",
      std::vector<Real>(),
      "Vector of exponents for each superellipsoid, n=2 is a normal ellipse");
  params.addParam<std::vector<Real>>("semiaxis_a_variation",
                                     std::vector<Real>(),
                                     "Vector of plus or minus fractions of random variation in the "
                                     "bubble semiaxis in the x direction for uniform, standard "
                                     "deviation for normal");
  params.addParam<std::vector<Real>>("semiaxis_b_variation",
                                     std::vector<Real>(),
                                     "Vector of plus or minus fractions of random variation in the "
                                     "bubble semiaxis in the y direction for uniform, standard "
                                     "deviation for normal");
  params.addParam<std::vector<Real>>("semiaxis_c_variation",
                                     std::vector<Real>(),
                                     "Vector of plus or minus fractions of random variation in the "
                                     "bubble semiaxis in the z direction for uniform, standard "
                                     "deviation for normal. Must be set to 0 if 2D.");
  params.addParam<bool>("check_extremes",
                        false,
                        "Check all Superellipsoid extremes (center +- "
                        "each semiaxis) for overlap, must have "
                        "prevent_overlap set to True.");
  params.addParam<bool>("prevent_overlap",
                        false,
                        "Check all Superellipsoid centers for overlap with other superellipsoids.");
  params.addParam<bool>("vary_axes_independently",
                        true,
                        "If true the length of each semiaxis is randomly chosen "
                        "within the provided parameters, if false then one random "
                        "number is generated and applied to all semiaxes.");
  MooseEnum rand_options("uniform normal none", "none");
  params.addParam<MooseEnum>(
      "semiaxis_variation_type",
      rand_options,
      "Type of distribution that random superellipsoid semiaxes will follow");
  return params;
}

MultiSmoothSuperellipsoidIC::MultiSmoothSuperellipsoidIC(const InputParameters & parameters)
  : SmoothSuperellipsoidBaseIC(parameters),
    _max_num_tries(getParam<unsigned int>("max_num_tries")),
    _semiaxis_variation_type(getParam<MooseEnum>("semiaxis_variation_type")),
    _prevent_overlap(getParam<bool>("prevent_overlap")),
    _check_extremes(getParam<bool>("check_extremes")),
    _vary_axes_independently(getParam<bool>("vary_axes_independently")),
    _numbub(parameters.get<std::vector<unsigned int>>("numbub")),
    _bubspac(parameters.get<std::vector<Real>>("bubspac")),
    _exponent(parameters.get<std::vector<Real>>("exponent")),
    _semiaxis_a(parameters.get<std::vector<Real>>("semiaxis_a")),
    _semiaxis_b(parameters.get<std::vector<Real>>("semiaxis_b")),
    _semiaxis_c(parameters.get<std::vector<Real>>("semiaxis_c")),
    _semiaxis_a_variation(parameters.get<std::vector<Real>>("semiaxis_a_variation")),
    _semiaxis_b_variation(parameters.get<std::vector<Real>>("semiaxis_b_variation")),
    _semiaxis_c_variation(parameters.get<std::vector<Real>>("semiaxis_c_variation"))
{
}

void
MultiSmoothSuperellipsoidIC::initialSetup()
{
  unsigned int nv = _numbub.size();

  if (nv != _bubspac.size() || nv != _exponent.size() || nv != _semiaxis_a.size() ||
      nv != _semiaxis_b.size() || nv != _semiaxis_c.size())
    mooseError("Vectors for numbub, bubspac, exponent, semiaxis_a, semiaxis_b, and semiaxis_c must "
               "be the same size.");

  if (_semiaxis_variation_type != 2 &&
      (nv != _semiaxis_a_variation.size() || nv != _semiaxis_b_variation.size() ||
       nv != _semiaxis_c_variation.size()))
    mooseError("Vectors for numbub, semiaxis_a_variation, semiaxis_b_variation, and "
               "semiaxis_c_variation must be the same size.");

  if (_semiaxis_variation_type == 2 &&
      (_semiaxis_a_variation.size() > 0 || _semiaxis_b_variation.size() > 0 ||
       _semiaxis_c_variation.size() > 0))
    mooseWarning(
        "Values were provided for semiaxis_a/b/c_variation but semiaxis_variation_type is set "
        "to 'none' in 'MultiSmoothSuperellipsoidIC'.");

  for (_gk = 0; _gk < nv; ++_gk)
  {
    // Set up domain bounds with mesh tools
    for (const auto i : make_range(Moose::dim))
    {
      _bottom_left(i) = _mesh.getMinInDimension(i);
      _top_right(i) = _mesh.getMaxInDimension(i);
    }
    _range = _top_right - _bottom_left;

    SmoothSuperellipsoidBaseIC::initialSetup();
  }
}

void
MultiSmoothSuperellipsoidIC::computeSuperellipsoidSemiaxes()
{
  Real randnum;
  unsigned int start = _as.size();
  _as.resize(start + _numbub[_gk]);
  _bs.resize(start + _numbub[_gk]);
  _cs.resize(start + _numbub[_gk]);

  for (unsigned int i = start; i < _as.size(); i++)
  {
    switch (_semiaxis_variation_type)
    {
      case 0: // Uniform distrubtion
        randnum = _random.rand(_tid);
        _as[i] = _semiaxis_a[_gk] * (1.0 + (1.0 - 2.0 * randnum) * _semiaxis_a_variation[_gk]);
        _bs[i] = _semiaxis_b[_gk] *
                 (1.0 + (1.0 - 2.0 * (_vary_axes_independently ? _random.rand(_tid) : randnum)) *
                            _semiaxis_b_variation[_gk]);
        _cs[i] = _semiaxis_c[_gk] *
                 (1.0 + (1.0 - 2.0 * (_vary_axes_independently ? _random.rand(_tid) : randnum)) *
                            _semiaxis_c_variation[_gk]);
        break;

      case 1: // Normal distribution
        randnum = _random.randNormal(_tid, 0, 1);
        _as[i] = _semiaxis_a[_gk] + (randnum * _semiaxis_a_variation[_gk]);
        _bs[i] = _semiaxis_b[_gk] +
                 ((_vary_axes_independently ? _random.randNormal(_tid, 0, 1) : randnum) *
                  _semiaxis_b_variation[_gk]);
        _cs[i] = _semiaxis_c[_gk] +
                 ((_vary_axes_independently ? _random.randNormal(_tid, 0, 1) : randnum) *
                  _semiaxis_c_variation[_gk]);
        break;

      case 2: // No variation
        _as[i] = _semiaxis_a[_gk];
        _bs[i] = _semiaxis_b[_gk];
        _cs[i] = _semiaxis_c[_gk];
    }

    _as[i] = _as[i] < 0.0 ? 0.0 : _as[i];
    _bs[i] = _bs[i] < 0.0 ? 0.0 : _bs[i];
    _cs[i] = _cs[i] < 0.0 ? 0.0 : _cs[i];
  }
}

void
MultiSmoothSuperellipsoidIC::computeSuperellipsoidCenters()
{
  unsigned int start = _centers.size();
  _centers.resize(start + _numbub[_gk]);

  for (unsigned int i = start; i < _centers.size(); i++)
  {
    // Vary circle center positions
    unsigned int num_tries = 0;
    while (num_tries < _max_num_tries)
    {
      num_tries++;

      RealTensorValue ran;
      ran(0, 0) = _random.rand(_tid);
      ran(1, 1) = _random.rand(_tid);
      ran(2, 2) = _random.rand(_tid);

      _centers[i] = _bottom_left + ran * _range;

      for (unsigned int j = 0; j < i; ++j)
        if (_mesh.minPeriodicDistance(_var.number(), _centers[i], _centers[j]) < _bubspac[_gk] ||
            ellipsoidsOverlap(i, j))
          goto fail;

      // accept the position of the new center
      goto accept;

    // retry a new position until tries are exhausted
    fail:
      continue;
    }

    if (num_tries == _max_num_tries)
      mooseError("max_num_tries reached in 'MultiSmoothSuperellipsoidIC'.");

  accept:
    continue;
  }
}

void
MultiSmoothSuperellipsoidIC::computeSuperellipsoidExponents()
{
  unsigned int start = _ns.size();
  _ns.resize(start + _numbub[_gk]);

  for (unsigned int i = start; i < _ns.size(); ++i)
    _ns[i] = _exponent[_gk];
}

bool
MultiSmoothSuperellipsoidIC::ellipsoidsOverlap(unsigned int i, unsigned int j)
{
  // Check for overlap between centers
  const Point dist_vec = _mesh.minPeriodicVector(_var.number(), _centers[i], _centers[j]);
  const Real dist = dist_vec.norm();

  // Handle this case independently because we cannot calculate polar angles at this point
  if (MooseUtils::absoluteFuzzyEqual(dist, 0.0))
    return true;

  // Compute the distance r from the center of the superellipsoid to its outside edge
  // along the vector from the center to the current point
  // This uses the equation for a superellipse in polar coordinates and substitutes
  // distances for sin, cos functions
  Real rmn;

  // calculate rmn = r^(-n), replacing sin, cos functions with distances
  rmn = (std::pow(std::abs(dist_vec(0) / dist / _as[i]), _ns[i]) +
         std::pow(std::abs(dist_vec(1) / dist / _bs[i]), _ns[i]) +
         std::pow(std::abs(dist_vec(2) / dist / _cs[i]), _ns[i]));
  // calculate r2 from rmn
  const Real r1 = std::pow(rmn, (-1.0 / _ns[i]));

  // calculate rmn = r^(-n), replacing sin, cos functions with distances
  rmn = (std::pow(std::abs(dist_vec(0) / dist / _as[j]), _ns[j]) +
         std::pow(std::abs(dist_vec(1) / dist / _bs[j]), _ns[j]) +
         std::pow(std::abs(dist_vec(2) / dist / _cs[j]), _ns[j]));
  const Real r2 = std::pow(rmn, (-1.0 / _ns[j]));

  if (dist < r1 + r2)
    return true;

  // Check for overlap between extremes of new ellipsoid candidate and the center
  // of accepted ellisoids if _check_extremes enabled
  if (_check_extremes)
    return checkExtremes(i, j) || checkExtremes(j, i);

  // otherwise no overlap has been detected
  return false;
}

bool
MultiSmoothSuperellipsoidIC::checkExtremes(unsigned int i, unsigned int j)
{
  Point tmp_p;
  for (unsigned int pc = 0; pc < 6; pc++)
  {
    tmp_p = _centers[j];
    // Find extremes along semiaxis of candidate ellipsoids
    if (pc == 0)
      tmp_p(0) -= _as[j];
    else if (pc == 1)
      tmp_p(0) += _as[j];
    else if (pc == 2)
      tmp_p(1) -= _bs[j];
    else if (pc == 3)
      tmp_p(1) += _bs[j];
    else if (pc == 4)
      tmp_p(2) -= _cs[j];
    else
      tmp_p(2) += _cs[j];

    const Point dist_vec = _mesh.minPeriodicVector(_var.number(), _centers[i], tmp_p);
    const Real dist = dist_vec.norm();

    // Handle this case independently because we cannot calculate polar angles at this point
    if (MooseUtils::absoluteFuzzyEqual(dist, 0.0))
      return true;

    // calculate rmn = r^(-n), replacing sin, cos functions with distances
    Real rmn = (std::pow(std::abs(dist_vec(0) / dist / _as[i]), _ns[i]) +
                std::pow(std::abs(dist_vec(1) / dist / _bs[i]), _ns[i]) +
                std::pow(std::abs(dist_vec(2) / dist / _cs[i]), _ns[i]));
    Real r = std::pow(rmn, (-1.0 / _ns[i]));

    if (dist < r)
      return true;
  }

  return false;
}
