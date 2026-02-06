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
 * Base class for mesh generators that add subdomains to the mesh
 */
class SubdomainsGeneratorBase : public MeshGenerator
{
public:
  static InputParameters validParams();

  SubdomainsGeneratorBase(const InputParameters & parameters);

protected:
  /**
   * This method is used to construct the FE objects that may be required
   */
  void setup(MeshBase & mesh);

  /**
   * This method implements a recursive flood routine to paint a surface subdomain of
   * mesh to neighboring faces given a starting element and a normal to that element.
   * @param elem starting element
   * @param normal a normal used for comparing 2D surface elements outgoing normals
   * @param sub_id subdomain id to assign to elements being painted
   */
  void
  flood(Elem * const elem, const Point & normal, const subdomain_id_type & sub_id);

  /**
   * Determines whether two normal vectors are within normal_tol of each other.
   * @param normal_1 The first normal vector to compare to normal_2.
   * @param normal_2 The second normal vector to compare to normal_1.
   * @param tol The comparison tolerance.
   * @return A bool indicating whether 1 - dot(normal_1, normal_2) <= tol.
   */
  bool normalsWithinTol(const Point & normal_1, const Point & normal_2, const Real & tol) const;

  /**
   * Determines whether the given element's subdomain id is in the given subdomain_id_list.
   */
  bool elementSubdomainIdInList(const Elem * const elem,
                                const std::vector<subdomain_id_type> & subdomain_id_list) const;

  /**
   * Determines whether the given element satisfies a set of criteria that are defined in this base class
   * @param elem element to consider
   * @param desired_normal for 2D elements, the desired outwards normal
   * @param elem_normal for 2D elements, the elem outwards normal
   */
  bool elementSatisfiesRequirements(const Elem * const elem,
                                    const Point & desired_normal,
                                    const Point & face_normal) const;

  /**
   * Get the normal of the 2D element
   * @param elem pointer to the element
   */
  Point get2DElemNormal(const Elem * const elem) const;

  /// the mesh to add the subdomains to
  std::unique_ptr<MeshBase> & _input;

  /// The list of new subdomain names
  std::vector<SubdomainName> _subdomain_names;

  /// whether to check the prior subdomain id of the element when choosing whether to change its subdomain id
  const bool _check_subdomains;

  /// A list of included subdomain ids that the element has to be priorly a part of, extracted from the included_subdomains parameter
  std::vector<subdomain_id_type> _included_subdomain_ids;

  /// Map used for the flooding algorithm
  std::map<subdomain_id_type, std::set<Elem *>> _visited;

  /// true if only faces close to "normal" will be added
  bool _using_normal;

  /// if specified, then faces are only added if their normal is close to this
  Point _normal;
  /**
   * Tolerance to group elements with normals such that
   * Useful to paint over 2D surface elements
   * face_normal.normal_hat <= 1 - normal_tol
   * where normal_hat = _normal/|_normal|
   */
  const Real _normal_tol;
};
