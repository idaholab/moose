//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SBMBndEdge2.h"
#include "Ball.h"

SBMBndEdge2::SBMBndEdge2(const Elem * elem)
  : SBMBndElementBase(elem), LineSegment(elem->point(0), elem->point(1))
{
  mooseAssert(elem->type() == EDGE2, "Element must be of type EDGE2");
  mooseAssert(MooseUtils::absoluteFuzzyEqual(elem->point(0)(2), elem->point(1)(2)),
              "Currently SBMBndEdge2 must be parallel to the x-y plane, i.e., z-coordinates of "
              "points must be equal");
}

const Point
SBMBndEdge2::computeNormal() const
{
  return LineSegment::normal();
}
