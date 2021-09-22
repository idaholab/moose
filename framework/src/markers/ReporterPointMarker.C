//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterPointMarker.h"

registerMooseObject("MooseApp", ReporterPointMarker);

InputParameters
ReporterPointMarker::validParams()
{
  InputParameters params = Marker::validParams();
  params.addClassDescription("Marks the region inside or empty if it contains a reporter defined "
                             "point for refinement or coarsening.");
  params.addRequiredParam<ReporterName>("x_coord_name", "reporter x-coordinate name");
  params.addRequiredParam<ReporterName>("y_coord_name", "reporter y-coordinate name");
  params.addRequiredParam<ReporterName>("z_coord_name", "reporter z-coordinate name");
  MooseEnum marker_states = Marker::markerStates();
  params.addRequiredParam<MooseEnum>(
      "inside", marker_states, "How to mark elements containing a point");
  params.addRequiredParam<MooseEnum>(
      "empty", marker_states, "How to mark elements not containing a point");
  return params;
}

ReporterPointMarker::ReporterPointMarker(const InputParameters & parameters)
  : Marker(parameters),
    ReporterInterface(this),
    _inside(parameters.get<MooseEnum>("inside").getEnum<MarkerValue>()),
    _empty(parameters.get<MooseEnum>("empty").getEnum<MarkerValue>()),
    _x_coord(getReporterValue<std::vector<Real>>("x_coord_name", REPORTER_MODE_REPLICATED)),
    _y_coord(getReporterValue<std::vector<Real>>("y_coord_name", REPORTER_MODE_REPLICATED)),
    _z_coord(getReporterValue<std::vector<Real>>("z_coord_name", REPORTER_MODE_REPLICATED))
{
}

void
ReporterPointMarker::markerSetup()
{
  _pl = _fe_problem.mesh().getPointLocator();
  _pl->enable_out_of_mesh_mode();
  const auto npoints = _x_coord.size();
  if (npoints != _y_coord.size() || npoints != _z_coord.size())
    mooseError("The coordinate vectors are a different size.  \n",
               "   x_coord size = ",
               npoints,
               ";  y_coord size = ",
               _y_coord.size(),
               ";  z_coord size = ",
               _z_coord.size());

  _point_elems.clear();
  for (std::size_t i = 0; i < npoints; ++i)
  {
    Point pt(_x_coord[i], _y_coord[i], _z_coord[i]);
    const auto elem = (*_pl)(pt);
    if (elem)
      _point_elems.insert(elem->id());
  }
}

Marker::MarkerValue
ReporterPointMarker::computeElementMarker()
{
  return (_point_elems.count(_current_elem->id()) ? _inside : _empty);
}
