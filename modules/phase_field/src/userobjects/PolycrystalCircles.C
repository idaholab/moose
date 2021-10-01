//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalCircles.h"
#include "MooseMesh.h"
#include "MooseVariable.h"

#include "libmesh/utility.h"
#include <fstream>

registerMooseObject("PhaseFieldApp", PolycrystalCircles);

InputParameters
PolycrystalCircles::validParams()
{
  InputParameters params = PolycrystalUserObjectBase::validParams();
  params.addClassDescription(
      "Polycrystal circles generated from a vector input or read from a file");
  params.addParam<bool>("read_from_file",
                        false,
                        "Set to true to read the position and radius "
                        "vectors from a file rather than inputing them "
                        "manually");
  params.addParam<bool>(
      "columnar_3D", false, "3D microstructure will be columnar in the z-direction?");
  params.addParam<std::vector<Real>>("x_positions", "x coordinate for each circle center");
  params.addParam<std::vector<Real>>("y_positions", "y coordinate for each circle center");
  params.addParam<std::vector<Real>>("z_positions", "z coordinate for each circle center");
  params.addParam<std::vector<Real>>("radii", "The radius for each circle");
  params.addParam<FileName>("file_name", "File containing circle centers and radii");
  params.addParam<Real>("int_width", 0.0, "Width of diffuse interface");

  return params;
}

PolycrystalCircles::PolycrystalCircles(const InputParameters & parameters)
  : PolycrystalUserObjectBase(parameters),
    _columnar_3D(getParam<bool>("columnar_3D")),
    _int_width(getParam<Real>("int_width")),
    _grain_num(0)
{
}

void
PolycrystalCircles::getGrainsBasedOnPoint(const Point & point,
                                          std::vector<unsigned int> & grains) const
{
  unsigned int n_grains = _centerpoints.size();
  grains.resize(0);

  for (unsigned int i = 0; i < n_grains; ++i)
  {
    Real distance = 0;

    if (_columnar_3D)
    {
      Real d_x = (point(0) - _centerpoints[i](0)) * (point(0) - _centerpoints[i](0));
      Real d_y = (point(1) - _centerpoints[i](1)) * (point(1) - _centerpoints[i](1));
      distance = std::sqrt(d_x + d_y);
    }
    else
      distance = _mesh.minPeriodicDistance(_vars[0]->number(), _centerpoints[i], point);

    if (distance < _radii[i] + _int_width)
      grains.push_back(i);
  }
}

Real
PolycrystalCircles::getVariableValue(unsigned int op_index, const Point & p) const
{
  std::vector<unsigned int> grain_ids;
  getGrainsBasedOnPoint(p, grain_ids);

  unsigned int active_grain_on_op = invalid_id;
  for (auto grain_id : grain_ids)
    if (op_index == _grain_to_op.at(grain_id))
    {
      active_grain_on_op = grain_id;
      break;
    }

  return active_grain_on_op != invalid_id ? computeDiffuseInterface(p, active_grain_on_op) : 0.0;
}

void
PolycrystalCircles::precomputeGrainStructure()
{
  bool readfromfile = getParam<bool>("read_from_file");
  if (readfromfile)
  {
    // Read file
    const FileName file_name = getParam<FileName>("file_name");
    MooseUtils::DelimitedFileReader txt_reader(file_name, &_communicator);

    txt_reader.read();
    std::vector<std::string> col_names = txt_reader.getNames();
    std::vector<std::vector<Real>> data = txt_reader.getData();
    _grain_num = data[0].size();
    _centerpoints.resize(_grain_num);

    std::array<int, 4> col_map = {{-1, -1, -1, -1}};

    for (unsigned int i = 0; i < col_names.size(); ++i)
    {
      // Check vector lengths
      if (data[i].size() != _grain_num)
        mooseError("Columns in ", file_name, " do not have uniform lengths.");

      // Map columns to variables
      if (col_names[i] == "x")
        col_map[X] = i;
      else if (col_names[i] == "y")
        col_map[Y] = i;
      else if (col_names[i] == "z")
        col_map[Z] = i;
      else if (col_names[i] == "r")
        col_map[R] = i;
    }

    // Check all columns are included
    if (col_map[X] == -1)
      mooseError("No column 'x' in ", file_name, ".");
    if (col_map[Y] == -1)
      mooseError("No column 'y' in ", file_name, ".");
    if (col_map[Z] == -1)
      mooseError("No column 'z' in ", file_name, ".");
    if (col_map[R] == -1)
      mooseError("No column 'r' in ", file_name, ".");

    // Write data to variables
    _radii.assign(data[col_map[R]].begin(), data[col_map[R]].end());
    for (unsigned int i = 0; i < _grain_num; ++i)
    {
      _centerpoints[i](0) = data[col_map[X]][i];
      _centerpoints[i](1) = data[col_map[Y]][i];
      _centerpoints[i](2) = data[col_map[Z]][i];
    }
  }
  else // if (readfromfile)
  {
    // Read vectors
    std::vector<Real> x_c = getParam<std::vector<Real>>("x_positions");
    std::vector<Real> y_c = getParam<std::vector<Real>>("y_positions");
    std::vector<Real> z_c = getParam<std::vector<Real>>("z_positions");
    std::vector<Real> r_c = getParam<std::vector<Real>>("radii");

    _grain_num = r_c.size();
    _centerpoints.resize(_grain_num);

    // Check vector lengths
    if (_grain_num != x_c.size())
      mooseError("The vector length of x_positions does not match the length of radii");
    else if (_grain_num != y_c.size())
      mooseError("The vector length of y_positions does not match the length of radii");
    else if (_grain_num != z_c.size())
      mooseError("The vector length of z_positions does not match the length of radii");

    // Assign values
    _radii.assign(r_c.begin(), r_c.end());
    for (unsigned int i = 0; i < _grain_num; ++i)
    {
      _centerpoints[i](0) = x_c[i];
      _centerpoints[i](1) = y_c[i];
      _centerpoints[i](2) = z_c[i];
    }
  }
}

Real
PolycrystalCircles::computeDiffuseInterface(const Point & p, const unsigned int & i) const
{
  if (_int_width == 0)
    return 1.0;

  Real d = 0;

  if (_columnar_3D)
  {
    Real d_x = (p(0) - _centerpoints[i](0)) * (p(0) - _centerpoints[i](0));
    Real d_y = (p(1) - _centerpoints[i](1)) * (p(1) - _centerpoints[i](1));
    d = std::sqrt(d_x + d_y);
  }
  else
    d = _mesh.minPeriodicDistance(_vars[0]->number(), _centerpoints[i], p);

  return 0.5 * (1 - std::tanh(2.0 * (d - _radii[i]) / _int_width));
}
