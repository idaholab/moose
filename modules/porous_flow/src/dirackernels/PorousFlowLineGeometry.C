//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowLineGeometry.h"
#include "RayTracing.h"
#include "LineSegment.h"
#include "libmesh/utility.h"

#include <fstream>

InputParameters
PorousFlowLineGeometry::validParams()
{
  InputParameters params = DiracKernel::validParams();
  params.addParam<std::string>(
      "point_file",
      "",
      "The file containing the coordinates of the points and their weightings that approximate the "
      "line sink.  The physical meaning of the weightings depend on the scenario, eg, they may be "
      "borehole radii.  Each line in the file must contain a space-separated weight and "
      "coordinate, viz r x y z.  For boreholes, the last point in the file is defined as the "
      "borehole bottom, where the borehole pressure is bottom_pressure.  If your file contains "
      "just one point, you must also specify the line_length and line_direction parameters.  Note "
      "that you will get segementation faults if your points do not lie within your mesh!");
  params.addParam<ReporterName>(
      "x_coord_reporter",
      "reporter x-coordinate name of line sink.  This uses the reporter syntax <reporter>/<name>.  "
      "Each point must adhere to the same requirements as those that would be given if using "
      "point_file ");
  params.addParam<ReporterName>(
      "y_coord_reporter",
      "reporter y-coordinate name of line sink.  This uses the reporter syntax <reporter>/<name>.  "
      "Each point must adhere to the same requirements as those that would be given if using "
      "point_file ");
  params.addParam<ReporterName>(
      "z_coord_reporter",
      "reporter z-coordinate name of line sink.  This uses the reporter syntax <reporter>/<name>.  "
      "Each point must adhere to the same requirements as those that would be given if using "
      "point_file ");
  params.addParam<ReporterName>(
      "weight_reporter",
      "reporter weight name of line sink. This uses the reporter syntax <reporter>/<name>.  "
      "Each point must adhere to the same requirements as those that would be given if using "
      "point_file ");
  params.addRangeCheckedParam<Real>(
      "line_length",
      0.0,
      "line_length>=0",
      "Line length.  Note this is only used if there is only one point in the point_file.");
  params.addParam<RealVectorValue>(
      "line_direction",
      RealVectorValue(0.0, 0.0, 1.0),
      "Line direction.  Note this is only used if there is only one point in the point_file.");
  params.addParam<std::vector<Real>>(
      "line_base",
      "Line base point x,y,z coordinates.  This is the same format as a single-line point_file. "
      "Note this is only used if there is no point file specified.");
  params.addClassDescription("Approximates a polyline sink in the mesh using a number of Dirac "
                             "point sinks with given weightings that are read from a file");
  return params;
}

PorousFlowLineGeometry::PorousFlowLineGeometry(const InputParameters & parameters)
  : DiracKernel(parameters),
    ReporterInterface(this),
    _line_length(getParam<Real>("line_length")),
    _line_direction(getParam<RealVectorValue>("line_direction")),
    _point_file(getParam<std::string>("point_file")),
    _x_coord(isParamValid("x_coord_reporter") ? &getReporterValue<std::vector<Real>>(
                                                    "x_coord_reporter", REPORTER_MODE_REPLICATED)
                                              : &_xs),
    _y_coord(isParamValid("y_coord_reporter") ? &getReporterValue<std::vector<Real>>(
                                                    "y_coord_reporter", REPORTER_MODE_REPLICATED)
                                              : &_ys),
    _z_coord(isParamValid("z_coord_reporter") ? &getReporterValue<std::vector<Real>>(
                                                    "z_coord_reporter", REPORTER_MODE_REPLICATED)
                                              : &_zs),
    _weight(isParamValid("weight_reporter")
                ? &getReporterValue<std::vector<Real>>("weight_reporter", REPORTER_MODE_REPLICATED)
                : &_rs),
    _usingReporter(isParamValid("x_coord_reporter"))
{
  statefulPropertiesAllowed(true);

  const int checkInputFormat =
      int(isParamValid("line_base")) + int(!_point_file.empty()) + int(_usingReporter);

  if (checkInputFormat > 1)
    paramError("point_file",
               "PorousFlowLineGeometry: must specify only one of 'point_file' or 'line_base' or "
               "reporter based input");
  else if (checkInputFormat == 0)
    paramError(
        "point_file",
        "PorousFlowLineGeometry: must specify at least one of 'point_file' or 'line_base' or "
        "reporter based input");
}

