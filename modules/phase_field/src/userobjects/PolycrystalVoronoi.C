/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
  params.addParam<unsigned int>("rand_seed", 0, "The random seed");
  params.addParam<bool>(
      "columnar_3D", false, "3D microstructure will be columnar in the z-direction?");

  return params;
}

PolycrystalVoronoi::PolycrystalVoronoi(const InputParameters & parameters)
  : PolycrystalUserObjectBase(parameters),
    _columnar_3D(getParam<bool>("columnar_3D")),
    _rand_seed(getParam<unsigned int>("rand_seed"))
{
}

unsigned int
PolycrystalVoronoi::getGrainBasedOnPoint(const Point & point) const
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

  return min_index;
}

void
PolycrystalVoronoi::precomputeGrainStructure()
{
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

  for (decltype(_grain_num) grain = 0; grain < _grain_num; grain++)
  {
    for (unsigned int i = 0; i < LIBMESH_DIM; i++)
      _centerpoints[grain](i) = _bottom_left(i) + _range(i) * MooseRandom::rand();
    if (_columnar_3D)
      _centerpoints[grain](2) = _bottom_left(2) + _range(2) * 0.5;
  }
}
