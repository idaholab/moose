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
 * Modifies the MFEM Mesh to label a subdomain consisting of elements adjacent to an
 * interior surface on one side, and constructs an stores an mfem::ParSubMesh object
 * associated with it.
 * Access using the getSubMesh() accessor.
 */
class MFEMCutTransitionSubMesh : public MFEMSubMesh, public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();
  MFEMCutTransitionSubMesh(const InputParameters & parameters);

protected:
  virtual void buildSubMesh() override;

  // Add attributes to the parent mesh representing the cut transition region
  void labelMesh(mfem::ParMesh & parent_mesh);

  // Set new attributes for the provided transition region elements
  void setAttributes(mfem::ParMesh & parent_mesh, mfem::Array<int> & transition_els);

  // Checks whether a given element is within a certain domain or vector of
  // domains.
  bool isInDomain(const int & el, const mfem::Array<int> & subdomains, const mfem::ParMesh & mesh);

  // Finds the normal vector of a face in the mesh from its vertices
  mfem::Vector findFaceNormal(const mfem::ParMesh & mesh, const int & face);

  // Checks whether an element lies on the positive or negative side of the cut plane
  int sideOfCut(const int & el, const int & el_vertex_on_cut, mfem::ParMesh & mesh);

  const BoundaryName & _cut_boundary;
  std::shared_ptr<mfem::ParSubMesh> _cut_submesh{nullptr};
  const BoundaryName & _transition_subdomain_boundary;
  const SubdomainName & _transition_subdomain;
  const SubdomainName & _closed_subdomain;
  mfem::Vector _cut_normal;
};

#endif
