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

template <>
InputParameters
validParams<PolycrystalVoronoi>()
{
  InputParameters params = validParams<PolycrystalUserObjectBase>();
  params.addClassDescription(
      "Random Voronoi tesselation polycrystal (used by PolycrystalVoronoiAction)");
  params.addParam<unsigned int>(
      "grain_num", 0, "Number of grains being represented by the order parameters");
  params.addParam<unsigned int>("rand_seed", 0, "The random seed");
  params.addParam<bool>(
      "columnar_3D", false, "3D microstructure will be columnar in the z-direction?");
  params.addParam<FileName>(
      "file_name",
      "",
      "File containing grain centroids, if file_name is provided, the centroids "
      "from the file will be used.");
  return params;
}

PolycrystalVoronoi::PolycrystalVoronoi(const InputParameters & parameters)
  : PolycrystalUserObjectBase(parameters),
    _grain_num(getParam<unsigned int>("grain_num")),
    _columnar_3D(getParam<bool>("columnar_3D")),
    _rand_seed(getParam<unsigned int>("rand_seed")),
    _file_name(getParam<FileName>("file_name"))
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
  // Set up domain bounds with mesh tools
  for (unsigned int i = 0; i < LIBMESH_DIM; i++)
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
      for (unsigned int i = 0; i < LIBMESH_DIM; i++)
        _centerpoints[grain](i) = _bottom_left(i) + _range(i) * MooseRandom::rand();
      if (_columnar_3D)
        _centerpoints[grain](2) = _bottom_left(2) + _range(2) * 0.5;
    }
  }
}
