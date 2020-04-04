//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BimodalInverseSuperellipsoidsIC.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariable.h"

registerMooseObject("PhaseFieldApp", BimodalInverseSuperellipsoidsIC);

InputParameters
BimodalInverseSuperellipsoidsIC::validParams()
{
  InputParameters params = BimodalSuperellipsoidsIC::validParams();
  params.addClassDescription("Bimodal size distribution of large particles (specified in input "
                             "file, value invalue) and small particles (placed randomly inside the "
                             "larger particles, value outvalue)");
  return params;
}

BimodalInverseSuperellipsoidsIC::BimodalInverseSuperellipsoidsIC(const InputParameters & parameters)
  : BimodalSuperellipsoidsIC(parameters)
{
}

void
BimodalInverseSuperellipsoidsIC::initialSetup()
{
  if (_size_variation_type == 2 && _size_variation > 0.0)
    paramError("size_variation",
               "If size_variation > 0.0, you must pass in a size_variation_type in "
               "BimodalInverseSuperellipsoidsIC");

  BimodalSuperellipsoidsIC::initialSetup();
}

Real
BimodalInverseSuperellipsoidsIC::value(const Point & p)
{
  Real value = _outvalue;
  Real val2 = 0.0;

  // First loop over the specified superellipsoids
  for (unsigned int ellip = 0; ellip < _x_positions.size() && value != _invalue; ++ellip)
  {
    val2 = computeSuperellipsoidValue(
        p, _centers[ellip], _as[ellip], _bs[ellip], _cs[ellip], _ns[ellip]);
    if ((val2 > value && _invalue > _outvalue) || (val2 < value && _outvalue > _invalue))
      value = val2;
  }

  // Then loop over the randomly positioned particles and set value inside them back to outvalue
  for (unsigned int ellip = _x_positions.size(); ellip < _x_positions.size() + _npart; ++ellip)
  {
    val2 = computeSuperellipsoidInverseValue(
        p, _centers[ellip], _as[ellip], _bs[ellip], _cs[ellip], _ns[ellip]);
    if ((val2 < value && _invalue > _outvalue) || (val2 > value && _outvalue > _invalue))
      value = val2;
  }

  return value;
}

void
BimodalInverseSuperellipsoidsIC::computeSuperellipsoidCenters()
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

        if (j < _x_positions.size())
        {
          if (r1 - dist - r2 < _large_spac)
            goto fail;
        }
        else
        {
          if (dist - r1 - r2 < _small_spac)
            goto fail;
        }
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
