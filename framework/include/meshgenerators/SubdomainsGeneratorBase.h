//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

/*
 * Base class for mesh generators that loop over elements in subdomains on a mesh
 * - defines some common parameters
 * - defines a useful flooding/painting algorithm to apply an operation on elements
 */
class SubdomainsGeneratorBase : public MeshGenerator
{
public:
  static InputParameters validParams();

  SubdomainsGeneratorBase(const InputParameters & parameters);

protected:
  /// Sets up various data structures
  void setup(MeshBase & mesh);

  /**
   * This method implements a recursive flood routine to paint (applying an operation)
   * to elements on mesh, going from elements to neighbors.
   * The flood/painting starts at a given element with a given normal (if using normals to paint).
   * @param elem starting element
   * @param normal a normal used for comparing 2D surface elements outgoing normals
   * @param starting_elem the starting element for the flooding
   * @param sub_id subdomain id to assign to elements being painted
   */
  void flood(Elem * const elem,
             const Point & normal,
             const Elem & starting_elem,
             const subdomain_id_type & sub_id,
             MeshBase & mesh);

  /**
   * Determines whether two normal vectors are within normal_tol of each other.
   * @param normal_1 The first normal vector to compare to normal_2.
   * @param normal_2 The second normal vector to compare to normal_1.
   * @param tol The comparison tolerance.
   * @return A bool indicating whether 1 - dot(normal_1, normal_2) <= tol.
   */
  bool normalsWithinTol(const Point & normal_1, const Point & normal_2, const Real tol) const;

  /**
   * Determines whether the given element's subdomain id is in the given subdomain_id_list.
   * @param elem the element to consider
   * @param subdomain_id_list a vector of all the subdomains to consider
   */
  bool elementSubdomainIdInList(const Elem * const elem,
                                const std::vector<subdomain_id_type> & subdomain_id_list) const;

  /**
   * Determines whether the given element satisfies a set of criteria that are defined in this base
   * class
   * @param elem element to consider
   * @param desired_normal for 2D elements, the desired outwards normal
   * @param base_elem the reference element for the criterion (max distance for now)
   * @param elem_normal for 2D elements, the elem outwards normal
   */
  bool elementSatisfiesRequirements(const Elem * const elem,
                                    const Point & desired_normal,
                                    const Elem & base_elem,
                                    const Point & face_normal) const;

  /**
   * Get the normal of the 2D element
   * @param elem pointer to the element
   */
  Point get2DElemNormal(const Elem * const elem) const;

  /// the mesh to add the subdomains to
  std::unique_ptr<MeshBase> & _input;

  /// The list of new subdomain names (useful for adding subdomains)
  std::vector<SubdomainName> _subdomain_names;

  /// whether to check the prior subdomain id of the element when choosing whether to change its subdomain id
  const bool _check_subdomains;

  /// A list of included subdomain ids that the element has to be priorly a part of, extracted from the 'included_subdomains' parameter
  std::vector<subdomain_id_type> _included_subdomain_ids;

  /// true if only elements are only considered when their normal is close to either the "_normal" or a moving "normal" vector
  bool _using_normal;

  /// if specified, then surface elements are only considered if their normal is close to this
  Point _normal;
  /**
   * Tolerance to group elements with normals such that
   * face_normal.normal_hat <= 1 - normal_tol
   * where normal_hat = _normal/|_normal|
   * Only useful to paint over 2D surface elements
   */
  const Real _normal_tol;
  /// Tolerance but when using the flipped normal
  const Real _flipped_normal_tol;
  /// Whether to paint/flood using a fixed normal or a moving normal
  const bool _fixed_normal;
  /// Whether to allow normal flips
  const bool _allow_normal_flips;
  /// Whether to painting beyond a certain radius
  const bool _has_max_distance_criterion;
  /// Distance to use for max painting radius. This distance can be specified per subdomain
  std::unordered_map<subdomain_id_type, Real> _max_elem_distance;

  /// Map used for the flooding algorithm to keep track of which elements have been visited for which subdomain
  std::map<subdomain_id_type, std::set<Elem *>> _visited;
  /// Only visit each element once
  const bool _flood_only_once;
  /// Set used when flooding each element once. If the element pointer is in the set, it has been visited and acted upon
  std::set<Elem *> _acted_upon_once;
  /// Additional heuristic: check the element neighbors and if they have already been painted with the subdomain,
  /// check if their normal is close to the current element's normal. If it is close, then accept the element
  /// as long as it also meets the other criteria (in included_subdomains, centroid within distance, etc)
  const bool _check_painted_neighor_normals;
};
