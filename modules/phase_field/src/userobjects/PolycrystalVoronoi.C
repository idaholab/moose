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
  grains.resize(0);
  Real d_min = _range.norm();
  Real distance;
  auto n_grains = _centerpoints.size();
  auto min_index = n_grains;

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
  mooseAssert(min_index < n_grains, "Couldn't find closest Voronoi cell");
  Point current_grain = _centerpoints[min_index];
  grains.push_back(min_index); // closest centerpoint always gets included

  if (_int_width > 0.0)
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

  Real profile_val = 0.0;
  if (active_grain_on_op != invalid_id)
    profile_val = computeDiffuseInterface(p, active_grain_on_op, grain_ids);

  return profile_val;
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
