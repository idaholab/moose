//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EFAElement.h"

#include "EFANode.h"
#include "EFAError.h"
#include "EFAFuncs.h"

EFAElement::EFAElement(unsigned int eid, unsigned int n_nodes)
  : _id(eid),
    _num_nodes(n_nodes),
    _nodes(_num_nodes, NULL),
    _parent(NULL),
    _crack_tip_split_element(false)
{
}

EFAElement::~EFAElement() {}

unsigned int
EFAElement::id() const
{
  return _id;
}

unsigned int
EFAElement::numNodes() const
{
  return _num_nodes;
}

void
EFAElement::setNode(unsigned int node_id, EFANode * node)
{
  _nodes[node_id] = node;
}

EFANode *
EFAElement::getNode(unsigned int node_id) const
{
  return _nodes[node_id];
}

bool
EFAElement::containsNode(EFANode * node) const
{
  for (unsigned int i = 0; i < _nodes.size(); ++i)
    if (_nodes[i] == node)
      return true;
  return false;
}

void
EFAElement::printNodes(std::ostream & ostream) const
{
  ostream << "***** nodes for element " << _id << " *****" << std::endl;
  for (unsigned int i = 0; i < _num_nodes; ++i)
    ostream << "addr " << _nodes[i] << ", ID " << _nodes[i]->idCatString() << ", category "
            << _nodes[i]->category() << std::endl;
}

EFANode *
EFAElement::createLocalNodeFromGlobalNode(const EFANode * global_node) const
{
  // Given a global node, create a new local node
  if (global_node->category() != EFANode::N_CATEGORY_PERMANENT &&
      global_node->category() != EFANode::N_CATEGORY_TEMP &&
      global_node->category() != EFANode::N_CATEGORY_EMBEDDED_PERMANENT)
    EFAError("In createLocalNodeFromGlobalNode node is not global");

  EFANode * new_local_node = NULL;
  unsigned int inode = 0;
  for (; inode < _nodes.size(); ++inode)
  {
    if (_nodes[inode] == global_node)
    {
      new_local_node = new EFANode(inode, EFANode::N_CATEGORY_LOCAL_INDEX);
      break;
    }
  }
  if (!new_local_node)
    EFAError("In createLocalNodeFromGlobalNode could not find global node");

  return new_local_node;
}

EFANode *
EFAElement::getGlobalNodeFromLocalNode(const EFANode * local_node) const
{
  // Given a local node, find the global node corresponding to that node
  if (local_node->category() != EFANode::N_CATEGORY_LOCAL_INDEX)
    EFAError("In getGlobalNodeFromLocalNode node passed in is not local");

  EFANode * global_node = _nodes[local_node->id()];

  if (global_node->category() != EFANode::N_CATEGORY_PERMANENT &&
      global_node->category() != EFANode::N_CATEGORY_TEMP)
    EFAError("In getGlobalNodeFromLocalNode, the node stored by the element is not global");

  return global_node;
}

unsigned int
EFAElement::getLocalNodeIndex(EFANode * node) const
{
  for (unsigned int i = 0; i < _num_nodes; ++i)
  {
    if (_nodes[i] == node)
      return i;
  }
  EFAError("In EFAelement::getLocalNodeIndex, cannot find the given node");
}

void
EFAElement::setCrackTipSplit()
{
  _crack_tip_split_element = true;
}

bool
EFAElement::isCrackTipSplit() const
{
  return _crack_tip_split_element;
}

unsigned int
EFAElement::numCrackTipNeighbors() const
{
  return _crack_tip_neighbors.size();
}

unsigned int
EFAElement::getCrackTipNeighbor(unsigned int index) const
{
  if (index < _crack_tip_neighbors.size())
    return _crack_tip_neighbors[index];
  else
    EFAError("in getCrackTipNeighbor index out of bounds");
}

void
EFAElement::addCrackTipNeighbor(EFAElement * neighbor_elem)
{
  // Find out what side the specified element is on, and add it as a crack tip neighbor
  // element for that side.
  unsigned int neighbor_index = getNeighborIndex(neighbor_elem);
  bool crack_tip_neighbor_exist = false;
  for (unsigned int i = 0; i < _crack_tip_neighbors.size(); ++i)
  {
    if (_crack_tip_neighbors[i] == neighbor_index)
    {
      crack_tip_neighbor_exist = true;
      break;
    }
  }
  if (!crack_tip_neighbor_exist)
    _crack_tip_neighbors.push_back(neighbor_index);
}

EFAElement *
EFAElement::getParent() const
{
  return _parent;
}

EFAElement *
EFAElement::getChild(unsigned int child_id) const
{
  if (child_id < _children.size())
    return _children[child_id];
  else
    EFAError("child_id out of bounds");
}

void
EFAElement::setParent(EFAElement * parent)
{
  _parent = parent;
}

unsigned int
EFAElement::numChildren() const
{
  return _children.size();
}

void
EFAElement::addChild(EFAElement * child)
{
  _children.push_back(child);
}

void
EFAElement::clearParentAndChildren()
{
  _parent = NULL;
  _children.clear();
}

void
EFAElement::findGeneralNeighbors(std::map<EFANode *, std::set<EFAElement *>> & InverseConnectivity)
{
  _general_neighbors.clear();
  std::set<EFAElement *> patch_elements;
  for (unsigned int inode = 0; inode < _num_nodes; ++inode)
  {
    std::set<EFAElement *> this_node_connected_elems = InverseConnectivity[_nodes[inode]];
    patch_elements.insert(this_node_connected_elems.begin(), this_node_connected_elems.end());
  }

  std::set<EFAElement *>::iterator eit2;
  for (eit2 = patch_elements.begin(); eit2 != patch_elements.end(); ++eit2)
  {
    EFAElement * neigh_elem = *eit2;
    if (neigh_elem != this)
      _general_neighbors.push_back(neigh_elem);
  }
}

