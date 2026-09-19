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
#include <optional>

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

/// Measurements of how the retained domain occupies an element. The node flags and
/// quadrature-based domain fraction are complementary: the node flags catch a surface that clips
/// only a corner, while the fraction catches a surface enclosed within the element.
struct ElementDomainOccupancy
{
  /// Whether all element nodes lie in the retained domain.
  bool all_nodes_in_domain;
  /// Whether all element nodes lie outside the retained domain.
  bool all_nodes_outside_domain;
  /// Fraction of the element's quadrature-weighted measure occupied by the retained domain.
  Real domain_fraction;
};

/// Measure how a domain occupies an element based on its nodes, quadrature points, and the
/// in-domain policy.
ElementDomainOccupancy
elementDomainOccupancy(const Elem & elem,
                       Order qrule_order,
                       const std::function<bool(const libMesh::Point &)> & is_in_domain);

/// Measure domain occupancy when boundary points are neither in nor outside the domain.
ElementDomainOccupancy
elementDomainOccupancy(const Elem & elem,
                       Order qrule_order,
                       const std::function<bool(const libMesh::Point &)> & is_in_domain,
                       const std::function<bool(const libMesh::Point &)> & is_outside_domain);

/// Policy for assigning partial elements to a dedicated intercepted subdomain.
struct InterceptedSubdomainPolicy
{
  bool mark_intercepted;
  SubdomainID subdomain_id;
};

/// A candidate subdomain and its occupancy of an element.
struct SubdomainOccupancy
{
  SubdomainID subdomain_id;
  ElementDomainOccupancy occupancy;
};

/// Select a subdomain based on how multiple candidate domains occupy an element.
///
/// A fully occupied candidate takes precedence. Otherwise, the candidate with the largest domain
/// fraction is selected, with fuzzy ties resolved by the lowest subdomain ID. A partial candidate
/// is accepted according to the intercepted-subdomain policy and lambda.
///
/// @param candidate_occupancies Candidate subdomain IDs and their measured element occupancies
/// @param intercepted_policy Whether partial elements receive a dedicated subdomain ID, and that ID
/// @param lambda Maximum fraction outside the selected domain for accepting a partial element
/// @return The selected subdomain ID, or std::nullopt if no candidate subdomain is selected
std::optional<SubdomainID>
selectSubdomainFromOccupancies(const std::vector<SubdomainOccupancy> & candidate_occupancies,
                               const InterceptedSubdomainPolicy & intercepted_policy,
                               Real lambda);

/// The subdomain IDs a (possibly partial) element can be labeled with.
struct ClassificationSubdomains
{
  SubdomainID inside;
  SubdomainID outside;
  SubdomainID intercepted;
};

/// Classify an element from its retained-domain occupancy measurements.
///
/// An element whose nodes and measure are entirely in the domain is inside; one whose nodes and
/// measure are entirely outside the domain is outside. A remaining (partial) element is assigned
/// the intercepted subdomain when mark_intercepted is true, and otherwise is resolved by the
/// lambda threshold (see isInactive).
SubdomainID classifySubdomainFromOccupancy(const ElementDomainOccupancy & occupancy,
                                           const ClassificationSubdomains & subdomain_id_settings,
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
