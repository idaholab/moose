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
#include <map>
#include <set>
#include <ostream>

class EFANode;

class EFAElement
{
public:
  EFAElement(unsigned int eid, unsigned int n_nodes);

  virtual ~EFAElement();

protected:
  unsigned int _id;
  unsigned int _num_nodes;
  std::vector<EFANode *> _nodes;
  std::vector<EFANode *> _local_nodes;
  EFAElement * _parent;
  std::vector<EFAElement *> _children;
  bool _crack_tip_split_element;
  std::vector<unsigned int> _crack_tip_neighbors;
  std::vector<EFAElement *>
      _general_neighbors; // all elements sharing at least one node with curr elem

public:
  // common methods
  unsigned int id() const;
  unsigned int numNodes() const;
  void setNode(unsigned int node_id, EFANode * node);
  EFANode * getNode(unsigned int node_id) const;
  bool containsNode(EFANode * node) const;
  void printNodes(std::ostream & ostream) const;
  EFANode * createLocalNodeFromGlobalNode(const EFANode * global_node) const;
  EFANode * getGlobalNodeFromLocalNode(const EFANode * local_node) const;
  unsigned int getLocalNodeIndex(EFANode * node) const;

  void setCrackTipSplit();
  bool isCrackTipSplit() const;
  unsigned int numCrackTipNeighbors() const;
  unsigned int getCrackTipNeighbor(unsigned int index) const;
  void addCrackTipNeighbor(EFAElement * neighbor_elem);

  EFAElement * getParent() const;
  EFAElement * getChild(unsigned int child_id) const;
  void setParent(EFAElement * parent);
  unsigned int numChildren() const;
  void addChild(EFAElement * child);
  void clearParentAndChildren();
  void findGeneralNeighbors(std::map<EFANode *, std::set<EFAElement *>> & InverseConnectivity);
  EFAElement * getGeneralNeighbor(unsigned int index) const;
  unsigned int numGeneralNeighbors() const;

  // pure virtual methods
  virtual unsigned int numFragments() const = 0;
  virtual bool isPartial() const = 0;
  virtual void getNonPhysicalNodes(std::set<EFANode *> & non_physical_nodes) const = 0;

  virtual void switchNode(EFANode * new_node, EFANode * old_node, bool descend_to_parent) = 0;
  virtual void switchEmbeddedNode(EFANode * new_node, EFANode * old_node) = 0;
  virtual void updateFragmentNode() = 0;
  virtual void getMasterInfo(EFANode * node,
                             std::vector<EFANode *> & master_nodes,
                             std::vector<double> & master_weights) const = 0;
  virtual unsigned int numInteriorNodes() const = 0;

  virtual unsigned int getNeighborIndex(const EFAElement * neighbor_elem) const = 0;
  virtual void clearNeighbors() = 0;
  virtual void
  setupNeighbors(std::map<EFANode *, std::set<EFAElement *>> & InverseConnectivityMap) = 0;
  virtual void neighborSanityCheck() const = 0;

  virtual void initCrackTip(std::set<EFAElement *> & CrackTipElements) = 0;
  virtual bool shouldDuplicateForCrackTip(const std::set<EFAElement *> & CrackTipElements) = 0;
  virtual bool
  shouldDuplicateCrackTipSplitElement(const std::set<EFAElement *> & CrackTipElements) = 0;
  virtual bool shouldDuplicateForPhantomCorner() = 0;
  virtual bool willCrackTipExtend(std::vector<unsigned int> & split_neighbors) const = 0;
  virtual bool isCrackTipElement() const = 0;

  virtual unsigned int getNumCuts() const = 0;
  virtual bool isFinalCut() const = 0;
  virtual void updateFragments(const std::set<EFAElement *> & CrackTipElements,
                               std::map<unsigned int, EFANode *> & EmbeddedNodes) = 0;
  virtual void fragmentSanityCheck(unsigned int n_old_frag_edges,
                                   unsigned int n_old_frag_cuts) const = 0;
  virtual void restoreFragment(const EFAElement * const from_elem) = 0;

  virtual void createChild(const std::set<EFAElement *> & CrackTipElements,
                           std::map<unsigned int, EFAElement *> & Elements,
                           std::map<unsigned int, EFAElement *> & newChildElements,
                           std::vector<EFAElement *> & ChildElements,
                           std::vector<EFAElement *> & ParentElements,
                           std::map<unsigned int, EFANode *> & TempNodes) = 0;
  virtual void removePhantomEmbeddedNode() = 0;
  virtual void
  connectNeighbors(std::map<unsigned int, EFANode *> & PermanentNodes,
                   std::map<unsigned int, EFANode *> & TempNodes,
                   std::map<EFANode *, std::set<EFAElement *>> & InverseConnectivityMap,
                   bool merge_phantom_edges) = 0;
  virtual void printElement(std::ostream & ostream) const = 0;

protected:
  // common methods
  void mergeNodes(EFANode *& childNode,
                  EFANode *& childOfNeighborNode,
                  EFAElement * childOfNeighborElem,
                  std::map<unsigned int, EFANode *> & PermanentNodes,
                  std::map<unsigned int, EFANode *> & TempNodes);
};