void
PorousFlowLineGeometry::initialSetup()
{
  if (!_point_file.empty())
  {
    // open file
    std::ifstream file(_point_file.c_str());
    if (!file.good())
      paramError("point_file", "PorousFlowLineGeometry: Error opening file " + _point_file);

    // construct the arrays of weight, x, y and z
    std::vector<Real> scratch;
    while (parseNextLineReals(file, scratch))
    {
      if (scratch.size() >= 2)
      {
        _rs.push_back(scratch[0]);
        _xs.push_back(scratch[1]);
        if (scratch.size() >= 3)
          _ys.push_back(scratch[2]);
        else
          _ys.push_back(0.0);
        if (scratch.size() >= 4)
          _zs.push_back(scratch[3]);
        else
          _zs.push_back(0.0);
      }
    }
    file.close();
    calcLineLengths();
  }
  else if (_usingReporter)
  {
    if (_weight->size() != _x_coord->size() || _weight->size() != _y_coord->size() ||
        _weight->size() != _z_coord->size())
    {
      std::string errMsg =
          "The value and coordinate vectors are a different size.  \n"
          "There must be one value per coordinate.  If the sizes are \n"
          "zero, the reporter or reporter may not have been initialized with data \n"
          "before the Dirac Kernel is called.  \n"
          "Try setting \"execute_on = timestep_begin\" in the reporter being read. \n"
          "weight size = " +
          std::to_string(_weight->size()) +
          ";  x_coord size = " + std::to_string(_x_coord->size()) +
          ";  y_coord size = " + std::to_string(_y_coord->size()) +
          ";  z_coord size = " + std::to_string(_z_coord->size());

      mooseError(errMsg);
    }

    for (std::size_t i = 0; i < _x_coord->size(); ++i)
    {
      _rs.push_back(_weight->at(i));
      _xs.push_back(_x_coord->at(i));
      _ys.push_back(_y_coord->at(i));
      _zs.push_back(_z_coord->at(i));
    }
    calcLineLengths();
  }
  else
  {
    _line_base = getParam<std::vector<Real>>("line_base");
    if (_line_base.size() != _mesh.dimension() + 1)
      paramError("line_base",
                 "PorousFlowLineGeometry: wrong number of arguments - got ",
                 _line_base.size(),
                 ", expected ",
                 _mesh.dimension() + 1,
                 " '<weight> <x> [<y> [z]]'");

    for (size_t i = _line_base.size(); i < 4; i++)
      _line_base.push_back(0); // fill out zeros up to weight+3 dimensions

    // make sure line base point is inside the mesh
    Point start(_line_base[1], _line_base[2], _line_base[3]);
    Point end = start + _line_length * _line_direction / _line_direction.norm();
    auto pl = _subproblem.mesh().getPointLocator();
    pl->enable_out_of_mesh_mode();

    auto * elem = (*pl)(start);
    auto elem_id = elem ? elem->id() : libMesh::DofObject::invalid_id;
    _communicator.min(elem_id);
    if (elem_id == libMesh::DofObject::invalid_id)
      paramError("line_base", "base point ", start, " lies outside the mesh");

    elem = (*pl)(end);
    elem_id = elem ? elem->id() : libMesh::DofObject::invalid_id;
    _communicator.min(elem_id);
    if (elem_id == libMesh::DofObject::invalid_id)
      paramError("line_length", "length causes end point ", end, " to lie outside the mesh");

    regenPoints();
  }
}

