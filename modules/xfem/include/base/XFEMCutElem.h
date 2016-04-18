/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMCUTELEM_H
#define XFEMCUTELEM_H

#include <vector>

#include "MooseTypes.h"
#include "XFEM.h"

using namespace libMesh;

namespace libMesh
{
  class MeshBase;
  class Elem;
  class Node;
  class QBase;
}
class EFANode;
class EFAElement;

class XFEMCutElem
{
public:

  XFEMCutElem(Elem* elem, unsigned int n_qpoints);
  virtual ~XFEMCutElem();

protected:

  unsigned int _n_nodes;
  unsigned int _n_qpoints;
  std::vector<Node*> _nodes;
  std::vector<Point> _qp_points;
  std::vector<Real> _qp_weights;
  Real _elem_volume;
  Real _physical_volfrac;
  bool _have_weights;
  std::vector<Real> _new_weights; // quadrature weights from moment fitting
  virtual Point getNodeCoordinates(EFANode* node, MeshBase* displaced_mesh = NULL) const = 0;

public:

  void setQuadraturePointsAndWeights(const std::vector<Point> &qp_points, const std::vector<Real> &qp_weights);
  virtual void computePhysicalVolumeFraction() = 0;
  Real getPhysicalVolumeFraction() const;
  virtual void computeMomentFittingWeights() = 0;
  Real getMomentFittingWeight(unsigned int i_qp) const;
  virtual Point getCutPlaneOrigin(unsigned int plane_id, MeshBase* displaced_mesh=NULL) const = 0;
  virtual Point getCutPlaneNormal(unsigned int plane_id, MeshBase* displaced_mesh=NULL) const = 0;
  virtual void getCrackTipOriginAndDirection(unsigned tip_id, Point & origin, Point & direction) const = 0;
  virtual void getFragmentFaces(std::vector<std::vector<Point> > &frag_faces, MeshBase* displaced_mesh=NULL) const = 0;
  virtual const EFAElement * getEFAElement() const = 0;
  virtual unsigned int numCutPlanes() const = 0;
  void getWeightMultipliers(MooseArray<Real> & weights, QBase * qrule, Xfem::XFEM_QRULE xfem_qrule, const MooseArray<Point> & q_points);
  void computeXFEMWeights(QBase * qrule, Xfem::XFEM_QRULE xfem_qrule, const MooseArray<Point> & q_points);
  bool isPointPhysical(const Point & p) const;
  virtual void getIntersectionInfo(unsigned int plane_id, Point & normal, std::vector<Point> & intersectionPoints, MeshBase * displaced_mesh=NULL) const = 0;
};
#endif
