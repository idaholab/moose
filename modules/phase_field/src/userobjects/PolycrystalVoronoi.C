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
#include "DelimitedFileReader.h"

registerMooseObject("PhaseFieldApp", PolycrystalVoronoi);

InputParameters
PolycrystalVoronoi::validParams()
{
  InputParameters params = PolycrystalUserObjectBase::validParams();
  params.addClassDescription(
      "Random Voronoi tessellation polycrystal (used by PolycrystalVoronoiAction)");
  params.addParam<unsigned int>(
      "grain_num", 0, "Number of grains being represented by the order parameters");
  params.addParam<unsigned int>("rand_seed", 0, "The random seed");
  params.addParam<bool>(
      "columnar_3D", false, "3D microstructure will be columnar in the z-direction?");
  params.addParam<bool>(
      "use_kdtree", false, "Whether or not to use a KD tree to speedup grain search");
  params.addRangeCheckedParam<unsigned int>("point_patch_size",
                                            1,
                                            "point_patch_size > 0",
                                            "How many nearest points KDTree should return");
  params.addRangeCheckedParam<unsigned int>("grain_patch_size",
                                            10,
                                            "grain_patch_size > 0",
                                            "How many nearest grains KDTree should return");
  params.addParam<FileName>(
      "file_name",
      "",
      "File containing grain centroids, if file_name is provided, the centroids "
      "from the file will be used.");
  params.addParam<Real>("int_width", 0.0, "Width of diffuse interfaces");
  return params;
}

PolycrystalVoronoi::PolycrystalVoronoi(const InputParameters & parameters)
  : PolycrystalUserObjectBase(parameters),
    _grain_num(getParam<unsigned int>("grain_num")),
    _columnar_3D(getParam<bool>("columnar_3D")),
    _rand_seed(getParam<unsigned int>("rand_seed")),
    _int_width(getParam<Real>("int_width")),
    _file_name(getParam<FileName>("file_name")),
    _kd_tree(nullptr),
    _use_kdtree(getParam<bool>("use_kdtree")),
    _point_patch_size(getParam<unsigned int>("point_patch_size")),
    _grain_patch_size(getParam<unsigned int>("grain_patch_size"))
{
  if (_file_name == "" && _grain_num == 0)
    mooseError("grain_num must be provided if the grain centroids are not read from a file");

  if (_file_name != "" && _grain_num > 0)
    mooseWarning("grain_num is ignored and will be determined from the file.");
}

void
PolycrystalVoronoi::getGrainsBasedOnPoint(const Point & point,
                                          std::vector<unsigned int> & grains) const
{
  grains.resize(0);
  Real d_min = _range.norm();
  Real distance;
  auto n_grains = _centerpoints.size();
  auto min_index = n_grains;

  if (_use_kdtree)
  {
    mooseAssert(_kd_tree, "KD tree is not constructed yet");
    mooseAssert(_grain_gtl_ids.size() == _new_points.size(),
                "The number of grain global IDs does not match that of new center points");

    std::vector<std::size_t> return_index(_point_patch_size);
    std::vector<Real> return_dist_sqr(_point_patch_size);

    _kd_tree->neighborSearch(point, _point_patch_size, return_index, return_dist_sqr);

    min_index = _grain_gtl_ids[return_index[0]];

    d_min = return_dist_sqr[0];

    // By default, _point_patch_size is one. A larger _point_patch_size
    // may be useful if nearest nodes are not unique, and
    // we want to select the node that has the smallest ID
    for (unsigned int i = 1; i < _point_patch_size; i++)
    {
      if (d_min > return_dist_sqr[i])
      {
        min_index = _grain_gtl_ids[return_index[i]];
        d_min = return_dist_sqr[i];
      }
      else if (d_min == return_dist_sqr[i])
      {
        min_index = min_index > _grain_gtl_ids[return_index[i]] ? _grain_gtl_ids[return_index[i]]
                                                                : min_index;
      }
    }
  }
  else
  {
    // Find the closest centerpoint to the current point
    for (MooseIndex(_centerpoints) grain = 0; grain < n_grains; ++grain)
    {
      distance = _mesh.minPeriodicDistance(_vars[0]->number(), _centerpoints[grain], point);
      if (distance < d_min)
      {
        d_min = distance;
        min_index = grain;
      }
    }
  }

  mooseAssert(min_index < n_grains, "Couldn't find closest Voronoi cell");
  Point current_grain = _centerpoints[min_index];
  grains.push_back(min_index); // closest centerpoint always gets included

  if (_int_width > 0.0)
  {
    if (_use_kdtree)
    {
      mooseAssert(_grain_patch_size < _grain_gtl_ids.size(),
                  "Number of neighboring grains should not exceed number of global grains");

      std::vector<std::size_t> return_index(_grain_patch_size);
      _kd_tree->neighborSearch(current_grain, _grain_patch_size, return_index);

      std::set<dof_id_type> neighbor_grains;
      for (unsigned int i = 0; i < _grain_patch_size; i++)
        if (_grain_gtl_ids[return_index[i]] != min_index)
          neighbor_grains.insert(_grain_gtl_ids[return_index[i]]);

      for (auto it = neighbor_grains.begin(); it != neighbor_grains.end(); ++it)
        if ((*it) != min_index)
        {
          Point next_grain = _centerpoints[*it];
          Point N = findNormalVector(point, current_grain, next_grain);
          Point cntr = findCenterPoint(point, current_grain, next_grain);
          distance = N * (cntr - point);
          if (distance < _int_width)
            grains.push_back(*it); // also include all grains with nearby boundaries
        }
    }
    else
    {
      for (MooseIndex(_centerpoints) grain = 0; grain < n_grains; ++grain)
        if (grain != min_index)
        {
          Point next_grain = _centerpoints[grain];
          Point N = findNormalVector(point, current_grain, next_grain);
          Point cntr = findCenterPoint(point, current_grain, next_grain);
          distance = N * (cntr - point);
          if (distance < _int_width)
            grains.push_back(grain); // also include all grains with nearby boundaries
        }
    }
  }
}

