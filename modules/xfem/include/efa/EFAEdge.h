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

class EFAEdge
{
public:
  EFAEdge(EFANode * node1, EFANode * node2);
  EFAEdge(const EFAEdge & other_edge);

  ~EFAEdge();

private:
  EFANode * _edge_node1;
  EFANode * _edge_node2;
  EFANode * _edge_interior_node; // The interior edge node for quad8 and quad9 elements
  std::vector<EFANode *> _embedded_nodes;
  std::vector<double> _intersection_x;

public:
  std::pair<EFANode *, EFANode *> getSortedNodes() const
  {
    return {std::min(_edge_node1, _edge_node2), std::max(_edge_node1, _edge_node2)};
  }

  bool equivalent(const EFAEdge & other) const;
  bool isEmbeddedPermanent() const;

  bool isPartialOverlap(const EFAEdge & other) const;
  bool containsEdge(const EFAEdge & other) const;
  bool getNodeMasters(EFANode * node,
                      std::vector<EFANode *> & master_nodes,
                      std::vector<double> & master_weights) const;
  //  bool operator < (const EFAEdge & other) const;

  void addIntersection(double position, EFANode * embedded_node_tmp, EFANode * from_node);
  void resetIntersection(double position, EFANode * embedded_node_tmp, EFANode * from_node);
  void copyIntersection(const EFAEdge & other, unsigned int from_node_id);
  EFANode * getNode(unsigned int index) const;
  EFANode * getInteriorNode() const { return _edge_interior_node; };
  void setInteriorNode(EFANode * node) { _edge_interior_node = node; };
  void reverseNodes();

  bool hasIntersection() const;
  bool hasIntersectionAtPosition(double position, EFANode * from_node) const;
  double getIntersection(unsigned int emb_id, EFANode * from_node) const;
  double distanceFromNode1(EFANode * node) const;
  bool isEmbeddedNode(const EFANode * node) const;
  unsigned int getEmbeddedNodeIndex(EFANode * node) const;
  unsigned int getEmbeddedNodeIndex(double position, EFANode * from_node) const;

  EFANode * getEmbeddedNode(unsigned int index) const;
  unsigned int numEmbeddedNodes() const;
  void consistencyCheck();
  void switchNode(EFANode * new_node, EFANode * old_node);
  // BWS TODO add documentation to this
  bool containsNode(const EFANode * node) const;
  void removeEmbeddedNodes();
  void removeEmbeddedNode(EFANode * node);

  // BWS TODO Delete this new method once I verify that it can be
  // replaced with a call to containsNode() in EFAElement2D.C
  /**
   * Check whether the edge contains the specified EFANode
   * @param node Pointer to the EFANode to be checked
   * @return true if the edge contains node, false otherwise
   */
  bool hasNode(EFANode * node);
};
