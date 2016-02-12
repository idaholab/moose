/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef XFEMCUTELEM3D_H
#define XFEMCUTELEM3D_H

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
  XFEMCutElem3D(Elem* elem, const EFAElement3D * const CEMelem, unsigned int n_qpoints);
  ~XFEMCutElem3D();

private:

  EFAElement3D _efa_elem3d; // 3D EFAelement
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
};

#endif
