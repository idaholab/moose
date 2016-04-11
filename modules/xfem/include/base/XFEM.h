/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEM_H
#define XFEM_H

#include <list>
#include "ElementFragmentAlgorithm.h"
#include "XFEMInterface.h"

#include "libmesh/vector_value.h"
#include "libmesh/quadrature.h"

namespace Xfem
{
enum XFEM_CUTPLANE_QUANTITY
{
  ORIGIN_X,
  ORIGIN_Y,
  ORIGIN_Z,
  NORMAL_X,
  NORMAL_Y,
  NORMAL_Z
};

enum XFEM_QRULE
{
  VOLFRAC,
  MOMENT_FITTING,
  DIRECT
};
} //namespace Xfem

class XFEMCutElem;
class XFEMGeometricCut;
class EFANode;
class EFAEdge;
class EFAElement;
class EFAElement2D;
class EFAElement3D;

/**
 * This is the \p XFEM class.  This class implements
 * algorithms for dynamic mesh modification in support of
 * a phantom node approach for XFEM
 */


// ------------------------------------------------------------
// XFEM class definition
class XFEM : public XFEMInterface
{
public:

  explicit
  XFEM(MooseApp & app);

  ~XFEM();

  void addGeometricCut(XFEMGeometricCut* geometric_cut);

  void addStateMarkedElem(unsigned int elem_id, RealVectorValue & normal);
  void addStateMarkedElem(unsigned int elem_id, RealVectorValue & normal, unsigned int marked_side);
  void addStateMarkedFrag(unsigned int elem_id, RealVectorValue & normal);

  void clearStateMarkedElems();

  /**
   * Method to update the mesh due to modified cut planes
   */
  virtual bool update(Real time);

  /**
   * Initialize the solution on newly created nodes
   */
  virtual void initSolution(NonlinearSystem & nl, AuxiliarySystem & aux);

  Node * getNodeFromUniqueID(unique_id_type uid);

  void buildEFAMesh();
  bool markCuts(Real time);
  bool markCutEdgesByGeometry(Real time);
  bool markCutEdgesByState(Real time);
  bool markCutFacesByGeometry(Real time);
  bool markCutFacesByState();
  bool initCutIntersectionEdge(Point cut_origin,
                               RealVectorValue cut_normal,
                               Point &edge_p1,
                               Point &edge_p2,
                               Real &dist);
  bool cutMeshWithEFA();
  Point getEFANodeCoords(EFANode* CEMnode,
                         EFAElement* CEMElem,
                         const Elem *elem,
                         MeshBase* displaced_mesh = NULL) const;

  /**
   * Get the volume fraction of an element that is physical
   */
  Real getPhysicalVolumeFraction(const Elem* elem) const;

  /**
   * Get specified component of normal or origin for cut plane for a given element
   */
  Real getCutPlane(const Elem* elem,
                   const Xfem::XFEM_CUTPLANE_QUANTITY quantity,
                   unsigned int plane_id) const;

  bool isElemAtCrackTip(const Elem* elem) const;
  bool isElemCut(const Elem* elem, XFEMCutElem *&xfce) const;
  bool isElemCut(const Elem* elem) const;
  void getFragmentFaces(const Elem* elem, std::vector<std::vector<Point> > &frag_faces,
                      bool displaced_mesh = false) const;
  void storeCrackTipOriginAndDirection();

  void correctCrackExtensionDirection(const Elem * elem,
                                      EFAElement2D * CEMElem,
                                      EFAEdge * orig_edge,
                                      Point normal,
                                      Point crack_tip_origin,
                                      Point crack_tip_direction,
                                      Real & distance_keep,
                                      unsigned int & edge_id_keep,
                                      Point & normal_keep);

  void getCrackTipOrigin(std::map<unsigned int, const Elem*> & elem_id_crack_tip, std::vector<Point> &  crack_front_points);
  //void update_crack_propagation_direction(const Elem* elem, Point direction);
  //void clear_crack_propagation_direction();
  /**
   * Set and get xfem cut data and type
   */
  std::vector<Real>& getXFEMCutData();
  void setXFEMCutData(std::vector<Real> &cut_data);
  std::string & getXFEMCutType();
  void setXFEMCutType(std::string & cut_type);
  Xfem::XFEM_QRULE & getXFEMQRule();
  void setXFEMQRule(std::string & xfem_qrule);
  void setCrackGrowthMethod(bool use_crack_growth_increment, Real crack_growth_increment);
  virtual bool getXFEMWeights(MooseArray<Real> &weights, const Elem * elem, QBase * qrule, const MooseArray<Point> & q_points);
  virtual const std::list<std::pair<const Elem*, const Elem*> > * getXFEMCutElemPairs() const {return & _sibling_elems;}
  virtual void getXFEMIntersectionInfo(const Elem* elem,
                                       unsigned int plane_id,
                                       Point & normal, std::vector<Point> & intersectionPoints,
                                       bool displaced_mesh = false) const;
  virtual void getXFEMqRuleOnLine(std::vector<Point> & intersection_points,
                                  std::vector<Point> & quad_pts,
                                  std::vector<Real> & quad_wts) const;
  virtual void getXFEMqRuleOnSurface(std::vector<Point> & intersection_points,
                                  std::vector<Point> & quad_pts,
                                  std::vector<Real> & quad_wts) const;
private:

  void getFragmentEdges(const Elem* elem,
                        EFAElement2D* CEMElem,
                        std::vector<std::vector<Point> > &frag_edges) const;
  void getFragmentFaces(const Elem* elem,
                        EFAElement3D* CEMElem,
                        std::vector<std::vector<Point> > &frag_faces) const;

private:

  /**
   * XFEM cut type and data
   */
  std::vector<Real> _XFEM_cut_data;
  std::string _XFEM_cut_type;

  Xfem::XFEM_QRULE _XFEM_qrule;

  bool _use_crack_growth_increment;
  Real _crack_growth_increment;

  std::vector<XFEMGeometricCut *> _geometric_cuts;

  std::map<unique_id_type, XFEMCutElem*> _cut_elem_map;
  std::set<const Elem*> _crack_tip_elems;
  std::list<std::pair<const Elem*, const Elem*> > _sibling_elems;

  std::map<const Elem*, std::vector<Point> > _elem_crack_origin_direction_map;

  //std::map<const Elem*, Point> _crack_propagation_direction_map;

  std::map<const Elem*, RealVectorValue> _state_marked_elems;
  std::set<const Elem*> _state_marked_frags;
  std::map<const Elem*, unsigned int> _state_marked_elem_sides;

  std::map<unique_id_type, unique_id_type> _new_node_to_parent_node;

  ElementFragmentAlgorithm _efa_mesh;
};

#endif // XFEM_H
