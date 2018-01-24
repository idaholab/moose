//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef XFEM_H
#define XFEM_H

#include "ElementPairLocator.h"
#include "ElementFragmentAlgorithm.h"
#include "XFEMInterface.h"
#include "XFEMCrackGrowthIncrement2DCut.h"

#include "libmesh/vector_value.h"
#include "libmesh/quadrature.h"

#include "GeometricCutUserObject.h"

// Forward declarations
class SystemBase;

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
} // namespace Xfem

class XFEMCutElem;
class XFEMCrackGrowthIncrement2DCut;
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
  explicit XFEM(const InputParameters & params);

  ~XFEM();

  void addGeometricCut(const GeometricCutUserObject * geometric_cut);

  void addStateMarkedElem(unsigned int elem_id, RealVectorValue & normal);
  void addStateMarkedElem(unsigned int elem_id, RealVectorValue & normal, unsigned int marked_side);
  void addStateMarkedFrag(unsigned int elem_id, RealVectorValue & normal);

  void clearStateMarkedElems();

  /**
   * Method to update the mesh due to modified cut planes
   */
  virtual bool update(Real time, NonlinearSystemBase & nl, AuxiliarySystem & aux);

  /**
   * Initialize the solution on newly created nodes
   */
  virtual void initSolution(NonlinearSystemBase & nl, AuxiliarySystem & aux);

  void buildEFAMesh();
  bool markCuts(Real time);
  bool markCutEdgesByGeometry(Real time);
  bool markCutEdgesByState(Real time);
  bool markCutFacesByGeometry(Real time);
  bool markCutFacesByState();
  bool initCutIntersectionEdge(
      Point cut_origin, RealVectorValue cut_normal, Point & edge_p1, Point & edge_p2, Real & dist);
  bool cutMeshWithEFA(NonlinearSystemBase & nl, AuxiliarySystem & aux);
  Point getEFANodeCoords(EFANode * CEMnode,
                         EFAElement * CEMElem,
                         const Elem * elem,
                         MeshBase * displaced_mesh = NULL) const;

  /**
   * Get the volume fraction of an element that is physical
   */
  Real getPhysicalVolumeFraction(const Elem * elem) const;

  /**
   * Get specified component of normal or origin for cut plane for a given element
   */
  Real getCutPlane(const Elem * elem,
                   const Xfem::XFEM_CUTPLANE_QUANTITY quantity,
                   unsigned int plane_id) const;

  bool isElemAtCrackTip(const Elem * elem) const;
  bool isElemCut(const Elem * elem, XFEMCutElem *& xfce) const;
  bool isElemCut(const Elem * elem) const;
  void getFragmentFaces(const Elem * elem,
                        std::vector<std::vector<Point>> & frag_faces,
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

  void getCrackTipOrigin(std::map<unsigned int, const Elem *> & elem_id_crack_tip,
                         std::vector<Point> & crack_front_points);
  // void update_crack_propagation_direction(const Elem* elem, Point direction);
  // void clear_crack_propagation_direction();
  /**
   * Set and get xfem cut data and type
   */
  std::vector<Real> & getXFEMCutData();
  void setXFEMCutData(std::vector<Real> & cut_data);
  std::string & getXFEMCutType();
  void setXFEMCutType(std::string & cut_type);
  Xfem::XFEM_QRULE & getXFEMQRule();
  void setXFEMQRule(std::string & xfem_qrule);
  void setCrackGrowthMethod(bool use_crack_growth_increment, Real crack_growth_increment);
  virtual bool getXFEMWeights(MooseArray<Real> & weights,
                              const Elem * elem,
                              QBase * qrule,
                              const MooseArray<Point> & q_points);
  virtual bool getXFEMFaceWeights(MooseArray<Real> & weights,
                                  const Elem * elem,
                                  QBase * qrule,
                                  const MooseArray<Point> & q_points,
                                  unsigned int side);
  virtual const ElementPairLocator::ElementPairList * getXFEMCutElemPairs() const
  {
    return &_sibling_elems;
  }
  virtual const ElementPairLocator::ElementPairList * getXFEMDisplacedCutElemPairs() const
  {
    return &_sibling_displaced_elems;
  }
  virtual void getXFEMIntersectionInfo(const Elem * elem,
                                       unsigned int plane_id,
                                       Point & normal,
                                       std::vector<Point> & intersectionPoints,
                                       bool displaced_mesh = false) const;
  virtual void getXFEMqRuleOnLine(std::vector<Point> & intersection_points,
                                  std::vector<Point> & quad_pts,
                                  std::vector<Real> & quad_wts) const;
  virtual void getXFEMqRuleOnSurface(std::vector<Point> & intersection_points,
                                     std::vector<Point> & quad_pts,
                                     std::vector<Real> & quad_wts) const;
  bool has_secondary_cut() { return _has_secondary_cut; }

private:
  void getFragmentEdges(const Elem * elem,
                        EFAElement2D * CEMElem,
                        std::vector<std::vector<Point>> & frag_edges) const;
  void getFragmentFaces(const Elem * elem,
                        EFAElement3D * CEMElem,
                        std::vector<std::vector<Point>> & frag_faces) const;

private:
  bool _has_secondary_cut;

  Xfem::XFEM_QRULE _XFEM_qrule;

  bool _use_crack_growth_increment;
  Real _crack_growth_increment;

  std::vector<const GeometricCutUserObject *> _geometric_cuts;

  std::map<unique_id_type, XFEMCutElem *> _cut_elem_map;
  std::set<const Elem *> _crack_tip_elems;
  ElementPairLocator::ElementPairList _sibling_elems;
  ElementPairLocator::ElementPairList _sibling_displaced_elems;

  std::map<const Elem *, std::vector<Point>> _elem_crack_origin_direction_map;

  // std::map<const Elem*, Point> _crack_propagation_direction_map;

  std::map<const Elem *, RealVectorValue> _state_marked_elems;
  std::set<const Elem *> _state_marked_frags;
  std::map<const Elem *, unsigned int> _state_marked_elem_sides;

  ElementFragmentAlgorithm _efa_mesh;

  /**
   * Data structure to store the nonlinear solution for nodes/elements affected by XFEM
   * For each node/element, this is stored as a vector that contains all components
   * of all applicable variables in an order defined by getElementSolutionDofs() or
   * getNodeSolutionDofs(). This vector first contains the current solution in that
   * order, followed by the old and older solutions, also in that same order.
   */
  std::map<unique_id_type, std::vector<Real>> _cached_solution;

  /**
   * Data structure to store the auxiliary solution for nodes/elements affected by XFEM
   * For each node/element, this is stored as a vector that contains all components
   * of all applicable variables in an order defined by getElementSolutionDofs() or
   * getNodeSolutionDofs(). This vector first contains the current solution in that
   * order, followed by the old and older solutions, also in that same order.
   */
  std::map<unique_id_type, std::vector<Real>> _cached_aux_solution;

  /**
   * Store the solution in stored_solution for a given node
   * @param node_to_store_to   Node for which the solution will be stored
   * @param node_to_store_from Node from which the solution to be stored is obtained
   * @param sys                System from which the solution is stored
   * @param stored_solution    Data structure that the stored solution is saved to
   * @param current_solution   Current solution vector that the solution is obtained from
   * @param old_solution       Old solution vector that the solution is obtained from
   * @param older_solution     Older solution vector that the solution is obtained from
   */
  void storeSolutionForNode(const Node * node_to_store_to,
                            const Node * node_to_store_from,
                            SystemBase & sys,
                            std::map<unique_id_type, std::vector<Real>> & stored_solution,
                            const NumericVector<Number> & current_solution,
                            const NumericVector<Number> & old_solution,
                            const NumericVector<Number> & older_solution);

  /**
   * Store the solution in stored_solution for a given element
   * @param elem_to_store_to   Element for which the solution will be stored
   * @param elem_to_store_from Element from which the solution to be stored is obtained
   * @param sys                System from which the solution is stored
   * @param stored_solution    Data structure that the stored solution is saved to
   * @param current_solution   Current solution vector that the solution is obtained from
   * @param old_solution       Old solution vector that the solution is obtained from
   * @param older_solution     Older solution vector that the solution is obtained from
   */
  void storeSolutionForElement(const Elem * elem_to_store_to,
                               const Elem * elem_to_store_from,
                               SystemBase & sys,
                               std::map<unique_id_type, std::vector<Real>> & stored_solution,
                               const NumericVector<Number> & current_solution,
                               const NumericVector<Number> & old_solution,
                               const NumericVector<Number> & older_solution);

  /**
   * Set the solution for all locally-owned nodes/elements that have stored values
   * @param sys              System for which the solution is set
   * @param stored_solution  Data structure that the stored solution is obtained from
   * @param current_solution Current solution vector that will be set
   * @param old_solution     Old solution vector that will be set
   * @param older_solution   Older solution vector that will be set
   */
  void setSolution(SystemBase & sys,
                   const std::map<unique_id_type, std::vector<Real>> & stored_solution,
                   NumericVector<Number> & current_solution,
                   NumericVector<Number> & old_solution,
                   NumericVector<Number> & older_solution);

  /**
   * Set the solution for a set of DOFs
   * @param stored_solution      Stored solution values to set the solution to
   * @param stored_solution_dofs Dof indices for the entries in stored_solution
   * @param current_solution     Current solution vector that will be set
   * @param old_solution         Old solution vector that will be set
   * @param older_solution       Older solution vector that will be set
   */
  void setSolutionForDOFs(const std::vector<Real> & stored_solution,
                          const std::vector<dof_id_type> & stored_solution_dofs,
                          NumericVector<Number> & current_solution,
                          NumericVector<Number> & old_solution,
                          NumericVector<Number> & older_solution);

  /**
   * Get a vector of the dof indices for all components of all variables
   * associated with an element
   * @param elem Element for which dof indices are found
   * @param sys  System for which the dof indices are found
   */
  std::vector<dof_id_type> getElementSolutionDofs(const Elem * elem, SystemBase & sys) const;

  /**
   * Get a vector of the dof indices for all components of all variables
   * associated with a node
   * @param node Node for which dof indices are found
   * @param sys  System for which the dof indices are found
   */
  std::vector<dof_id_type> getNodeSolutionDofs(const Node * node, SystemBase & sys) const;
};

#endif // XFEM_H
