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

typedef std::pair<libMesh::Real, libMesh::Real> FunctionNode;
typedef std::vector<FunctionNode> FunctionNodeList;

/**
 * compute the perpendicular distance of a point P from a line defined by begin
 * and end points.
 *
 * @param point The (x,y) point P
 * @param begin The first (x,y) point defining the line to compute the distance to
 * @param end The second (x,y) point defining the line to compute the distance to
 */
libMesh::Real perpendicularDistance(const FunctionNode & point,
                                    const FunctionNode & begin,
                                    const FunctionNode & end);

/**
 * Generate a pruned function node list using the Ramer-Douglas-Peucker algorithm.
 *
 * @param list An ordered (by x) list of (x,y) points defining a pointwise defined function.
 * @param epsilon The Ramer-Douglas-Peucker tolerance parameter for coarsening.
 */
FunctionNodeList douglasPeucker(const FunctionNodeList &, libMesh::Real epsilon);

} // namespace PointReduction
