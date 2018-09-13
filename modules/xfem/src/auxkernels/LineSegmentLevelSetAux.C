//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LineSegmentLevelSetAux.h"
#include "LineSegmentCutSetUserObject.h"
#include "FEProblem.h"

registerMooseObject("MooseApp", LineSegmentLevelSetAux);

template <>
InputParameters
validParams<LineSegmentLevelSetAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription(
      "Auxiliary Kernel that calcuates level set value using line segments' description.");
  params.addParam<UserObjectName>(
      "line_segment_cut_set_user_object",
      "Name of GeometricCutUserObject that gives the line segments information.");
  return params;
}

LineSegmentLevelSetAux::LineSegmentLevelSetAux(const InputParameters & parameters)
  : AuxKernel(parameters)
{
  if (!isNodal())
    mooseError("LineSegmentLevelSetAux: Aux variable must be nodal variable.");

  FEProblemBase * fe_problem = dynamic_cast<FEProblemBase *>(&_subproblem);

  const UserObject * uo = &(
      fe_problem->getUserObjectBase(getParam<UserObjectName>("line_segment_cut_set_user_object")));

  if (dynamic_cast<const LineSegmentCutSetUserObject *>(uo) == nullptr)
    mooseError(
        "Failed to cast UserObject to LineSegmentCutSetUserObject in LineSegmentLevelSetAux");

  _linesegment_uo = dynamic_cast<const LineSegmentCutSetUserObject *>(uo);
}

Real
LineSegmentLevelSetAux::calculateSignedDistance(Point p)
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

void
LineSegmentLevelSetAux::compute()
{
  _cut_data = _linesegment_uo->getCutData();
  AuxKernel::compute();
}

Real
LineSegmentLevelSetAux::computeValue()
{
  return calculateSignedDistance(*_current_node);
}
