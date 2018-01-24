//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EFAEdge.h"

#include "EFANode.h"
#include "EFAError.h"
#include "XFEMFuncs.h"

EFAEdge::EFAEdge(EFANode * node1, EFANode * node2) : _edge_node1(node1), _edge_node2(node2)
{
  _embedded_nodes.clear();
  _intersection_x.clear();
  _edge_interior_node = NULL;
  consistencyCheck();
}

EFAEdge::EFAEdge(const EFAEdge & other_edge)
{
  _edge_node1 = other_edge._edge_node1;
  _edge_node2 = other_edge._edge_node2;
  _intersection_x = other_edge._intersection_x;
  _embedded_nodes = other_edge._embedded_nodes;
  consistencyCheck();
}

EFAEdge::~EFAEdge() // do not delete edge node - they will be deleted
{                   // in EFAelement's destructor
}

bool
EFAEdge::equivalent(const EFAEdge & other) const
{
  bool isEqual = false;
  if (other._edge_node1 == _edge_node1 && other._edge_node2 == _edge_node2)
    isEqual = true;
  else if (other._edge_node2 == _edge_node1 && other._edge_node1 == _edge_node2)
    isEqual = true;

  // For cut along the edge case
  if (isEqual)
  {
    if (_edge_node1->category() == EFANode::N_CATEGORY_EMBEDDED_PERMANENT &&
        _edge_node2->category() == EFANode::N_CATEGORY_EMBEDDED_PERMANENT)
      isEqual = false;
  }
  return isEqual;
}

bool
EFAEdge::isPartialOverlap(const EFAEdge & other) const
{
  return containsEdge(other) || other.containsEdge(*this);
}

bool
EFAEdge::containsEdge(const EFAEdge & other) const
{
  return containsNode(other._edge_node1) && containsNode(other._edge_node2);
}

bool
EFAEdge::getNodeMasters(EFANode * node,
                        std::vector<EFANode *> & master_nodes,
                        std::vector<double> & master_weights) const
{
  master_nodes.clear();
  master_weights.clear();
  bool masters_found = false;
  if (_edge_node1 == node || _edge_node2 == node)
  {
    master_nodes.push_back(node);
    master_weights.push_back(1.0);
    masters_found = true;
  }
  else
  {
    for (unsigned int i = 0; i < _embedded_nodes.size(); ++i)
    {
      if (_embedded_nodes[i] == node)
      {
        master_nodes.push_back(_edge_node1);
        master_nodes.push_back(_edge_node2);
        master_weights.push_back(1.0 - _intersection_x[i]);
        master_weights.push_back(_intersection_x[i]);
        masters_found = true;
        break;
      }
    }
  }
  return masters_found;
}

// TODO: Saving because I don't want to throw it away, but it needs more work to be used.
// bool
// EFAEdge::operator < (const edge_t & other) const
//{
//  node_t * this_min_node;
//  node_t * this_max_node;
//  node_t * other_min_node;
//  node_t * other_max_node;
//
//  int this_node1_unique_index = ((int) edge_node1->category + 1) * edge_node1->id;
//  int this_node2_unique_index = ((int) edge_node2->category + 1) * edge_node2->id;
//  int other_node1_unique_index = ((int) other.edge_node1->category + 1) * edge_node1->id;
//  int other_node2_unique_index = ((int) other.edge_node2->category + 1) * edge_node2->id;
//  int this_min_index = std::min(this_node1_unique_index, this_node2_unique_index);
//  int other_min_index = std::min(other_node1_unique_index, other_node2_unique_index);
//
//  if (this_min_index < other_min_index)
//    return true;
//  else if (this_min_index == other_min_index)
//  {
//    int this_max_index = std::max(this_node1_unique_index, this_node2_unique_index);
//    int other_max_index = std::max(other_node1_unique_index, other_node2_unique_index);
//
//    if (this_max_index < other_max_index)
//      return true;
//  }
//  return false;
//}

