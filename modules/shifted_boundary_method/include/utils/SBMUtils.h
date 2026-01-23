//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"
#include "MooseTypes.h"

class Function;
class FunctionInterface;
class InputParameters;
namespace libMesh
{
class Point;
}

namespace SBMUtils
{
bool checkWatertightnessFromRawElems(const std::vector<const Elem *> & bd_elements);

/// Build a list of distance functions based on names specified in input.
std::vector<const Function *>
buildDistanceFunctions(const std::vector<FunctionName> & function_names,
                       const FunctionInterface & function_provider);

/// Compute the distance vector induced by a distance function.
/// The provided function must represent a distance to the true boundary:
/// either a signed distance function (ParsedFunction) or an unsigned distance
/// function provided by UnsignedDistanceToSurfaceMesh.
RealVectorValue
distanceVectorFromFunction(const Function * func, const libMesh::Point & pt, Real t);

/// Compute the true boundary surface normal using a distance function.
/// The query point is first projected onto the true boundary, and the
/// surface normal of the boundary is then evaluated at the projected
/// (closest) point.
RealVectorValue trueNormalFromFunction(const Function * func, const libMesh::Point & pt, Real t);

/// Scan all distance functions and return the closest distance vector.
RealVectorValue closestDistanceVector(const std::vector<const Function *> & funcs,
                                      const libMesh::Point & pt,
                                      Real t);

/// Scan all distance functions and return the corresponding normal vector.
RealVectorValue closestTrueNormalVector(const std::vector<const Function *> & funcs,
                                        const libMesh::Point & pt,
                                        Real t);
}

/// Enum for different distance function types.
enum class DistanceFunctionType
{
  NONE = -1,
  SIGNED_DISTANCE = 0,
  SBM_BOUNDARY_DISTANCE = 1,
};
