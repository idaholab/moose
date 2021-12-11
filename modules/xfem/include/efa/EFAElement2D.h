//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "EFAElement.h"
#include "EFAPoint.h"

class EFAEdge;
class EFAFace;
class EFAFaceNode;
class EFAFragment2D;

class EFAElement2D : public EFAElement
{
public:
  EFAElement2D(unsigned int eid, unsigned int n_nodes);
  EFAElement2D(const EFAElement2D * from_elem, bool convert_to_local);
  EFAElement2D(const EFAFace * from_face);

  ~EFAElement2D();

private:
  unsigned int _num_edges;
  std::vector<EFAEdge *> _edges;
  std::vector<EFAFaceNode *> _interior_nodes;
  std::vector<std::vector<EFAElement2D *>> _edge_neighbors;
  std::vector<EFAFragment2D *> _fragments;
  std::vector<EFAPoint> _local_node_coor;
  // Index of the current cut plane
  unsigned int _current_cut_plane_idx = 0;
  // Vector of cut plane indices
  std::vector<unsigned int> _cut_plane_idx;
  // Vector of vectors tracking nodes cut by each cut index referenced against _cut_plane_idx
  std::vector<std::vector<EFANode *>> _cut_plane_nodes;

public:
  // override virtual methods in base class
  virtual unsigned int numFragments() const;
  virtual bool isPartial() const;
  virtual void getNonPhysicalNodes(std::set<EFANode *> & non_physical_nodes) const;

  virtual void switchNode(EFANode * new_node, EFANode * old_node, bool descend_to_parent);
  virtual void switchEmbeddedNode(EFANode * new_node, EFANode * old_node);
  virtual void updateFragmentNode();
  virtual void getMasterInfo(EFANode * node,
                             std::vector<EFANode *> & master_nodes,
                             std::vector<double> & master_weights) const;
  virtual unsigned int numInteriorNodes() const;

  bool overlaysElement(const EFAElement2D * other_elem) const;
  virtual unsigned int getNeighborIndex(const EFAElement * neighbor_elem) const;
  virtual void clearNeighbors();
  virtual void setupNeighbors(std::map<EFANode *, std::set<EFAElement *>> & InverseConnectivityMap);
  virtual void neighborSanityCheck() const;

  virtual void initCrackTip(std::set<EFAElement *> & CrackTipElements);
  virtual bool shouldDuplicateForCrackTip(const std::set<EFAElement *> & CrackTipElements);
  virtual bool shouldDuplicateCrackTipSplitElement(const std::set<EFAElement *> & CrackTipElements);
  virtual bool shouldDuplicateForPhantomCorner();
  virtual bool willCrackTipExtend(std::vector<unsigned int> & split_neighbors) const;
  virtual bool isCrackTipElement() const;

  virtual unsigned int getNumCuts() const;
  virtual bool isFinalCut() const;
  virtual void updateFragments(const std::set<EFAElement *> & CrackTipElements,
                               std::map<unsigned int, EFANode *> & EmbeddedNodes);
  virtual void fragmentSanityCheck(unsigned int n_old_frag_edges,
                                   unsigned int n_old_frag_cuts) const;
  virtual void restoreFragment(const EFAElement * const from_elem);

  virtual void createChild(const std::set<EFAElement *> & CrackTipElements,
                           std::map<unsigned int, EFAElement *> & Elements,
                           std::map<unsigned int, EFAElement *> & newChildElements,
                           std::vector<EFAElement *> & ChildElements,
                           std::vector<EFAElement *> & ParentElements,
                           std::map<unsigned int, EFANode *> & TempNodes);
  virtual void removePhantomEmbeddedNode();
  virtual void
  connectNeighbors(std::map<unsigned int, EFANode *> & PermanentNodes,
                   std::map<unsigned int, EFANode *> & TempNodes,
                   std::map<EFANode *, std::set<EFAElement *>> & InverseConnectivityMap,
                   bool merge_phantom_edges);
  virtual void printElement(std::ostream & ostream) const;

  // EFAelement2D specific methods
  EFAFragment2D * getFragment(unsigned int frag_id) const;
  std::set<EFANode *> getEdgeNodes(unsigned int edge_id) const;
  bool getEdgeNodeParametricCoordinate(EFANode * node, std::vector<double> & para_coor) const;
  EFAFaceNode * getInteriorNode(unsigned int interior_node_id) const;
  void deleteInteriorNodes();

