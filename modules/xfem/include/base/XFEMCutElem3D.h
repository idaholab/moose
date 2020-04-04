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
#include "EFAElement3D.h"

using namespace libMesh;

namespace libMesh
{
class MeshBase;
class Elem;
class Node;
}

class XFEMCutElem3D : public XFEMCutElem
{
public:
  /**
   * Constructor initializes XFEMCutElem3D object
   * @param elem The element on which XFEMCutElem3D is built
   * @param CEMelem The EFAFragment3D object that belongs to XFEMCutElem3D
   * @param n_qpoints The number of quadrature points
   * @param n_sides The number of sides which the element has
   */
  XFEMCutElem3D(Elem * elem,
                const EFAElement3D * const CEMelem,
                unsigned int n_qpoints,
                unsigned int n_sides);
  ~XFEMCutElem3D();

private:
  EFAElement3D _efa_elem3d; // 3D EFAelement
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
};