void
PorousFlowLineGeometry::calcLineLengths()
{
  const int num_pts = _zs.size();
  if (num_pts == 0)
    mooseError("PorousFlowLineGeometry: No points found in input.\nIf using reporters, make sure "
               "they have data.");
  _bottom_point(0) = _xs[num_pts - 1];
  _bottom_point(1) = _ys[num_pts - 1];
  _bottom_point(2) = _zs[num_pts - 1];

  // construct the line-segment lengths between each point
  _half_seg_len.clear();
  _half_seg_len.resize(std::max(num_pts - 1, 1));
  for (unsigned int i = 0; i + 1 < _xs.size(); ++i)
  {
    _half_seg_len[i] = 0.5 * std::sqrt(Utility::pow<2>(_xs[i + 1] - _xs[i]) +
                                       Utility::pow<2>(_ys[i + 1] - _ys[i]) +
                                       Utility::pow<2>(_zs[i + 1] - _zs[i]));
    if (_half_seg_len[i] == 0)
      mooseError("PorousFlowLineGeometry: zero-segment length detected at (x,y,z) = ",
                 _xs[i],
                 " ",
                 _ys[i],
                 " ",
                 _zs[i],
                 "\n");
  }
  if (num_pts == 1)
    _half_seg_len[0] = _line_length;
}

void
PorousFlowLineGeometry::regenPoints()
{
  if (!_point_file.empty() || _usingReporter)
    return;

  // recalculate the auto-generated points:
  _rs.clear();
  _xs.clear();
  _ys.clear();
  _zs.clear();

  Point p0(_line_base[1], _line_base[2], _line_base[3]);
  Point p1 = p0 + _line_length * _line_direction / _line_direction.norm();

  // add point for each cell the line passes through
  auto ploc = _mesh.getPointLocator();
  std::vector<Elem *> elems;
  std::vector<LineSegment> segs;
  Moose::elementsIntersectedByLine(p0, p1, _mesh, *ploc, elems, segs);
  for (size_t i = 0; i < segs.size(); i++)
  {
    // elementsIntersectedByLine sometimes returns segments with coincident points - check for this:
    auto & seg = segs[i];
    if (seg.start() == seg.end())
      continue;

    auto middle = (seg.start() + seg.end()) * 0.5;
    _rs.push_back(_line_base[0]);
    _xs.push_back(middle(0));
    _ys.push_back(middle(1));
    _zs.push_back(middle(2));
  }

  // make the start point be the line base point
  _rs.front() = _line_base[0];
  _xs.front() = p0(0);
  _ys.front() = p0(1);
  _zs.front() = p0(2);

  // force the end point only if our line traverses more than one element
  if (segs.size() > 1)
  {
    _rs.back() = _line_base[0];
    _xs.back() = p1(0);
    _ys.back() = p1(1);
    _zs.back() = p1(2);
  }
  calcLineLengths();
}

void
PorousFlowLineGeometry::meshChanged()
{
  DiracKernel::meshChanged();
  regenPoints();
}

bool
PorousFlowLineGeometry::parseNextLineReals(std::ifstream & ifs, std::vector<Real> & myvec)
// reads a space-separated line of floats from ifs and puts in myvec
{
  std::string line;
  myvec.clear();
  bool gotline(false);
  if (getline(ifs, line))
  {
    gotline = true;

    // Harvest floats separated by whitespace
    std::istringstream iss(line);
    Real f;
    while (iss >> f)
    {
      myvec.push_back(f);
    }
  }
  return gotline;
}

void
PorousFlowLineGeometry::addPoints()
{
  // Add point using the unique ID "i", let the DiracKernel take
  // care of the caching.  This should be fast after the first call,
  // as long as the points don't move around.
  for (unsigned int i = 0; i < _x_coord->size(); i++)
    addPoint(Point(_x_coord->at(i), _y_coord->at(i), _z_coord->at(i)), i);
}