void
EFAEdge::addIntersection(double position, EFANode * embedded_node_tmp, EFANode * from_node)
{
  _embedded_nodes.push_back(embedded_node_tmp);
  if (from_node == _edge_node1)
    _intersection_x.push_back(position);
  else if (from_node == _edge_node2)
    _intersection_x.push_back(1.0 - position);
  else
    EFAError("In addIntersection from_node does not exist on edge");
}

void
EFAEdge::resetIntersection(double position, EFANode * embedded_node_tmp, EFANode * from_node)
{
  for (unsigned int i = 0; i < _embedded_nodes.size(); ++i)
  {
    if (_embedded_nodes[i] == embedded_node_tmp)
    {
      if (from_node == _edge_node1)
        _intersection_x[i] = position;
      else if (from_node == _edge_node2)
        _intersection_x[i] = 1.0 - position;
      else
        EFAError("In resetIntersection from_node does not exist on edge");
      break;
    }
  }
}

void
EFAEdge::copyIntersection(const EFAEdge & other, unsigned int from_node_id)
{
  _embedded_nodes.clear();
  _intersection_x.clear();
  _embedded_nodes = other._embedded_nodes;
  if (from_node_id == 0)
    _intersection_x = other._intersection_x;
  else if (from_node_id == 1)
  {
    for (unsigned int i = 0; i < other.numEmbeddedNodes(); ++i)
      _intersection_x.push_back(1.0 - other._intersection_x[i]);
  }
  else
    EFAError("from_node_id out of bounds");
  if (_embedded_nodes.size() != _intersection_x.size())
    EFAError("in copyIntersection num emb_nodes must == num of inters_x");
}

EFANode *
EFAEdge::getNode(unsigned int index) const
{
  if (index == 0)
    return _edge_node1;
  else if (index == 1)
    return _edge_node2;
  else
    EFAError("In getNode index out of bounds");
}

void
EFAEdge::reverseNodes()
{
  std::swap(_edge_node1, _edge_node2);
  for (unsigned int i = 0; i < _embedded_nodes.size(); ++i)
    _intersection_x[i] = 1.0 - _intersection_x[i];
}

bool
EFAEdge::hasIntersection() const
{
  bool has = false;
  if (_edge_node1->parent() != NULL)
    has = has || _edge_node1->parent()->category() == EFANode::N_CATEGORY_EMBEDDED_PERMANENT;

  if (_edge_node2->parent() != NULL)
    has = has || _edge_node2->parent()->category() == EFANode::N_CATEGORY_EMBEDDED_PERMANENT;

  return has || _embedded_nodes.size() > 0;
}

bool
EFAEdge::hasIntersectionAtPosition(double position, EFANode * from_node) const
{
  bool has_int = false;
  if (hasIntersection())
  {
    double tmp_intersection_x = -1.0;
    if (from_node == _edge_node1)
      tmp_intersection_x = position;
    else if (from_node == _edge_node2)
      tmp_intersection_x = 1.0 - position;
    else
      EFAError("In hasIntersectionAtPosition from_node does not exist on edge");

    for (unsigned int i = 0; i < _embedded_nodes.size(); ++i)
    {
      if (std::abs(tmp_intersection_x - _intersection_x[i]) < Xfem::tol)
      {
        has_int = true;
        break;
      }
    }
  }
  return has_int;
}

double
EFAEdge::getIntersection(unsigned int emb_id, EFANode * from_node) const
{
  if (from_node == _edge_node1)
    return _intersection_x[emb_id];
  else if (from_node == _edge_node2)
    return 1.0 - _intersection_x[emb_id];
  else
    EFAError("In getIntersection node not in edge");
}

double
EFAEdge::distanceFromNode1(EFANode * node) const
{
  double xi = -100.0;
  if (_edge_node1 == node)
    xi = 0.0;
  else if (_edge_node2 == node)
    xi = 1.0;
  else if (isEmbeddedNode(node))
  {
    unsigned int embedded_node_id = getEmbeddedNodeIndex(node);
    xi = _intersection_x[embedded_node_id];
  }
  else
    EFAError("the given node is not found in the current edge");
  return xi;
}

