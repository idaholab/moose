//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MovingLineSegmentCutSetUserObject.h"

registerMooseObject("XFEMApp", MovingLineSegmentCutSetUserObject);

template <>
InputParameters
validParams<MovingLineSegmentCutSetUserObject>()
{
  InputParameters params = validParams<LineSegmentCutSetUserObject>();
  params.addRequiredParam<UserObjectName>("interface_velocity",
                                          "The name of userobject that computes the velocity.");
  params.addClassDescription(
      "Creates a UserObject for a moving line segment cut on 2D meshes for XFEM");
  return params;
}

MovingLineSegmentCutSetUserObject::MovingLineSegmentCutSetUserObject(
    const InputParameters & parameters)
  : LineSegmentCutSetUserObject(parameters)
{
}

const std::vector<Point>
MovingLineSegmentCutSetUserObject::getCrackFrontPoints(
    unsigned int /*num_crack_front_points*/) const
{
  mooseError("getCrackFrontPoints() is not implemented for this object.");
}

void
MovingLineSegmentCutSetUserObject::initialize()
{
  const UserObject * uo =
      &(_fe_problem.getUserObjectBase(getParam<UserObjectName>("interface_velocity")));

  if (dynamic_cast<const XFEMMovingInterfaceVelocityBase *>(uo) == nullptr)
    mooseError("UserObject casting to XFEMMovingInterfaceVelocityBase in "
               "MovingLineSegmentCutSetUserObject");

  _interface_velocity = dynamic_cast<const XFEMMovingInterfaceVelocityBase *>(uo);

  const_cast<XFEMMovingInterfaceVelocityBase *>(_interface_velocity)->initialize();
}

void
MovingLineSegmentCutSetUserObject::execute()
{
  std::vector<Real> cut_data_copy = _cut_data;

  const int line_cut_data_len = 6;

  if (_cut_data.size() % line_cut_data_len != 0)
    mooseError("Length of MovingLineSegmentCutSetUserObject cut_data must be a multiple of 6.");

  unsigned int num_cuts = _cut_data.size() / line_cut_data_len;

  if (_t_step > 1)
  {
    for (unsigned int i = 1; i < _interface_velocity->numberPoints(); ++i)
    {
      cut_data_copy[(i - 1) * line_cut_data_len + 0] +=
          _interface_velocity->computeMovingInterfaceVelocity((i - 1)) * _dt;
      cut_data_copy[(i - 1) * line_cut_data_len + 2] +=
          _interface_velocity->computeMovingInterfaceVelocity(i) * _dt;
    }
  }

  _cut_line_endpoints.clear();
  for (unsigned int i = 0; i < num_cuts; ++i)
  {
    Real x0 = cut_data_copy[i * line_cut_data_len + 0];
    Real y0 = cut_data_copy[i * line_cut_data_len + 1];
    Real x1 = cut_data_copy[i * line_cut_data_len + 2];
    Real y1 = cut_data_copy[i * line_cut_data_len + 3];
    _cut_line_endpoints.push_back(std::make_pair(Point(x0, y0, 0.0), Point(x1, y1, 0.0)));
  }

  GeometricCutUserObject::execute();
}

void
MovingLineSegmentCutSetUserObject::finalize()
{
  const int line_cut_data_len = 6;

  if (_t_step > 1)
  {
    for (unsigned int i = 1; i < _interface_velocity->numberPoints(); ++i)
    {
      _cut_data[(i - 1) * line_cut_data_len + 0] +=
          _interface_velocity->computeMovingInterfaceVelocity((i - 1)) * _dt;
      _cut_data[(i - 1) * line_cut_data_len + 2] +=
          _interface_velocity->computeMovingInterfaceVelocity(i) * _dt;
    }
  }

  GeometricCutUserObject::finalize();
}

Real
MovingLineSegmentCutSetUserObject::cutFraction(unsigned int /*cut_num*/, Real /*time*/) const
{
  return 1;
}