  unsigned int numEdges() const;
  void setEdge(unsigned int edge_id, EFAEdge * edge);
  void createEdges();
  EFAEdge * getEdge(unsigned int edge_id) const;

  EFAEdge * getFragmentEdge(unsigned int frag_id, unsigned int edge_id) const;
  std::set<EFANode *> getPhantomNodeOnEdge(unsigned int edge_id) const;
  bool getFragmentEdgeID(unsigned int elem_edge_id, unsigned int & frag_edge_id) const;
  bool isEdgePhantom(unsigned int edge_id) const;

  unsigned int numEdgeNeighbors(unsigned int edge_id) const;
  EFAElement2D * getEdgeNeighbor(unsigned int edge_id, unsigned int neighbor_id) const;

  unsigned int getCrackTipSplitElementID() const;

  bool fragmentHasTipEdges() const;
  unsigned int getTipEdgeID() const;
  EFANode * getTipEmbeddedNode() const;
  bool edgeContainsTip(unsigned int edge_id) const;
  bool fragmentEdgeAlreadyCut(unsigned int ElemEdgeID) const;

  void addEdgeCut(unsigned int edge_id,
                  double position,
                  EFANode * embedded_node,
                  std::map<unsigned int, EFANode *> & EmbeddedNodes,
                  bool add_to_neighbor);
  void addNodeCut(unsigned int node_id,
                  EFANode * embedded_permanent_node,
                  std::map<unsigned int, EFANode *> & PermanentNodes,
                  std::map<unsigned int, EFANode *> & EmbeddedPermanentNodes);
  bool addFragmentEdgeCut(unsigned int frag_edge_id,
                          double position,
                          std::map<unsigned int, EFANode *> & EmbeddedNodes);
  std::vector<EFAFragment2D *> branchingSplit(std::map<unsigned int, EFANode *> & EmbeddedNodes);

  std::vector<EFANode *> getCommonNodes(const EFAElement2D * other_elem) const;

  /**
   * Add a new face node to 2d element
   * @param faceNode   The EFAFaceNode object to be added to the vector for this element's interior
   * nodes
   */
  void addInteriorNode(EFAFaceNode * faceNode);

  /**
   * Return boolean answer for if the input node is on the interior of the current element
   * @param node   The EFANode object to be compared against the vector of interior nodes
   */
  bool isInteriorNode(EFANode * node) const;
  /**
   * Return the faceNode object containing the input EFANode
   * @param node   The EFANode object to be compared against the vector of interior nodes, then
   * returning the faceNode containing the input node
   */
  EFAFaceNode * getFaceNode(EFANode * node) const;
  /**
   * Return boolean answer for if the input node's coordinates are found
   * @param node   The EFANode object to be checked against interior nodes and then edge nodes to
   * find its parametric coordinates
   * @param para_coor   The vector of doubles, initially empty but parametric coordinates are added
   * once found in either interior node or edge node
   */
  bool getNodeParametricCoordinate(EFANode * node, std::vector<double> & para_coor) const;

  void getNewCutPlaneIdx();
  unsigned int getCurrentCutPlaneIdx();
  void addNodeToCutPlaneIdx(EFANode * node, unsigned int cut);
  void reassignNodeToCutPlaneIdx(EFANode * node, unsigned int cut);
  std::vector<std::vector<EFANode *>> getCutPlaneNodes();
  std::vector<unsigned int> getCutPlaneIndices();
  void removeCutPlaneNode(EFANode * node, unsigned int cut);
  void removeCutPlane(unsigned int cut);
  bool hasSameCut(EFANode * otherNode, unsigned int cut);
  unsigned int numSoloNodes();
  void pairSoloNodes();

  void addNodeCutToNeighbors(EFANode * cut_node);

private:
  // given the 1D parent coord of a point in an 2D element edge, translate it to 2D parametric
  // coords
  void mapParametricCoordFrom1Dto2D(unsigned int edge_id,
                                    double xi_1d,
                                    std::vector<double> & para_coor) const;
  void setLocalCoordinates();
};