bool
EFAEdge::isEmbeddedNode(const EFANode * node) const
{
  bool is_emb = false;
  for (unsigned int i = 0; i < _embedded_nodes.size(); ++i)
  {
    if (_embedded_nodes[i] == node)
    {
      is_emb = true;
      break;
    }
  }
  return is_emb;
}

unsigned int
EFAEdge::getEmbeddedNodeIndex(EFANode * node) const
{
  unsigned int index;
  bool have_index = false;
  for (unsigned int i = 0; i < _embedded_nodes.size(); ++i)
  {
    if (_embedded_nodes[i] == node)
    {
      have_index = true;
      index = i;
      break;
    }
  }
  if (!have_index)
    EFAError("In getEmbeddedNodeIndex, could not find index");
  return index;
}

unsigned int
EFAEdge::getEmbeddedNodeIndex(double position, EFANode * from_node) const
{
  bool have_index = false;
  unsigned int index;
  if (hasIntersection())
  {
    double tmp_intersection_x = -1.0; // dist from edge_node1
    if (from_node == _edge_node1)
      tmp_intersection_x = position;
    else if (from_node == _edge_node2)
      tmp_intersection_x = 1.0 - position;
    else
      EFAError("In getEmbeddedNodeIndex, from_node does not exist on edge");

    for (unsigned int i = 0; i < _embedded_nodes.size(); ++i)
    {
      if (std::abs(tmp_intersection_x - _intersection_x[i]) < Xfem::tol)
      {
        have_index = true;
        index = i;
        break;
      }
    }
  }
  if (!have_index)
    EFAError("In getEmbeddedNodeIndex, could not find index");
  return index;
}

EFANode *
EFAEdge::getEmbeddedNode(unsigned int index) const
{
  if (index < _embedded_nodes.size())
    return _embedded_nodes[index];
  else
    EFAError("in getEmbeddedNode index out of bounds");
}

unsigned int
EFAEdge::numEmbeddedNodes() const
{
  return _embedded_nodes.size();
}

void
EFAEdge::consistencyCheck()
{
  bool consistent = true;
  if ((_edge_node1->category() == EFANode::N_CATEGORY_PERMANENT ||
       _edge_node1->category() == EFANode::N_CATEGORY_TEMP) &&
      _edge_node2->category() == EFANode::N_CATEGORY_LOCAL_INDEX)
    consistent = false;
  else if ((_edge_node2->category() == EFANode::N_CATEGORY_PERMANENT ||
            _edge_node2->category() == EFANode::N_CATEGORY_TEMP) &&
           _edge_node1->category() == EFANode::N_CATEGORY_LOCAL_INDEX)
    consistent = false;
  if (!consistent)
    EFAError("In consistencyCheck nodes on edge are not consistent");
  if (_embedded_nodes.size() != _intersection_x.size())
    EFAError("In consistencyCheck num of emb_nodes must be = num of inters_x");
}

void
EFAEdge::switchNode(EFANode * new_node, EFANode * old_node)
{
  if (_edge_node1 == old_node)
    _edge_node1 = new_node;
  else if (_edge_node2 == old_node)
    _edge_node2 = new_node;
  else if (isEmbeddedNode(old_node))
  {
    unsigned int id = getEmbeddedNodeIndex(old_node);
    _embedded_nodes[id] = new_node;
  }
}

bool
EFAEdge::containsNode(const EFANode * node) const
{
  return _edge_node1 == node || _edge_node2 == node || isEmbeddedNode(node);
}

void
EFAEdge::removeEmbeddedNodes()
{
  _embedded_nodes.clear();
  _intersection_x.clear();
}

void
EFAEdge::removeEmbeddedNode(EFANode * node)
{
  unsigned int index = 0;
  bool node_found = false;
  for (unsigned int i = 0; i < _embedded_nodes.size(); ++i)
  {
    if (_embedded_nodes[i] == node)
    {
      index = i;
      node_found = true;
      break;
    }
  }
  if (node_found)
  {
    _embedded_nodes.erase(_embedded_nodes.begin() + index);
    _intersection_x.erase(_intersection_x.begin() + index);
  }
}
