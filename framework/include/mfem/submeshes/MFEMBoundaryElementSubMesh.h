//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once
#include "MFEMSubMesh.h"


class Plane3D {

  public:
    Plane3D();
    ~Plane3D();
  
    // Constructs a mathematical 3D plane from a mesh face
    void make3DPlane(const mfem::ParMesh *pm, const int face);
  
    // Calculates on which side of the infinite 3D plane a point is.
    // Returns 1, -1, or 0, the latter meaning the point is on the plane
    int side(const mfem::Vector v);
  
  private:
    mfem::Vector *u;
    double d;
  };

/**
 * Constructs and stores an mfem::ParSubMesh object as
 * as a restriction of the parent mesh to the set of user-specified boundaries.
 * Access using the getSubMesh() accessor.
 */
class MFEMBoundaryElementSubMesh : public MFEMSubMesh
{
public:
  static InputParameters validParams();
  MFEMBoundaryElementSubMesh(const InputParameters & parameters);

protected:
  virtual void buildSubMesh() override;
  void makeWedge(mfem::ParMesh & parent_mesh);

    // Finds the coordinates for the "centre of mass" of the vertices of an
  // element.
  mfem::Vector elementCentre(int el, mfem::ParMesh *pm);

  // Checks whether a given element is within a certain domain or vector of
  // domains.
  bool isInDomain(const int el, const mfem::Array<int> &dom,
                  const mfem::ParMesh *mesh);
  bool isInDomain(const int el, const int &sd, const mfem::ParMesh *mesh);

  const BoundaryName & _boundary_name;
  const int _bdr_attribute;
  int _subdomain_label;
};

#endif
