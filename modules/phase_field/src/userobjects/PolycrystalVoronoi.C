//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalVoronoi.h"
#include "IndirectSort.h"
#include "MooseRandom.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "NonlinearSystemBase.h"

template <>
InputParameters
validParams<PolycrystalVoronoi>()
{
  InputParameters params = validParams<PolycrystalUserObjectBase>();
  params.addClassDescription(
      "Random Voronoi tesselation polycrystal (used by PolycrystalVoronoiAction)");
  params.addRequiredParam<unsigned int>(
      "grain_num", "Number of grains being represented by the order parameters");

  params.addParam<unsigned int>("rand_seed", 0, "The random seed");
  params.addParam<bool>(
      "columnar_3D", false, "3D microstructure will be columnar in the z-direction?");

  return params;
}

PolycrystalVoronoi::PolycrystalVoronoi(const InputParameters & parameters)
  : PolycrystalUserObjectBase(parameters),
    _grain_num(getParam<unsigned int>("grain_num")),
    _columnar_3D(getParam<bool>("columnar_3D")),
    _rand_seed(getParam<unsigned int>("rand_seed"))
{
}

void
PolycrystalVoronoi::getGrainsBasedOnPoint(const Point & point,
                                          std::vector<unsigned int> & grains) const
{
  auto n_grains = _centerpoints.size();
  auto min_distance = _range.norm();
  auto min_index = n_grains;

  // Loops through all of the grain centers and finds the center that is closest to the point p
  for (auto grain = beginIndex(_centerpoints); grain < n_grains; ++grain)
  {
    auto distance = _mesh.minPeriodicDistance(_vars[0]->number(), _centerpoints[grain], point);

    if (distance < min_distance)
    {
      min_distance = distance;
      min_index = grain;
    }
  }

  mooseAssert(min_index < n_grains, "Couldn't find closest Voronoi cell");

  grains.resize(1);
  grains[0] = min_index;
}

Real
PolycrystalVoronoi::getVariableValue(unsigned int op_index, const Point & p) const
{
  std::vector<unsigned int> grain_ids;
  getGrainsBasedOnPoint(p, grain_ids);

  // Now see if any of those grains are represented by the passed in order parameter
  unsigned int active_grain_on_op = invalid_id;
  for (auto grain_id : grain_ids)
    if (op_index == _grain_to_op[grain_id])
    {
      active_grain_on_op = grain_id;
      break;
    }

  return active_grain_on_op != invalid_id ? 1.0 : 0.0;
}

void
PolycrystalVoronoi::precomputeGrainStructure()
{
  MooseRandom::seed(_rand_seed);

  // Set up domain bounds with mesh tools
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;

  // Randomly generate the centers of the individual grains represented by the Voronoi tessellation
  _centerpoints.resize(_grain_num);
  std::vector<Real> distances(_grain_num);

  for (auto grain = decltype(_grain_num)(0); grain < _grain_num; grain++)
  {
    for (unsigned int i = 0; i < LIBMESH_DIM; i++)
      _centerpoints[grain](i) = _bottom_left(i) + _range(i) * MooseRandom::rand();
    if (_columnar_3D)
      _centerpoints[grain](2) = _bottom_left(2) + _range(2) * 0.5;
  }
}
