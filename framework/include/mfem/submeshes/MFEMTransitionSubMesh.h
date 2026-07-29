//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMSubMesh.h"
#include "MFEMBlockRestrictable.h"

/**
 * Modifies the MFEM Mesh to label a subdomain consisting of the one-element-wide layer of
 * elements adjacent to a specified boundary, and constructs and stores an mfem::ParSubMesh
 * object associated with it. The boundary may be an interior cut (in which case elements on
 * one side are taken) or an exterior boundary of the mesh (in which case the single interior
 * side is taken). Access using the getSubMesh() accessor.
 */
class MFEMTransitionSubMesh : public MFEMSubMesh, public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();
  MFEMTransitionSubMesh(const InputParameters & parameters);

protected:
  virtual void buildSubMesh() override;

  /// Add attributes to the parent mesh representing the transition region
  void labelMesh(mfem::ParMesh & parent_mesh);

  /// Set new attributes for the provided transition region elements
  void setAttributes(mfem::ParMesh & parent_mesh, mfem::Array<int> & transition_els);

  /// Checks whether a given element is within a certain domain or vector of domains.
  bool isInDomain(const int & el, const mfem::Array<int> & subdomains, const mfem::ParMesh & mesh);

  /// Finds the normal vector of a face in the mesh from its vertices
  mfem::Vector findFaceNormal(const mfem::ParMesh & mesh, const int & face);

  /// Checks whether an element lies on the positive side of the boundary at a given boundary
  /// vertex, using the averaged surface normal at that vertex.
  bool isPositiveSide(const int & el,
                      const int & boundary_vertex,
                      const mfem::Vector & normal,
                      mfem::ParMesh & mesh);

  /// Averaged unit surface normal at every boundary vertex, keyed by global vertex id and summed
  /// across ranks so that vertices shared between processors agree.
  std::map<HYPRE_BigInt, mfem::Vector>
  computeVertexNormals(mfem::ParMesh & parent_mesh,
                       const mfem::Array<HYPRE_BigInt> & global_vertex_ids);

  /// Grow num_layers element rings from seed into transition_set, staying on the requested side of
  /// the boundary and within the user's subdomains.
  void growLayers(mfem::ParMesh & parent_mesh,
                  const mfem::Array<HYPRE_BigInt> & global_vertex_ids,
                  const mfem::Table & vert_to_elem,
                  const std::map<HYPRE_BigInt, mfem::Vector> & vertex_normals,
                  const mfem::Array<int> & seed,
                  unsigned int num_layers,
                  bool positive_side,
                  std::set<int> & transition_set,
                  mfem::Array<int> & transition_els);

  /// Resolve the 'boundary' parameter (numeric attributes and/or named attribute sets) into the
  /// set of boundary attribute IDs the transition region is built from.
  mfem::Array<int> boundaryAttributes();

  std::shared_ptr<mfem::ParSubMesh> _boundary_submesh{nullptr};
  const BoundaryName & _transition_subdomain_boundary;
  const SubdomainName & _transition_subdomain;
  const SubdomainName & _closed_subdomain;

  /// True if the supplied boundary lies on the exterior of the mesh (only one side has
  /// elements), in which case no side selection is performed.
  bool _exterior_boundary{false};

  /// Number of element-thick layers grown on the positive side of the boundary, and inwards for
  /// an exterior boundary.
  const unsigned int _num_layers_positive;
  /// Number of element-thick layers grown on the negative side of the boundary.
  const unsigned int _num_layers_negative;
};

#endif
