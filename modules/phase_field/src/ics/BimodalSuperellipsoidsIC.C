//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BimodalSuperellipsoidsIC.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariable.h"

registerMooseObject("PhaseFieldApp", BimodalSuperellipsoidsIC);

InputParameters
BimodalSuperellipsoidsIC::validParams()
{
  InputParameters params = SpecifiedSmoothSuperellipsoidIC::validParams();
  params.addClassDescription("Bimodal size distribution of large particles (specified in input "
                             "file) and small particles (placed randomly outside the larger "
                             "particles)");
  params.addRequiredParam<unsigned int>("npart", "The number of random (small) particles to place");
  params.addRequiredParam<Real>(
      "small_spac",
      "minimum spacing between small particles, measured from closest edge to closest edge");
  params.addRequiredParam<Real>("large_spac",
                                "minimum spacing between large and small particles, "
                                "measured from closest edge to closest edge");
  params.addRequiredParam<Real>(
      "small_a", "Mean semiaxis a value for the randomly placed (small) superellipsoids");
  params.addRequiredParam<Real>(
      "small_b", "Mean semiaxis b value for the randomly placed (small) superellipsoids");
  params.addRequiredParam<Real>(
      "small_c", "Mean semiaxis c value for the randomly placed (small) superellipsoids");
  params.addRequiredParam<Real>("small_n",
                                "Exponent n for the randomly placed (small) superellipsoids");
  params.addParam<Real>("size_variation",
                        0.0,
                        "Plus or minus fraction of random variation in the "
                        "semiaxes for uniform, standard deviation for "
                        "normal");
  MooseEnum rand_options("uniform normal none", "none");
  params.addParam<MooseEnum>(
      "size_variation_type", rand_options, "Type of distribution that random semiaxes will follow");
  params.addParam<unsigned int>(
      "numtries", 1000, "The number of tries to place the random particles");
  return params;
}

BimodalSuperellipsoidsIC::BimodalSuperellipsoidsIC(const InputParameters & parameters)
  : SpecifiedSmoothSuperellipsoidIC(parameters),
    _npart(getParam<unsigned int>("npart")),
    _small_spac(getParam<Real>("small_spac")),
    _large_spac(getParam<Real>("large_spac")),
    _small_a(getParam<Real>("small_a")),
    _small_b(getParam<Real>("small_b")),
    _small_c(getParam<Real>("small_c")),
    _small_n(getParam<Real>("small_n")),
    _size_variation(getParam<Real>("size_variation")),
    _size_variation_type(getParam<MooseEnum>("size_variation_type")),
    _max_num_tries(getParam<unsigned int>("numtries"))
{
}

void
BimodalSuperellipsoidsIC::initialSetup()
{
  // Set up domain bounds with mesh tools
  for (const auto i : make_range(Moose::dim))
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;

  if (_size_variation_type == 2 && _size_variation > 0.0)
    mooseError("If size_variation > 0.0, you must pass in a size_variation_type in "
               "BimodalSuperellipsoidsIC");

  SmoothSuperellipsoidBaseIC::initialSetup();
}

void
BimodalSuperellipsoidsIC::computeSuperellipsoidSemiaxes()
{
  _as.resize(_input_as.size() + _npart);
  _bs.resize(_input_bs.size() + _npart);
  _cs.resize(_input_cs.size() + _npart);

  // First fill in the specified (large) superellipsoids from the input file
  for (unsigned int i = 0; i < _input_as.size(); ++i)
  {
    _as[i] = _input_as[i];
    _bs[i] = _input_bs[i];
    _cs[i] = _input_cs[i];
  }

  // Then fill in the randomly positioned (small) superellipsoids
  for (unsigned int i = _input_as.size(); i < _input_as.size() + _npart; ++i)
  {
    // Vary semiaxes
    switch (_size_variation_type)
    {
      case 0: // Random distrubtion, maintaining constant shape
      {
        Real rand_num = _random.rand(_tid);
        _as[i] = _small_a * (1.0 + (1.0 - 2.0 * rand_num) * _size_variation);
        _bs[i] = _small_b * (1.0 + (1.0 - 2.0 * rand_num) * _size_variation);
        _cs[i] = _small_c * (1.0 + (1.0 - 2.0 * rand_num) * _size_variation);
        break;
      }
      case 1: // Normal distribution of semiaxis size, maintaining constant shape
        _as[i] = _random.randNormal(_tid, _small_a, _size_variation);
        _bs[i] = _as[i] * _small_b / _small_a;
        _cs[i] = _as[i] * _small_c / _small_a;
        break;
      case 2: // No variation
        _as[i] = _small_a;
        _bs[i] = _small_b;
        _cs[i] = _small_c;
    }

    if (_as[i] < 0.0)
      _as[i] = 0.0;
    if (_bs[i] < 0.0)
      _bs[i] = 0.0;
    if (_cs[i] < 0.0)
      _cs[i] = 0.0;
  }
}

