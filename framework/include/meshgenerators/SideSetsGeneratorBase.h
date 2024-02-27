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

// libMesh forward declarations
namespace libMesh
{
class QGauss;
class Elem;
template <typename>
class FEGenericBase;
typedef FEGenericBase<Real> FEBase;
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
  void flood(const Elem * elem, Point normal, boundary_id_type side_id, MeshBase & mesh);

  bool normalsWithinTol(const Point & normal_1, const Point & normal_2) const;

  /// the mesh to add the sidesets to
  std::unique_ptr<MeshBase> & _input;

  const bool _fixed_normal;

  /// Whether or not to remove the old sidesets (if any) when adding sidesets
  const bool _replace;

  /// whether to check boundary ids when adding sides or not
  const bool _check_boundaries;

  /// whether to check subdomain ids when adding sides or not
  const bool _check_subdomains;

  /// whether to check neighbor subdomain ids when adding sides or not
  const bool _check_neighbor_subdomains;

  /// A list of included subdomain ids that the side has to be part of
  std::vector<subdomain_id_type> _included_ids;

  /// A list of included neighbor subdomain ids
  std::vector<subdomain_id_type> _included_neighbor_ids;

  /// Whether to only include external side when considering sides to add to the sideset
  const bool _include_only_external_sides;

  /// if specified, then faces are only added if their normal is close to this
  Point _normal;

  /// true if only faces close to "normal" will be added
  bool _using_normal;

  /**
   * if normal is specified, then faces are only added
   * if face_normal.normal_hat <= 1 - normal_tol
   * where normal_hat = _normal/|_normal|
   */
  Real _normal_tol;

  std::unique_ptr<FEBase> _fe_face;
  std::unique_ptr<QGauss> _qface;
  std::map<boundary_id_type, std::set<const Elem *>> _visited;
};
