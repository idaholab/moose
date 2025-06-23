//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BSpline.h"
#include "libmesh/point.h"

namespace Moose
{
BSpline::BSpline(const unsigned int degree, const std::vector<libMesh::Point> & control_points)
  : _degree(degree), _control_points(control_points), _knot_vector(buildKnotVector())
{
}

libMesh::Point
BSpline::getPoint(const double t) const
{
  return {0, 0, 0};
}

std::vector<double>
BSpline::buildKnotVector() const
{
  if (_degree == 1)
    return {1, 0, 0};
  else
    return {0};
}
}
