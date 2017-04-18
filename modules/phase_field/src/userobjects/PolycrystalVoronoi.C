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
#include "PolycrystalICTools.h"

template <>
InputParameters
validParams<PolycrystalVoronoi>()
{
  InputParameters params = validParams<PolycrystalUserObjectBase>();
  params.addClassDescription(
      "Random Voronoi tesselation polycrystal (used by PolycrystalVoronoiAction)");
  params.addRequiredParam<unsigned int>("op_num", "Number of order parameters");
  params.addRequiredParam<unsigned int>(
      "grain_num", "Number of grains being represented by the order parameters");
  params.addParam<unsigned int>("rand_seed", 0, "The random seed");
  params.addParam<bool>(
      "columnar_3D", false, "3D microstructure will be columnar in the z-direction?");
  params.addParam<MooseEnum>("coloring_algorithm",
                             PolycrystalUserObjectBase::coloringAlgorithms(),
                             PolycrystalUserObjectBase::coloringAlgorithmDescriptions());

  params.addRequiredCoupledVarWithAutoBuild(
      "variable", "var_name_base", "op_num", "Array of coupled variables");

  return params;
}

PolycrystalVoronoi::PolycrystalVoronoi(const InputParameters & parameters)
  : PolycrystalUserObjectBase(parameters),
    _columnar_3D(getParam<bool>("columnar_3D")),
    _rand_seed(getParam<unsigned int>("rand_seed"))
{
}

unsigned int
PolycrystalVoronoi::getGrainID(dof_id_type elem_id) const
{
  auto el_it = _elem_to_grain.find(elem_id);
  mooseAssert(el_it != _elem_to_grain.end(), "Element ID not found in map");

  return el_it->second;
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
PolycrystalVoronoi::initialSetup()
{
  // Set up domain bounds with mesh tools
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;
}

void
PolycrystalVoronoi::execute()
{

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

  mooseAssert(!_vars.empty(), "Coupled vars is empty");
  const auto end = _mesh.getMesh().semilocal_elements_end();
  for (auto el = _mesh.getMesh().semilocal_elements_begin(); el != end; ++el)
  {
    Point centroid = (*el)->centroid();
    unsigned int grain_index = getGrainBasedOnPoint(centroid);

    _elem_to_grain.insert(std::pair<dof_id_type, unsigned int>((*el)->id(), grain_index));
  }
}
