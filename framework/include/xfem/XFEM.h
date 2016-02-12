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

#ifndef XFEM_H
#define XFEM_H

#include "ElementFragmentAlgorithm.h"
#include "ConsoleStreamInterface.h"
#include "MooseTypes.h"
#include "MooseVariableBase.h"

#include "libmesh/vector_value.h"
#include "libmesh/quadrature.h"

enum XFEM_CUTPLANE_QUANTITY
{
  XCC_ORIGIN_X,
  XCC_ORIGIN_Y,
  XCC_ORIGIN_Z,
  XCC_NORMAL_X,
  XCC_NORMAL_Y,
  XCC_NORMAL_Z
};

enum XFEM_QRULE
{
  VOLFRAC,
  MOMENT_FITTING,
  DIRECT
};

class XFEMCutElem;
class XFEMGeometricCut;
class EFANode;
class EFAEdge;
class EFAElement;
class EFAElement2D;
class EFAElement3D;
class MooseApp;
class AuxiliarySystem;
class NonlinearSystem;
class MaterialData;

namespace libMesh
{
  class QBase;
}

/**
 * This is the \p XFEM class.  This class implements
 * algorithms for dynamic mesh modification in support of
 * a phantom node approach for XFEM
 */


// ------------------------------------------------------------
// XFEM class definition
class XFEM : public ConsoleStreamInterface
{
public:

  /**
   * Constructor
   */
  explicit
  XFEM(MooseApp & app, std::vector<MooseSharedPointer<MaterialData> > & material_data, MeshBase* mesh, MeshBase* mesh2=NULL);


  /**
   * Destructor
   */
  ~XFEM();

  void setSecondMesh(MeshBase* mesh2);

  void addGeometricCut(XFEMGeometricCut* geometric_cut);

  void addStateMarkedElem(unsigned int elem_id, RealVectorValue normal);
  void addStateMarkedElem(unsigned int elem_id, RealVectorValue normal, unsigned int marked_side);
  void addStateMarkedFrag(unsigned int elem_id, RealVectorValue normal);

  void clearStateMarkedElems();

  /**
   * Method to update the mesh due to modified cut planes
   */
  bool update(Real time);

  /**
   * Initialize the solution on newly created nodes
   */
  void initSolution(NonlinearSystem & nl, AuxiliarySystem & aux);

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
                   const XFEM_CUTPLANE_QUANTITY quantity,
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
  XFEM_QRULE & getXFEMQRule();
  void setXFEMQRule(std::string & xfem_qrule);
  void setCrackGrowthMethod(bool use_crack_growth_increment, Real crack_growth_increment);
  bool getXFEMWeights(MooseArray<Real> &weights, const Elem * elem, QBase * qrule);

private:

  void getFragmentEdges(const Elem* elem,
                        EFAElement2D* CEMElem,
                        std::vector<std::vector<Point> > &frag_edges) const;
  void getFragmentFaces(const Elem* elem,
                        EFAElement3D* CEMElem,
                        std::vector<std::vector<Point> > &frag_faces) const;

private:
  std::vector<MooseSharedPointer<MaterialData> > & _material_data;

  /**
   * XFEM cut type and data
   */
  std::vector<Real> _XFEM_cut_data;
  std::string _XFEM_cut_type;

  XFEM_QRULE _XFEM_qrule;

  bool _use_crack_growth_increment;
  Real _crack_growth_increment;

  /**
   * Reference to the mesh.
   */
  MeshBase* _mesh;
  MeshBase* _mesh2;
  std::vector<XFEMGeometricCut *> _geometric_cuts;

  std::map<unique_id_type, XFEMCutElem*> _cut_elem_map;
  std::set<const Elem*> _crack_tip_elems;

  std::map<const Elem*, std::vector<Point> > _elem_crack_origin_direction_map;

  //std::map<const Elem*, Point> _crack_propagation_direction_map;

  std::map<const Elem*, RealVectorValue> _state_marked_elems;
  std::set<const Elem*> _state_marked_frags;
  std::map<const Elem*, unsigned int> _state_marked_elem_sides;

  std::map<unique_id_type, unique_id_type> _new_node_to_parent_node;

  ElementFragmentAlgorithm _efa_mesh;
};

#endif // XFEM_H
