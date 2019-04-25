//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

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
  /**
   * Constructor initializes XFEMCutElem object
   * @param elem The element on which XFEMCutElem is built
   * @param n_qpoints The number of quadrature points
   * @param n_sides The number of sides which the element has
   */
  XFEMCutElem(Elem * elem, unsigned int n_qpoints, unsigned int n_sides);
  virtual ~XFEMCutElem();

protected:
  unsigned int _n_nodes;
  unsigned int _n_qpoints;
  unsigned int _n_sides;
  std::vector<Node *> _nodes;
  std::vector<Point> _qp_points;
  std::vector<Real> _qp_weights;
  Real _elem_volume;
  std::vector<Real> _elem_side_area;
  Real _physical_volfrac;
  std::vector<Real> _physical_areafrac;
  bool _have_weights;
  std::vector<bool> _have_face_weights;
  /// quadrature weights from volume fraction and moment fitting
  std::vector<Real> _new_weights;
  /// face quadrature weights from surface area fraction
  std::vector<std::vector<Real>> _new_face_weights;
  virtual Point getNodeCoordinates(EFANode * node, MeshBase * displaced_mesh = NULL) const = 0;

public:
  void setQuadraturePointsAndWeights(const std::vector<Point> & qp_points,
                                     const std::vector<Real> & qp_weights);
  /**
   * Computes the volume fraction of the element fragment
   */
  virtual void computePhysicalVolumeFraction() = 0;

  /**
   * Returns the volume fraction of the element fragment
   */
  Real getPhysicalVolumeFraction() const;

  /**
   * Computes the surface area fraction of the element side
   * @param side The side of the element
   */
  virtual void computePhysicalFaceAreaFraction(unsigned int side) = 0;

  /**
   * Returns the surface area fraction of the element side
   * @param side The side of the element
   */
  Real getPhysicalFaceAreaFraction(unsigned int side) const;

  virtual void computeMomentFittingWeights() = 0;
  Real getMomentFittingWeight(unsigned int i_qp) const;
  virtual Point getCutPlaneOrigin(unsigned int plane_id,
                                  MeshBase * displaced_mesh = NULL) const = 0;
  virtual Point getCutPlaneNormal(unsigned int plane_id,
                                  MeshBase * displaced_mesh = NULL) const = 0;
  virtual void
  getCrackTipOriginAndDirection(unsigned tip_id, Point & origin, Point & direction) const = 0;
  virtual void getFragmentFaces(std::vector<std::vector<Point>> & frag_faces,
                                MeshBase * displaced_mesh = NULL) const = 0;
  virtual const EFAElement * getEFAElement() const = 0;
  virtual unsigned int numCutPlanes() const = 0;
  void getWeightMultipliers(MooseArray<Real> & weights,
                            QBase * qrule,
                            Xfem::XFEM_QRULE xfem_qrule,
                            const MooseArray<Point> & q_points);
  void getFaceWeightMultipliers(MooseArray<Real> & face_weights,
                                QBase * qrule,
                                Xfem::XFEM_QRULE xfem_qrule,
                                const MooseArray<Point> & q_points,
                                unsigned int side);

  /**
   * Computes integration weights for the cut element
   * @param qrule The standard MOOSE quadrature rule
   * @param xfem_qrule The integration scheme for the cut element
   * @param q_points The quadrature points for the element
   */
  void computeXFEMWeights(QBase * qrule,
                          Xfem::XFEM_QRULE xfem_qrule,
                          const MooseArray<Point> & q_points);

  /**
   * Computes face integration weights for the cut element side
   * @param qrule The standard MOOSE face quadrature rule
   * @param xfem_qrule The integration scheme for the cut element (We use surface area fraction
   * only)
   * @param q_points The quadrature points for the element side
   * @param side The side of the element
   */
  void computeXFEMFaceWeights(QBase * qrule,
                              Xfem::XFEM_QRULE xfem_qrule,
                              const MooseArray<Point> & q_points,
                              unsigned int side);
  bool isPointPhysical(const Point & p) const;
  virtual void getIntersectionInfo(unsigned int plane_id,
                                   Point & normal,
                                   std::vector<Point> & intersectionPoints,
                                   MeshBase * displaced_mesh = NULL) const = 0;
};
