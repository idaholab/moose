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

  // Checks whether a given element is within a certain domain or vector of
  // domains.
  bool isInDomain(const int & el, const mfem::Array<int> & subdomains, const mfem::ParMesh & mesh);

  const BoundaryName & _cut_boundary;
  const int _cut_bdr_attribute;
  const BoundaryName & _transition_subdomain_boundary;
  const SubdomainName & _transition_subdomain;  
  const SubdomainName & _closed_subdomain;    
  int _subdomain_label;
};

class Plane3D
{
public:
  Plane3D();

  // Constructs a mathematical 3D plane from a mesh face
  void make3DPlane(const mfem::ParMesh & mesh, const int & face);

  // Calculates on which side of the infinite 3D plane a point is.
  // Returns 1, -1, or 0, the latter meaning the point is on the plane
  int side(const mfem::Vector & v);

private:
  mfem::Vector normal;
  double d;
};

#endif