Real
PolycrystalVoronoi::getVariableValue(unsigned int op_index, const Point & p) const
{
  std::vector<unsigned int> grain_ids;
  getGrainsBasedOnPoint(p, grain_ids);

  // Now see if any of those grains are represented by the passed in order parameter
  unsigned int active_grain_on_op = invalid_id;
  for (auto grain_id : grain_ids)
    if (op_index == _grain_to_op.at(grain_id))
    {
      active_grain_on_op = grain_id;
      break;
    }

  Real profile_val = 0.0;
  if (active_grain_on_op != invalid_id)
    profile_val = computeDiffuseInterface(p, active_grain_on_op, grain_ids);

  return profile_val;
}

void
PolycrystalVoronoi::precomputeGrainStructure()
{
  // Set up domain bounds with mesh tools
  for (const auto i : make_range(Moose::dim))
  {
    _bottom_left(i) = _mesh.getMinInDimension(i);
    _top_right(i) = _mesh.getMaxInDimension(i);
  }
  _range = _top_right - _bottom_left;

  if (!_file_name.empty())
  {
    MooseUtils::DelimitedFileReader txt_reader(_file_name, &_communicator);

    txt_reader.read();
    std::vector<std::string> col_names = txt_reader.getNames();
    std::vector<std::vector<Real>> data = txt_reader.getData();
    _grain_num = data[0].size();
    _centerpoints.resize(_grain_num);

    for (unsigned int i = 0; i < col_names.size(); ++i)
    {
      // Check vector lengths
      if (data[i].size() != _grain_num)
        paramError("Columns in ", _file_name, " do not have uniform lengths.");
    }

    for (unsigned int grain = 0; grain < _grain_num; ++grain)
    {
      _centerpoints[grain](0) = data[0][grain];
      _centerpoints[grain](1) = data[1][grain];
      if (col_names.size() > 2)
        _centerpoints[grain](2) = data[2][grain];
      else
        _centerpoints[grain](2) = 0.0;
    }
  }
  else
  {
    MooseRandom::seed(_rand_seed);

    // Randomly generate the centers of the individual grains represented by the Voronoi
    // tessellation
    _centerpoints.resize(_grain_num);
    std::vector<Real> distances(_grain_num);

    for (auto grain = decltype(_grain_num)(0); grain < _grain_num; grain++)
    {
      for (const auto i : make_range(Moose::dim))
        _centerpoints[grain](i) = _bottom_left(i) + _range(i) * MooseRandom::rand();
      if (_columnar_3D)
        _centerpoints[grain](2) = _bottom_left(2) + _range(2) * 0.5;
    }
  }

  // Build a KDTree that is used to speedup point search
  buildSearchTree();
}

