//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LatticeSmoothCircleIC.h"
#include "MooseMesh.h"

registerMooseObject("PhaseFieldApp", LatticeSmoothCircleIC);

InputParameters
LatticeSmoothCircleIC::validParams()
{
  InputParameters params = SmoothCircleBaseIC::validParams();
  params.addClassDescription("Perturbed square lattice of smooth circles");
  params.addDeprecatedParam<Real>("Rnd_variation",
                                  "Variation from central lattice position",
                                  "use the new parameter name pos_variation");
  params.addParam<Real>("pos_variation", 0.0, "Variation from central lattice position");
  params.addRequiredParam<std::vector<unsigned int>>(
      "circles_per_side", "Vector containing the number of bubbles along each side");
  params.addParam<unsigned int>("rand_seed", 2000, "random seed");
  params.addRequiredParam<Real>("radius", "Mean radius value for the circles");
  params.addParam<Real>(
      "radius_variation", 0.0, "Plus or minus fraction of random variation in the bubble radius");
  MooseEnum rand_options("uniform normal none", "none");
  params.addParam<MooseEnum>("radius_variation_type",
                             rand_options,
                             "Type of distribution that random circle radii will follow");
  params.addParam<bool>(
      "avoid_bounds", true, "Don't place any bubbles on the simulation cell boundaries");
  return params;
}

LatticeSmoothCircleIC::LatticeSmoothCircleIC(const InputParameters & parameters)
  : SmoothCircleBaseIC(parameters),
    _lattice_variation(isParamValid("Rnd_variation") ? getParam<Real>("Rnd_variation")
                                                     : getParam<Real>("pos_variation")),
    _circles_per_side(getParam<std::vector<unsigned int>>("circles_per_side")),
    _radius(getParam<Real>("radius")),
    _radius_variation(getParam<Real>("radius_variation")),
    _radius_variation_type(getParam<MooseEnum>("radius_variation_type")),
    _avoid_bounds(getParam<bool>("avoid_bounds"))
{
}

void
LatticeSmoothCircleIC::initialSetup()
{
  // pad circles per side vector to size 3 (with 0)
  _circles_per_side.resize(3);

  // Set up domain bounds with mesh tools
  for (const auto i : make_range(Moose::dim))
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;

  // Error checks
  if (_range(0) != 0.0 && _range(1) != 0.0 && _circles_per_side[1] == 0)
    mooseError("If domain is > 1D, circles_per_side must have more than one value");

  if (_range(2) != 0.0 && _circles_per_side[2] == 0)
    mooseError("If domain is 3D, circles_per_side must have three values");

  if (_range(1) == 0.0 && _range(2) == 0.0)
  {
    _circles_per_side[1] = 0;
    _circles_per_side[2] = 0;
  }

  // Set _numbub
  if (_range(2) == 0.0)
  {
    _circles_per_side[2] = 0;
    _numbub = _circles_per_side[0] * _circles_per_side[1];
  }
  else
    _numbub = _circles_per_side[0] * _circles_per_side[1] * _circles_per_side[2];

  switch (_radius_variation_type)
  {
    case 2: // No variation
      if (_radius_variation > 0.0)
        mooseError("If radius_variation > 0.0, you must pass in a radius_variation_type in "
                   "LatticeSmoothCircleIC");
      break;
  }
  SmoothCircleBaseIC::initialSetup();
}

void
LatticeSmoothCircleIC::computeCircleRadii()
{
  _radii.resize(_numbub);

  for (unsigned int i = 0; i < _numbub; i++)
  {
    // Vary bubble radius
    switch (_radius_variation_type)
    {
      case 0: // Uniform distrubtion
        _radii[i] = _radius * (1.0 + (1.0 - 2.0 * _random.rand(_tid)) * _radius_variation);
        break;
      case 1: // Normal distribution
        _radii[i] = _random.randNormal(_tid, _radius, _radius_variation);
        break;
      case 2: // No variation
        _radii[i] = _radius;
    }

    if (_radii[i] < 0.0)
      _radii[i] = 0.0;
  }
}

void
LatticeSmoothCircleIC::computeCircleCenters()
{
  _centers.resize(_numbub);

  Real x_sep = _range(0) / _circles_per_side[0];
  Real y_sep = _range(1) / _circles_per_side[1];

  Real z_sep = 0.0;
  unsigned int z_num = 1.0;

  if (_range(2) > 0.0)
  {
    z_sep = _range(2) / _circles_per_side[2];
    z_num = _circles_per_side[2];
  }

  unsigned int cnt = 0;
  for (unsigned int i = 0; i < _circles_per_side[0]; ++i)
    for (unsigned int j = 0; j < _circles_per_side[1]; ++j)
      for (unsigned int k = 0; k < z_num; ++k)
      {
        Real xx = x_sep / 2.0 + i * x_sep;
        Real yy = y_sep / 2.0 + j * y_sep;
        Real zz = z_sep / 2.0 + k * z_sep;

        // Vary circle position
        xx = xx + (1.0 - 2.0 * _random.rand(_tid)) * _lattice_variation;
        yy = yy + (1.0 - 2.0 * _random.rand(_tid)) * _lattice_variation;

        if (_range(2) != 0.0)
          zz = zz + (1.0 - 2.0 * _random.rand(_tid)) * _lattice_variation;

        // Verify not out of bounds
        if (_avoid_bounds && xx < _radii[cnt] + _int_width)
          xx = _radii[cnt] + _int_width;
        if (_avoid_bounds && xx > _range(0) - (_radii[cnt] + _int_width))
          xx = _range(0) - (_radii[cnt] + _int_width);
        if (_avoid_bounds && yy < _radii[cnt] + _int_width)
          yy = _radii[cnt] + _int_width;
        if (_avoid_bounds && yy > _range(1) - (_radii[cnt] + _int_width))
          yy = _range(1) - (_radii[cnt] + _int_width);
        if (_range(2) != 0.0)
        {
          if (_avoid_bounds && zz < _radii[cnt] + _int_width)
            zz = _radii[cnt] + _int_width;
          if (_avoid_bounds && zz > _range(2) - (_radii[cnt] + _int_width))
            zz = _range(2) - (_radii[cnt] + _int_width);
        }

        _centers[cnt](0) = xx;
        _centers[cnt](1) = yy;
        _centers[cnt](2) = zz;

        cnt++;
      }
}
