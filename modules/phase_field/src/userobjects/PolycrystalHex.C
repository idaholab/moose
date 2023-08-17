//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalHex.h"
#include "MooseRandom.h"
#include "MooseMesh.h"
#include "MathUtils.h"

registerMooseObject("PhaseFieldApp", PolycrystalHex);

InputParameters
PolycrystalHex::validParams()
{
  InputParameters params = PolycrystalVoronoi::validParams();
  params.addClassDescription("Perturbed hexagonal polycrystal");
  params.addParam<Real>("x_offset", 0.5, "Specifies offset of hexagon grid in x-direction");
  params.addRangeCheckedParam<Real>(
      "perturbation_percent",
      0.0,
      "perturbation_percent >= 0.0 & perturbation_percent <= 1.0",
      "The percent to randomly perturb centers of grains relative to the size of the grain");
  return params;
}

PolycrystalHex::PolycrystalHex(const InputParameters & parameters)
  : PolycrystalVoronoi(parameters),
    _x_offset(getParam<Real>("x_offset")),
    _perturbation_percent(getParam<Real>("perturbation_percent"))
{
  if (_columnar_3D == false && _dim == 3)
    mooseError(
        "PolycrystalHex is supported on 2D domains or 3D domains with the columnar_3D option");
  if (_grain_num % 2 != 0)
    mooseError("PolycrystalHex requires an even square number for 2D or columnar 3D");

  _random.seed(_tid, getParam<unsigned int>("rand_seed"));
}

void
PolycrystalHex::precomputeGrainStructure()
{
  unsigned int d = _dim;

  if (_columnar_3D && _dim == 3)
    d -= 1;

  // check if periodic boundary condition is set
  for (unsigned int j = 0; j < d; ++j)
  {
    for (unsigned int i = 0; i < _vars.size(); ++i)
      if (!_mesh.isTranslatedPeriodic(_vars[i]->number(), j))
        mooseError("PolycrystalHex works only with periodic BCs");
  }

  const unsigned int root = MathUtils::round(std::pow(_grain_num, 1.0 / d));

  // integer power the rounded root and check if we recover the grain number
  unsigned int grain_pow = root;
  for (unsigned int i = 1; i < d; ++i)
    grain_pow *= root;

  if (_grain_num != grain_pow)
    mooseError(
        "PolycrystalHex requires a square number for 2D or columnar 3D and a cubic number for 3D");

  // Set up domain bounds with mesh tools
  for (const auto i : make_range(Moose::dim))
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;

  _centerpoints.resize(_grain_num);

  std::vector<Real> distances(_grain_num);
  std::vector<Point> holder(_grain_num);

  const Real ndist = 1.0 / root;

  // Assign the relative center points positions, defining the grains according to a hexagonal
  // pattern
  unsigned int count = 0;

  for (unsigned int k = 0; k < (d == 3 ? root : 1); ++k)
    for (unsigned int j = 0; j < (d >= 2 ? root : 1); ++j)
      for (unsigned int i = 0; i < root; ++i)
      {
        // set x-coordinate
        holder[count](0) = i * ndist + (0.5 * ndist * (j % 2)) + _x_offset * ndist;

        // set y-coordinate
        holder[count](1) = j * ndist + (0.5 * ndist * (k % 2));

        // set z-coordinate
        holder[count](2) = k * ndist;

        // increment counter
        count++;
      }

  // Assign center point values
  for (unsigned int grain = 0; grain < _grain_num; ++grain)
    for (const auto i : make_range(Moose::dim))
    {
      if (_range(i) == 0)
        continue;

      Real perturbation_dist = (_range(i) / root * (_random.rand(_tid) * 2 - 1.0)) *
                               _perturbation_percent; // Perturb -100 to 100%
      _centerpoints[grain](i) = _bottom_left(i) + _range(i) * holder[grain](i) + perturbation_dist;

      if (_centerpoints[grain](i) > _top_right(i))
        _centerpoints[grain](i) = _top_right(i);
      if (_centerpoints[grain](i) < _bottom_left(i))
        _centerpoints[grain](i) = _bottom_left(i);

      if (_columnar_3D && _dim == 3)
        _centerpoints[grain](2) = _bottom_left(2) + _range(2) * 0.5;
    }

  buildSearchTree();
}