void
PolycrystalVoronoi::buildSearchTree()
{
  if (!_use_kdtree)
    return;

  auto midplane = _bottom_left + _range / 2.0;
  dof_id_type local_grain_id = 0;
  _grain_gtl_ids.clear();
  _grain_gtl_ids.reserve(_centerpoints.size() * std::pow(2.0, _mesh.dimension()));
  _new_points.clear();
  // Use more memory. Factor is 2^dim
  _new_points.reserve(_centerpoints.size() * std::pow(2.0, _mesh.dimension()));
  // Domain will be extended along the periodic directions
  // For each direction, a half domain is constructed
  std::vector<std::vector<Real>> xyzs(LIBMESH_DIM);
  for (auto & point : _centerpoints)
  {
    // Cear up
    for (unsigned int i = 0; i < LIBMESH_DIM; i++)
      xyzs[i].clear();

    // Have at least the original point
    for (unsigned int i = 0; i < LIBMESH_DIM; i++)
      xyzs[i].push_back(point(i));

    // Add new coords when there exists periodic boundary conditions
    // We extend half domain
    for (unsigned int i = 0; i < _mesh.dimension(); i++)
      if (_mesh.isTranslatedPeriodic(_vars[0]->number(), i))
        xyzs[i].push_back(point(i) <= midplane(i) ? point(i) + _range(i) : point(i) - _range(i));

    // Construct all combinations
    for (auto x : xyzs[0])
      for (auto y : xyzs[1])
        for (auto z : xyzs[2])
        {
          _new_points.push_back(Point(x, y, z));
          _grain_gtl_ids.push_back(local_grain_id);
        }

    local_grain_id++;
  }

  // Build a KDTree that is used to speedup point search
  // We should not destroy _new_points after the tree is contributed
  // KDTree use a reference to "_new_points"
  _kd_tree = std::make_unique<KDTree>(_new_points, _mesh.getMaxLeafSize());
}

Real
PolycrystalVoronoi::computeDiffuseInterface(const Point & point,
                                            const unsigned int & gr_index,
                                            const std::vector<unsigned int> & grain_ids) const
{
  Real val = 1.0;
  Point current_grain = _centerpoints[gr_index];
  for (auto i : grain_ids)
    if (i != gr_index)
    {
      Point next_grain = _centerpoints[i];
      Point N = findNormalVector(point, current_grain, next_grain);
      Point cntr = findCenterPoint(point, current_grain, next_grain);
      for (unsigned int vcomp = 0; vcomp < 3; ++vcomp)
        if (N(vcomp) != 0.0)
        {
          Real L = findLinePoint(point, N, cntr, vcomp);
          val *= 0.5 * (1.0 - std::tanh(2.0 * (point(vcomp) - L) * N(vcomp) / _int_width));
          break;
        }
    }
  return val;
}

Point
PolycrystalVoronoi::findNormalVector(const Point & point, const Point & p1, const Point & p2) const
{
  Point pa = point + _mesh.minPeriodicVector(_vars[0]->number(), point, p1);
  Point pb = point + _mesh.minPeriodicVector(_vars[0]->number(), point, p2);
  Point N = pb - pa;
  return N / N.norm();
}

Point
PolycrystalVoronoi::findCenterPoint(const Point & point, const Point & p1, const Point & p2) const
{
  Point pa = point + _mesh.minPeriodicVector(_vars[0]->number(), point, p1);
  Point pb = point + _mesh.minPeriodicVector(_vars[0]->number(), point, p2);
  return 0.5 * (pa + pb);
}

Real
PolycrystalVoronoi::findLinePoint(const Point & point,
                                  const Point & N,
                                  const Point & cntr,
                                  const unsigned int vcomp) const
{
  const Real l_sum = N((vcomp + 1) % 3) * (point((vcomp + 1) % 3) - cntr((vcomp + 1) % 3)) +
                     N((vcomp + 2) % 3) * (point((vcomp + 2) % 3) - cntr((vcomp + 2) % 3));

  return cntr(vcomp) - l_sum / N(vcomp);
}
