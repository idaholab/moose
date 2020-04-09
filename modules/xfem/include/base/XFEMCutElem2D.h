//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "XFEMCutElem.h"
#include "EFAElement2D.h"

using namespace libMesh;

namespace libMesh
{
class MeshBase;
class Elem;
class Node;
}

class XFEMCutElem2D : public XFEMCutElem
{
public:
  /**
   * Constructor initializes XFEMCutElem2D object
   * @param elem The element on which XFEMCutElem2D is built
   * @param CEMelem The EFAFragment2D object that belongs to XFEMCutElem2D
   * @param n_qpoints The number of quadrature points
   * @param n_sides The number of sides which the element has
   */
  XFEMCutElem2D(Elem * elem,
                const EFAElement2D * const CEMelem,
                unsigned int n_qpoints,
                unsigned int n_sides);
  ~XFEMCutElem2D();

private:
  EFAElement2D _efa_elem2d; // 2D EFAelement
  virtual Point getNodeCoordinates(EFANode * node, MeshBase * displaced_mesh = NULL) const;

public:
  virtual void computePhysicalVolumeFraction();
  virtual void computePhysicalFaceAreaFraction(unsigned int side);
  virtual void computeMomentFittingWeights();
  virtual Point getCutPlaneOrigin(unsigned int plane_id, MeshBase * displaced_mesh = NULL) const;
  virtual Point getCutPlaneNormal(unsigned int plane_id, MeshBase * displaced_mesh = NULL) const;
  virtual void
  getCrackTipOriginAndDirection(unsigned tip_id, Point & origin, Point & direction) const;
  virtual void getFragmentFaces(std::vector<std::vector<Point>> & frag_faces,
                                MeshBase * displaced_mesh = NULL) const;
  virtual const EFAElement * getEFAElement() const;
  virtual unsigned int numCutPlanes() const;
  virtual void getIntersectionInfo(unsigned int plane_id,
                                   Point & normal,
                                   std::vector<Point> & intersectionPoints,
                                   MeshBase * displaced_mesh = NULL) const;

private:
  void getPhysicalQuadraturePoints(std::vector<std::vector<Real>> & tsg);
  void solveMomentFitting(unsigned int nen,
                          unsigned int nqp,
                          std::vector<Point> & elem_nodes,
                          std::vector<std::vector<Real>> & tsg,
                          std::vector<std::vector<Real>> & wsg);
};