void
BimodalSuperellipsoidsIC::computeSuperellipsoidExponents()
{
  _ns.resize(_input_ns.size() + _npart);

  // First fill in the specified (large) superellipsoids from the input file
  for (unsigned int i = 0; i < _input_ns.size(); ++i)
    _ns[i] = _input_ns[i];

  // Then fill in the randomly positioned (small) superellipsoids
  // The shape is assumed to stay constant so n does not vary
  for (unsigned int i = _input_ns.size(); i < _input_ns.size() + _npart; ++i)
    _ns[i] = _small_n;
}

void
BimodalSuperellipsoidsIC::computeSuperellipsoidCenters()
{
  _centers.resize(_x_positions.size() + _npart);

  // First place the specified (large) particles from the input file
  for (unsigned int i = 0; i < _x_positions.size(); ++i)
  {
    _centers[i](0) = _x_positions[i];
    _centers[i](1) = _y_positions[i];
    _centers[i](2) = _z_positions[i];
  }

  // Next place the randomly positioned (small) particles
  for (unsigned int i = _x_positions.size(); i < _x_positions.size() + _npart; ++i)
  {
    unsigned int num_tries = 0;

    while (num_tries < _max_num_tries)
    {
      num_tries++;

      RealTensorValue ran;
      ran(0, 0) = _random.rand(_tid);
      ran(1, 1) = _random.rand(_tid);
      ran(2, 2) = _random.rand(_tid);

      _centers[i] = _bottom_left + ran * _range;

      // check for collisions with the specified (large) and randomly placed small particles
      for (unsigned int j = 0; j < i; ++j)
      {
        // Compute the distance r1 from the center of each specified superellipsoid to its
        // outside edge along the vector between the specified superellipsoid and the current
        // randomly positioned one.
        // This uses the equation for a superellipse in polar coordinates and substitutes
        // distances for sin, cos functions.
        Point dist_vec = _mesh.minPeriodicVector(_var.number(), _centers[i], _centers[j]);
        const Real dist = dist_vec.norm();

        // First calculate rmn1 = r1^(-n), replacing sin, cos functions with distances
        Real rmn1 = (std::pow(std::abs(dist_vec(0) / dist / _as[j]), _ns[j]) +
                     std::pow(std::abs(dist_vec(1) / dist / _bs[j]), _ns[j]) +
                     std::pow(std::abs(dist_vec(2) / dist / _cs[j]), _ns[j]));
        // Then calculate r1 from rmn1
        const Real r1 = std::pow(rmn1, (-1.0 / _ns[j]));

        // Now calculate the distance r2 from the center of the randomly placed
        // superellipsoid to its outside edge in the same manner
        Real rmn2 = (std::pow(std::abs(dist_vec(0) / dist / _as[i]), _ns[i]) +
                     std::pow(std::abs(dist_vec(1) / dist / _bs[i]), _ns[i]) +
                     std::pow(std::abs(dist_vec(2) / dist / _cs[i]), _ns[i]));
        const Real r2 = std::pow(rmn2, (-1.0 / _ns[i]));

        // Calculate the distance between the edges (first in the list are the large then come the
        // small)
        if ((dist - r1 - r2) < (j < _x_positions.size() ? _large_spac : _small_spac))
          goto fail;
      }

      // accept the position of the new center
      goto accept;

    // retry a new position until tries are exhausted
    fail:
      continue;
    }

    if (num_tries == _max_num_tries)
      mooseError("Too many tries in MultiSmoothCircleIC");

  accept:
    continue;
  }
}
