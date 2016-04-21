/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMCUTELEM2D_H
#define XFEMCUTELEM2D_H

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

  XFEMCutElem2D(Elem* elem, const EFAElement2D * const CEMelem, unsigned int n_qpoints);
  ~XFEMCutElem2D();

private:

  EFAElement2D _efa_elem2d; // 2D EFAelement
  virtual Point getNodeCoordinates(EFANode* node, MeshBase* displaced_mesh = NULL) const;

public:

  virtual void computePhysicalVolumeFraction();
  virtual void computeMomentFittingWeights();
  virtual Point getCutPlaneOrigin(unsigned int plane_id, MeshBase* displaced_mesh=NULL) const;
  virtual Point getCutPlaneNormal(unsigned int plane_id, MeshBase* displaced_mesh=NULL) const;
  virtual void getCrackTipOriginAndDirection(unsigned tip_id, Point & origin, Point & direction) const;
  virtual void getFragmentFaces(std::vector<std::vector<Point> > &frag_faces, MeshBase* displaced_mesh=NULL) const;
  virtual const EFAElement * getEFAElement() const;
  virtual unsigned int numCutPlanes() const;
  virtual void getIntersectionInfo(unsigned int plane_id, Point & normal, std::vector<Point> & intersectionPoints, MeshBase * displaced_mesh=NULL) const;

private:

  void getPhysicalQuadraturePoints(std::vector<std::vector<Real> > &tsg);
  void solveMomentFitting(unsigned int nen, unsigned int nqp, std::vector<Point> &elem_nodes,
                std::vector<std::vector<Real> > &tsg, std::vector<std::vector<Real> > &wsg);

};

#endif
