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

InputParameters
MovingLineSegmentCutSetUserObject::validParams()
{
  InputParameters params = LineSegmentCutSetUserObject::validParams();
  params.addRequiredParam<UserObjectName>("interface_velocity",
                                          "The name of userobject that computes the velocity.");
  params.addClassDescription(
      "Creates a UserObject for a moving line segment cut on 2D meshes for XFEM");
  params.addParam<CutSubdomainID>(
      "negative_id", 0, "The CutSubdomainID corresponding to a non-positive signed distance");
  params.addParam<CutSubdomainID>(
      "positive_id", 1, "The CutSubdomainID corresponding to a positive signed distance");
  return params;
}

MovingLineSegmentCutSetUserObject::MovingLineSegmentCutSetUserObject(
    const InputParameters & parameters)
  : LineSegmentCutSetUserObject(parameters),
    _interface_velocity(&getUserObject<XFEMMovingInterfaceVelocityBase>("interface_velocity")),
    _negative_id(getParam<CutSubdomainID>("negative_id")),
    _positive_id(getParam<CutSubdomainID>("positive_id"))
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
MovingLineSegmentCutSetUserObject::cutFraction(unsigned int /*cut_num*/) const
{
  return 1;
}

CutSubdomainID
MovingLineSegmentCutSetUserObject::getCutSubdomainID(const Node * node) const
{
  return calculateSignedDistance(*node) > 0.0 ? _positive_id : _negative_id;
}

Real
MovingLineSegmentCutSetUserObject::calculateSignedDistance(Point p) const
{
  const int line_cut_data_len = 6;

  Real min_dist = std::numeric_limits<Real>::max();

  for (unsigned int i = 0; i < _cut_data.size() / line_cut_data_len; ++i)
  {
    Point a = Point(_cut_data[i * line_cut_data_len + 0], _cut_data[i * line_cut_data_len + 1], 0);
    Point b = Point(_cut_data[i * line_cut_data_len + 2], _cut_data[i * line_cut_data_len + 3], 0);

    Point c = p - a;
    Point v = (b - a) / (b - a).norm();
    Real d = (b - a).norm();
    Real t = v * c;

    Real dist;
    Point nearest_point;

    if (t < 0)
    {
      dist = (p - a).norm();
      nearest_point = a;
    }
    else if (t > d)
    {
      dist = (p - b).norm();
      nearest_point = b;
    }
    else
    {
      v *= t;
      dist = (p - a - v).norm();
      nearest_point = (a + v);
    }

    Point p_nearest_point = nearest_point - p;

    Point normal_ab = Point(-(b - a)(1), (b - a)(0), 0);

    if (normal_ab * p_nearest_point < 0)
      dist = -dist;

    if (std::abs(dist) < std::abs(min_dist))
      min_dist = dist;
  }

  return min_dist;
}
