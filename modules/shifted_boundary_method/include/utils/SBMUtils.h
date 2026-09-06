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
#include "libmesh/enum_order.h"

#include <functional>

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

/// Compute the fraction of an element's quadrature-weighted measure satisfying a predicate.
Real activeElementFraction(const Elem & elem,
                           Order qrule_order,
                           const std::function<bool(const libMesh::Point &)> & is_active);

/// Return whether a partial element is inactive, i.e. its inactive fraction exceeds lambda.
///
/// The endpoint handling preserves the convention that lambda zero rejects and lambda one
/// accepts a partially active element.
bool isInactive(Real active_fraction, Real lambda);

/// Measured activity state of a (possibly partial) element, used to classify it. All fields are
/// raw measurements: the two node flags come from a nodal test and active_fraction from a
/// quadrature test. They are complementary (neither implies the other): the node flags catch a
/// surface that clips only a corner (fraction near one), and the fraction catches a surface
/// enclosed in the interior (all nodes on one side).
struct ElementActivity
{
  /// Whether all of the element's nodes are on the active (retained) side.
  bool all_nodes_active;
  /// Whether all of the element's nodes are on the inactive (removed) side.
  bool all_nodes_inactive;
  /// Fraction of the element's quadrature-weighted measure that is active, in [0, 1].
  Real active_fraction;
};

/// The subdomain IDs a (possibly partial) element can be labeled with.
struct ClassificationSubdomains
{
  SubdomainID inside;
  SubdomainID outside;
  SubdomainID intercepted;
};

/// Classify a (possibly partial) element into an inside/outside/intercepted subdomain.
///
/// An element with all nodes active and a fully active measure is inside; one with all nodes
/// inactive and a fully inactive measure is outside. A remaining (partial) element is assigned
/// the intercepted subdomain when mark_intercepted is true, and otherwise is resolved by the
/// lambda threshold (see isInactive).
SubdomainID classifyPartialElement(const ElementActivity & activity,
                                   const ClassificationSubdomains & subdomains,
                                   bool mark_intercepted,
                                   Real lambda);

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

/// Compute the true boundary surface normal at the point on the boundary closest to pt.
RealVectorValue trueNormalFromFunction(const Function * func, const libMesh::Point & pt, Real t);

/// Scan all distance functions and return the closest distance vector.
RealVectorValue closestDistanceVector(const std::vector<const Function *> & funcs,
                                      const libMesh::Point & pt,
                                      Real t);

/// Scan all distance functions and return the corresponding normal vector.
RealVectorValue closestTrueNormalVector(const std::vector<const Function *> & funcs,
                                        const libMesh::Point & pt,
                                        Real t);

/// Computes the union signed distance by taking the minimum of all
/// signed distance functions. This definition is valid for overlapping
/// geometries and preserves the correct union boundary.
Real unionSignedDistance(const std::vector<const Function *> & funcs, Real t, const Point & p);
}