EFAElement *
EFAElement::getGeneralNeighbor(unsigned int index) const
{
  return _general_neighbors[index];
}

unsigned int
EFAElement::numGeneralNeighbors() const
{
  return _general_neighbors.size();
}

void
EFAElement::mergeNodes(EFANode *& childNode,
                       EFANode *& childOfNeighborNode,
                       EFAElement * childOfNeighborElem,
                       std::map<unsigned int, EFANode *> & PermanentNodes,
                       std::map<unsigned int, EFANode *> & TempNodes)
{
  // Important: this must be run only on child elements that were just created
  if (!_parent)
    EFAError("no getParent element for child element ", _id, " in mergeNodes");

  EFAElement * childElem = this;
  if (childNode != childOfNeighborNode)
  {
    if (childNode->category() == EFANode::N_CATEGORY_PERMANENT)
    {
      if (childOfNeighborNode->category() == EFANode::N_CATEGORY_PERMANENT)
      {
        if (childOfNeighborNode->parent() == childNode) // merge into childNode
        {
          childOfNeighborElem->switchNode(childNode, childOfNeighborNode, true);
          if (!Efa::deleteFromMap(PermanentNodes, childOfNeighborNode))
          {
            EFAError("Attempted to delete node: ",
                     childOfNeighborNode->id(),
                     " from PermanentNodes, but couldn't find it");
          }
          childOfNeighborNode = childNode;
        }
        else if (childNode->parent() == childOfNeighborNode) // merge into childOfNeighborNode
        {
          childElem->switchNode(childOfNeighborNode, childNode, true);
          if (!Efa::deleteFromMap(PermanentNodes, childNode))
          {
            EFAError("Attempted to delete node: ",
                     childNode->id(),
                     " from PermanentNodes, but couldn't find it");
          }
          childNode = childOfNeighborNode;
        }
        else if (childNode->parent() != NULL &&
                 childNode->parent() == childOfNeighborNode->parent())
        {
          // merge into childNode if both nodes are child permanent
          childOfNeighborElem->switchNode(childNode, childOfNeighborNode, true);
          if (!Efa::deleteFromMap(PermanentNodes,
                                  childOfNeighborNode)) // delete childOfNeighborNode
          {
            EFAError("Attempted to delete node: ",
                     childOfNeighborNode->id(),
                     " from PermanentNodes, but couldn't find it");
          }
          childOfNeighborNode = childNode;
        }
        else
        {
          EFAError("Attempting to merge nodes: ",
                   childNode->id(),
                   " and ",
                   childOfNeighborNode->id(),
                   " but both are permanent themselves");
        }
      }
      else
      {
        if (childOfNeighborNode->parent() != childNode &&
            childOfNeighborNode->parent() != childNode->parent())
        {
          EFAError("Attempting to merge nodes ",
                   childOfNeighborNode->idCatString(),
                   " and ",
                   childNode->idCatString(),
                   " but neither the 2nd node nor its parent is parent of the 1st");
        }
        childOfNeighborElem->switchNode(childNode, childOfNeighborNode, true);
        if (!Efa::deleteFromMap(TempNodes, childOfNeighborNode))
          EFAError("Attempted to delete node: ",
                   childOfNeighborNode->id(),
                   " from TempNodes, but couldn't find it");
        childOfNeighborNode = childNode;
      }
    }
    else if (childOfNeighborNode->category() == EFANode::N_CATEGORY_PERMANENT)
    {
      if (childNode->parent() != childOfNeighborNode &&
          childNode->parent() != childOfNeighborNode->parent())
      {
        EFAError("Attempting to merge nodes ",
                 childNode->id(),
                 " and ",
                 childOfNeighborNode->id(),
                 " but neither the 2nd node nor its parent is parent of the 1st");
      }
      childElem->switchNode(childOfNeighborNode, childNode, true);
      if (!Efa::deleteFromMap(TempNodes, childNode))
        EFAError(
            "Attempted to delete node: ", childNode->id(), " from TempNodes, but couldn't find it");
      childNode = childOfNeighborNode;
    }
    else // both nodes are temporary -- create new permanent node and delete temporary nodes
    {
      unsigned int new_node_id = Efa::getNewID(PermanentNodes);
      EFANode * newNode =
          new EFANode(new_node_id, EFANode::N_CATEGORY_PERMANENT, childNode->parent());
      PermanentNodes.insert(std::make_pair(new_node_id, newNode));

      childOfNeighborElem->switchNode(newNode, childOfNeighborNode, true);
      childElem->switchNode(newNode, childNode, true);

      if (childNode->parent() != childOfNeighborNode->parent())
      {
        EFAError("Attempting to merge nodes ",
                 childNode->id(),
                 " and ",
                 childOfNeighborNode->id(),
                 " but they don't share a common parent");
      }

      if (!Efa::deleteFromMap(TempNodes, childOfNeighborNode))
        EFAError("Attempted to delete node: ",
                 childOfNeighborNode->id(),
                 " from TempNodes, but couldn't find it");
      if (!Efa::deleteFromMap(TempNodes, childNode))
        EFAError(
            "Attempted to delete node: ", childNode->id(), " from TempNodes, but couldn't find it");
      childOfNeighborNode = newNode;
      childNode = newNode;
    }
  }
}
