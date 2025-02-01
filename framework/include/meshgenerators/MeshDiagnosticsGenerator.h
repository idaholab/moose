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
#include "MooseEnum.h"

/*
 * Mesh 'generator' to diagnose potentially unsupported features or miscellaneous issues
 */
class MeshDiagnosticsGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  MeshDiagnosticsGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// the input mesh to be diagnosed
  std::unique_ptr<MeshBase> & _input;

private:
  /// Routine to check sideset orientation near subdomains
  void checkSidesetsOrientation(const std::unique_ptr<MeshBase> & mesh) const;
  //// Routine to check is mesh is fully covered in sidesets
  void checkWatertightSidesets(const std::unique_ptr<MeshBase> & mesh) const;
  //// Routine to check is mesh is fully covered in nodesets
  void checkWatertightNodesets(const std::unique_ptr<MeshBase> & mesh) const;
  /// Helper function that finds the intersection between the given vectors
  std::vector<boundary_id_type>
  findBoundaryOverlap(const std::vector<boundary_id_type> & watertight_boundaries,
                      std::vector<boundary_id_type> & boundary_ids) const;
  /// Routine to check the element volumes
  void checkElementVolumes(const std::unique_ptr<MeshBase> & mesh) const;
  /// Routine to check the element types in each subdomain
  void checkElementTypes(const std::unique_ptr<MeshBase> & mesh) const;
  /// Routine to check whether elements overlap in the mesh
  void checkElementOverlap(const std::unique_ptr<MeshBase> & mesh) const;
  /// Routine to check whether there are non-planar sides in the mesh
  void checkNonPlanarSides(const std::unique_ptr<MeshBase> & mesh) const;
  /// Routine to check whether a mesh presents non-conformality
  void checkNonConformalMesh(const std::unique_ptr<MeshBase> & mesh) const;
  /// Routine to check whether a mesh presents non-conformality born from adaptivity
  void checkNonConformalMeshFromAdaptivity(const std::unique_ptr<MeshBase> & mesh) const;
  /// Routine to check whether the Jacobians (elem and side) are not negative
  void checkLocalJacobians(const std::unique_ptr<MeshBase> & mesh) const;
  //// Routine to check for non matching edges
  void checkNonMatchingEdges(const std::unique_ptr<MeshBase> & mesh) const;

  /**
   * Utility routine to output the final diagnostics level in the desired mode
   * @param msg the message to output
   * @param log_level the log level to output the message at
   * @param problem_detected if set to false, prevents erroring from the log, despite the log level
   * problem_detected is used to avoid erroring when the log is requested but there are no issues so
   * it should just say "0 problems" with an info message
   */
  void diagnosticsLog(std::string msg, const MooseEnum & log_level, bool problem_detected) const;

  /// whether to check that sidesets are consistently oriented using neighbor subdomains
  const MooseEnum _check_sidesets_orientation;
  /// whether to check that each external side is assigned to a sideset
  const MooseEnum _check_watertight_sidesets;
  /// whether to check that each external node is assigned to a nodeset
  const MooseEnum _check_watertight_nodesets;
  /// Names of boundaries to be checked in watertight checks
  std::vector<BoundaryName> _watertight_boundary_names;
  /// IDs of boundaries to be checked in watertight checks
  std::vector<BoundaryID> _watertight_boundaries;
  /// whether to check element volumes
  const MooseEnum _check_element_volumes;
  /// minimum size for element volume to be counted as a tiny element
  const Real _min_volume;
  /// maximum size for element volume to be counted as a big element
  const Real _max_volume;
  /// whether to check different element types in the same sub-domain
  const MooseEnum _check_element_types;
  /// whether to check for intersecting elements
  const MooseEnum _check_element_overlap;
  /// whether to check for elements in different planes (non_planar)
  const MooseEnum _check_non_planar_sides;
  /// whether to check for non-conformal meshes
  const MooseEnum _check_non_conformal_mesh;
  /// tolerance for detecting when meshes are not conformal
  const Real _non_conformality_tol;
  //// whether to check for intersecting edges
  const MooseEnum _check_non_matching_edges;
  //// tolerance for detecting when edges intersect
  const Real _non_matching_edge_tol;
  /// whether to check for the adaptivity of non-conformal meshes
  const MooseEnum _check_adaptivity_non_conformality;
  /// whether to check for negative jacobians in the domain
  const MooseEnum _check_local_jacobian;
  /// number of logs to output at most for each check
  const unsigned int _num_outputs;
};
