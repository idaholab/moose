//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGHalfspace.h"

namespace CSG
{

CSGHalfspace::CSGHalfspace(std::shared_ptr<CSGSurface> surf, const CSGSurface::Direction direction)
  : _surface(surf), _direction(direction)
{
}

CSGSurface::Direction
CSGHalfspace::getInverseDirection() const
{
  return (_direction == CSGSurface::Direction::positive) ? CSGSurface::Direction::negative
                                                         : CSGSurface::Direction::positive;
}

std::string
CSGHalfspace::toString() const
{
  return ((_direction == CSGSurface::Direction::positive) ? "+" : "-") + _surface->getName();
}
} // namespace CSG
