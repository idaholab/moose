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

class EFANode;
class EFAFace;
class EFAVolumeNode;
class EFAFragment3D;

class EFAElement3D : public EFAElement
{
public:
  EFAElement3D(unsigned int eid, unsigned int n_nodes, unsigned int n_faces);
  EFAElement3D(const EFAElement3D * from_elem, bool convert_to_local);

  ~EFAElement3D();

private:
  unsigned int _num_faces;
  std::vector<EFAFace *> _faces;
  std::vector<EFAVolumeNode *> _interior_nodes;
  std::vector<std::vector<EFAElement3D *>> _face_neighbors;
  std::vector<std::vector<std::vector<EFAElement3D *>>> _face_edge_neighbors;
  std::vector<EFAFragment3D *> _fragments;
  std::vector<std::vector<EFAFace *>> _faces_adjacent_to_faces;
  unsigned int _num_vertices;
  unsigned int _num_interior_face_nodes;
  std::vector<EFAPoint> _local_node_coor;

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

  bool overlaysElement(const EFAElement3D * other_elem) const;
  virtual unsigned int getNeighborIndex(const EFAElement * neighbor_elem) const;
  virtual void getNeighborEdgeIndex(const EFAElement3D * neighbor_elem,
                                    unsigned int face_id,
                                    unsigned int edge_id,
                                    unsigned int & neigh_face_id,
                                    unsigned int & neigh_edge_id) const;
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
  virtual void fragmentSanityCheck(unsigned int n_old_frag_faces,
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
                   bool merge_phantom_faces);
  virtual void printElement(std::ostream & ostream) const;

  // EFAelement3D specific methods
  EFAFragment3D * getFragment(unsigned int frag_id) const;
  std::set<EFANode *> getFaceNodes(unsigned int face_id) const;
  bool getFaceNodeParametricCoordinates(EFANode * node, std::vector<double> & xi_3d) const;
  EFAVolumeNode * getInteriorNode(unsigned int interior_node_id) const;
  void removeEmbeddedNode(EFANode * emb_node, bool remove_for_neighbor);

  unsigned int numFaces() const;
  void setFace(unsigned int face_id, EFAFace * face);
  void createFaces();
  EFAFace * getFace(unsigned int face_id) const;
  unsigned int getFaceID(EFAFace * face) const;
  std::vector<unsigned int> getCommonFaceID(const EFAElement3D * other_elem) const;
  bool getCommonEdgeID(const EFAElement3D * other_elem,
                       std::vector<unsigned int> & face_id,
                       std::vector<unsigned int> & edge_id) const;
  unsigned int getNeighborFaceNodeID(unsigned int face_id,
                                     unsigned int node_id,
                                     EFAElement3D * neighbor_elem) const;
  unsigned int getNeighborFaceInteriorNodeID(unsigned int face_id,
                                             unsigned int node_id,
                                             EFAElement3D * neighbor_elem) const;
  unsigned int getNeighborFaceEdgeID(unsigned int face_id,
                                     unsigned int edg_id,
                                     EFAElement3D * neighbor_elem) const;
  void findFacesAdjacentToFaces();
  EFAFace * getAdjacentFace(unsigned int face_id, unsigned int edge_id) const;

  EFAFace * getFragmentFace(unsigned int frag_id, unsigned int face_id) const;
  std::set<EFANode *> getPhantomNodeOnFace(unsigned int face_id) const;
  bool getFragmentFaceID(unsigned int elem_face_id, unsigned int & frag_face_id) const;
  bool getFragmentFaceEdgeID(unsigned int ElemFaceID,
                             unsigned int ElemFaceEdgeID,
                             unsigned int & FragFaceID,
                             unsigned int & FragFaceEdgeID) const;
  bool
  isPhysicalEdgeCut(unsigned int ElemFaceID, unsigned int ElemFaceEdgeID, double position) const;
  bool isFacePhantom(unsigned int face_id) const;
  unsigned int numFaceNeighbors(unsigned int face_id) const;
  unsigned int numEdgeNeighbors(unsigned int face_id, unsigned int edge_id) const;
  EFAElement3D * getFaceNeighbor(unsigned int face_id, unsigned int neighbor_id) const;
  EFAElement3D *
  getEdgeNeighbor(unsigned int face_id, unsigned int edge_id, unsigned int neighbor_id) const;

  bool fragmentHasTipFaces() const;
  std::vector<unsigned int> getTipFaceIDs() const;
  std::set<EFANode *> getTipEmbeddedNodes() const;
  bool faceContainsTip(unsigned int face_id) const;
  bool fragmentFaceAlreadyCut(unsigned int ElemFaceID) const;

  void addFaceEdgeCut(unsigned int face_id,
                      unsigned int edge_id,
                      double position,
                      EFANode * embedded_node,
                      std::map<unsigned int, EFANode *> & EmbeddedNodes,
                      bool add_to_neighbor,
                      bool add_to_adjacent);
  void addFragFaceEdgeCut(unsigned int frag_face_id,
                          unsigned int frag_edge_id,
                          double position,
                          std::map<unsigned int, EFANode *> & EmbeddedNodes,
                          bool add_to_neighbor,
                          bool add_to_adjacent);
  std::vector<EFANode *> getCommonNodes(const EFAElement3D * other_elem) const;

private:
  // EFAelement3D specific methods
  void checkNeighborFaceCut(unsigned int face_id,
                            unsigned int edge_id,
                            double position,
                            EFANode * from_node,
                            EFANode * embedded_node,
                            EFANode *& local_embedded);
  void mapParametricCoordinateFrom2DTo3D(unsigned int face_id,
                                         std::vector<double> & xi_2d,
                                         std::vector<double> & xi_3d) const;

  void setLocalCoordinates();
};
