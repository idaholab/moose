//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SBMBndTri3.h"
#include "Ball.h"

SBMBndTri3::SBMBndTri3(const Elem * elem)
  : SBMBndElementBase(elem), Triangle(elem->point(0), elem->point(1), elem->point(2))
{
  mooseAssert(elem->type() == TRI3, "Element must be of type TRI3");
}

const Point
SBMBndTri3::computeNormal() const
{
  return Triangle::normal();
}
