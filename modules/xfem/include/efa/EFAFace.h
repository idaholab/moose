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

class EFANode;
class EFAEdge;
class EFAFaceNode;
class EFAFragment2D;

class EFAFace
{
public:
  EFAFace(unsigned int n_nodes, unsigned int num_interior_face_nodes = 0);
  EFAFace(const EFAFace & other_face);
  EFAFace(const EFAFragment2D * frag);

  ~EFAFace();

private:
  unsigned int _num_nodes;
  std::vector<EFANode *> _nodes;
  unsigned int _num_edges;
  std::vector<EFAEdge *> _edges;
  std::vector<EFAFaceNode *> _interior_nodes;
  std::vector<EFANode *> _face_interior_nodes;

public:
  unsigned int numNodes() const;
  void setNode(unsigned int node_id, EFANode * node);
  EFANode * getNode(unsigned int node_id) const;
  void switchNode(EFANode * new_node, EFANode * old_node);
  bool getMasterInfo(EFANode * node,
                     std::vector<EFANode *> & master_nodes,
                     std::vector<double> & master_weights) const;
  bool getEdgeNodeParametricCoords(EFANode * node, std::vector<double> & xi_2d) const;
  bool getFaceNodeParametricCoords(EFANode * node, std::vector<double> & xi_2d) const;
  unsigned int numInteriorNodes() const;
  void createNodes();

  unsigned int numEdges() const;
  EFAEdge * getEdge(unsigned int edge_id) const;
  void setEdge(unsigned int edge_id, EFAEdge * new_edge);
  void createEdges();
  void combineTwoEdges(unsigned int edge_id1, unsigned int edge_id2);
  void sortEdges();
  void reverseEdges();
  bool isTriOrQuad() const;

  EFANode * getInteriorFaceNode(unsigned int i) const { return _face_interior_nodes[i]; };
  void setInteriorFaceNode(unsigned int i, EFANode * node);
  bool equivalent(const EFAFace * other_face) const;
  bool containsNode(const EFANode * node) const;
  bool containsFace(const EFAFace * other_face) const;
  bool ownsEdge(const EFAEdge * other_edge) const;
  void removeEmbeddedNode(EFANode * emb_node);
  std::vector<EFAFace *> split() const;
  EFAFace * combineWithFace(const EFAFace * other_face) const;
  void resetEdgeIntersection(const EFAFace * ref_face);

  unsigned int getNumCuts() const;
  bool hasIntersection() const;
  void copyIntersection(const EFAFace & from_face);
  bool isAdjacent(const EFAFace * other_face) const;
  unsigned int adjacentCommonEdge(const EFAFace * other_face) const;
  bool hasSameOrientation(const EFAFace * other_face) const;
  EFAFaceNode * getInteriorNode(unsigned int index) const;

private:
  void mapParametricCoordsFrom1DTo2D(unsigned int edge_id,
                                     double xi_1d,
                                     std::vector<double> & xi_2d) const;
};
