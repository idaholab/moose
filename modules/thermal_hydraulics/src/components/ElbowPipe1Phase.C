//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElbowPipe1Phase.h"
#include "THMMesh.h"

registerMooseObject("ThermalHydraulicsApp", ElbowPipe1Phase);

InputParameters
ElbowPipe1Phase::validParams()
{
  InputParameters params = FlowChannel1Phase::validParams();
  params.addRequiredParam<Real>("radius", "Radius of the pipe [m]");
  params.addRequiredParam<Real>("start_angle", "Angle at which the pipe starts [degrees]");
  params.addRequiredParam<Real>("end_angle", "Angle at which the pipe ends [degrees]");

  // Suppress length. Also need to set it to something, because it is required in the parent class
  params.set<std::vector<Real>>("length") = {0.0};
  params.suppressParameter<std::vector<Real>>("length");

  params.addClassDescription("Bent pipe for 1-phase flow");

  return params;
}

ElbowPipe1Phase::ElbowPipe1Phase(const InputParameters & params)
  : FlowChannel1Phase(params),
    _radius(getParam<Real>("radius")),
    _start_angle(getParam<Real>("start_angle")),
    _end_angle(getParam<Real>("end_angle"))
{
  if (_start_angle > _end_angle)
    _end_angle += 360.;
  _central_angle = _end_angle - _start_angle;
  if (_central_angle > 360)
    logError("The difference between the angle parameters 'end_angle' and 'start_angle' (",
             _central_angle,
             ") is greater than 360 degrees");

  _length = 2 * M_PI * _radius * (_central_angle / 360.);
  _lengths[0] = _length;
}

void
ElbowPipe1Phase::buildMeshNodes()
{
  Real arc_length = 2 * M_PI * _radius * (_central_angle / 360.);
  for (unsigned int i = 0; i < _node_locations.size(); i++)
  {
    // distance from the origin (to account for elements of different sizes)
    Point dist(_node_locations[i], 0., 0.);
    Real x_pos = dist.norm();
    Real alpha = (_start_angle + (_central_angle * x_pos / arc_length)) * M_PI / 180.;
    RealVectorValue p(_radius * cos(alpha), _radius * sin(alpha), 0);
    addNode(p);
  }
}
