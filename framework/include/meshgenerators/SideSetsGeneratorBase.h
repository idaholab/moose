//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
#include "libmesh/fe_base.h"

// libMesh forward declarations
namespace libMesh
{
class QGauss;
class Elem;
}

/*
 * Base class for mesh generators that add sidesets to the mesh
 */
class SideSetsGeneratorBase : public MeshGenerator
{
public:
  static InputParameters validParams();

  SideSetsGeneratorBase(const InputParameters & parameters);
  virtual ~SideSetsGeneratorBase(); // dtor required for unique_ptr with forward declarations

protected:
  /**
   * This method is used to construct the FE object so we can compute
   * normals of faces.  Additionaly this method also grabs the sideset
   * ids from the mesh given a list of sideset names.
   */
  void setup(MeshBase & mesh);

  /**
   * This method finalizes the object, setting names back in the
   * boundary_info object and releasing memory.
   */
  void finalize();

  /**
   * This method implements a recursive flood routine to paint a sideset of
   * mesh to neighboring faces given a starting element and normal.
   */
  void
  flood(const Elem * elem, const Point & normal, const boundary_id_type & side_id, MeshBase & mesh);

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
   * Determines whether the given side of an element belongs to any boundaries in the
   * included_boundaries parameter.
   */
  bool elementSideInIncludedBoundaries(const Elem * const elem,
                                       const unsigned int side,
                                       const MeshBase & mesh) const;

  /**
   * Determines whether the given side of an element belongs to any boundaries in the
   * excluded_boundaries parameter.
   */
  bool elementSideInExcludedBoundaries(const Elem * const elem,
                                       const unsigned int side,
                                       const MeshBase & mesh) const;

  /**
   * Determines whether the given element's side satisfies the following parameters:
   * include_only_external_sides, included_boundaries, included_neighbor_subdomains and normal
   */
  bool elemSideSatisfiesRequirements(const Elem * const elem,
                                     const unsigned int side,
                                     const MeshBase & mesh,
                                     const Point & normal,
                                     const Point & face_normal);

  /// the mesh to add the sidesets to
  std::unique_ptr<MeshBase> & _input;

  /// The list of new boundary names
  std::vector<BoundaryName> _boundary_names;

  /// Whether to fix the normal or allow it to vary to "paint" around curves
  const bool _fixed_normal;

  /// Whether or not to remove the old sidesets (all of them, if any) when adding sidesets
  const bool _replace;

  /// whether to check boundary ids against the included boundary list when adding sides or not
  const bool _check_included_boundaries;

  /// whether to check boundary ids against the excluded boundary list when adding sides or not
  const bool _check_excluded_boundaries;

  /// whether to check subdomain ids of the element in the (element, side, boundary id) tuple when adding sides
  const bool _check_subdomains;

  /// whether to check the subdomain ids of the neighbor element (on the other 'side' of the side) when adding sides
  const bool _check_neighbor_subdomains;

  /// A list of boundary ids that the side has to be part of, extracted from the included_boundaries parameter
  std::vector<boundary_id_type> _included_boundary_ids;

  /// A list of boundary ids that the side must not be a part of, extracted from the excluded_boundaries parameter
  std::vector<boundary_id_type> _excluded_boundary_ids;

  /// A list of included subdomain ids that the side has to be part of, extracted from the included_subdomains parameter
  std::vector<subdomain_id_type> _included_subdomain_ids;

  /// A list of included neighbor subdomain ids that the sides' neighbor element must be a part of
  std::vector<subdomain_id_type> _included_neighbor_subdomain_ids;

  /// Whether to only include external side when considering sides to add to the sideset
  const bool _include_only_external_sides;

  /// true if only faces close to "normal" will be added
  bool _using_normal;

  /// if specified, then faces are only added if their normal is close to this
  Point _normal;

  /**
   * if normal is specified, then faces are only added
   * if face_normal.normal_hat <= 1 - normal_tol
   * where normal_hat = _normal/|_normal|
   */
  const Real _normal_tol;

  std::unique_ptr<libMesh::FEBase> _fe_face;
  std::unique_ptr<libMesh::QGauss> _qface;
  std::map<boundary_id_type, std::set<const Elem *>> _visited;
};
