//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include <vector>

namespace PointReduction
{

typedef std::pair<Real, Real> FunctionNode;
typedef std::vector<FunctionNode> FunctionNodeList;

/// compute the perpendicular distance of a point from a line defined by begin and end points
Real perpendicularDistance(const FunctionNode & point,
                           const FunctionNode & begin,
                           const FunctionNode & end);

/// return a pruned function node list using the Ramer-Douglas-Peucker algorithm
FunctionNodeList douglasPeucker(const FunctionNodeList &, Real epsilon);

} // namespace PointReduction
