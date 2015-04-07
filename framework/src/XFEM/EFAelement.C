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

#include "EFAfuncs.h"
#include "EFAelement.h"

EFAelement::EFAelement(unsigned int eid, unsigned int n_nodes):
  _id(eid),
  _num_nodes(n_nodes),
  _nodes(_num_nodes, NULL),
  _parent(NULL),
  _crack_tip_split_element(false)
{}

EFAelement::~EFAelement()
{}

unsigned int
EFAelement::id() const
{
  return _id;
}

unsigned int
EFAelement::num_nodes() const
{
  return _num_nodes;
}

void
EFAelement::set_node(unsigned int node_id, EFAnode* node)
{
  _nodes[node_id] = node;
}

EFAnode*
EFAelement::get_node(unsigned int node_id) const
{
  return _nodes[node_id];
}

void
EFAelement::display_nodes() const
{
  std::cout << "***** display nodes for element " << _id << " *****" << std::endl;
  for (unsigned int i = 0; i < _num_nodes; ++i)
    std::cout << "addr " << _nodes[i] << ", ID " << _nodes[i]->id_cat_str() << ", category " << _nodes[i]->category() << std::endl;
}

EFAnode *
EFAelement::create_local_node_from_global_node(const EFAnode * global_node) const
{
  //Given a global node, create a new local node
  if (global_node->category() != N_CATEGORY_PERMANENT &&
      global_node->category() != N_CATEGORY_TEMP)
    mooseError("In create_local_node_from_global_node node is not global");

  EFAnode * new_local_node = NULL;
  unsigned int inode = 0;
  for (; inode < _nodes.size(); ++inode)
  {
    if (_nodes[inode] == global_node)
    {
      new_local_node = new EFAnode(inode, N_CATEGORY_LOCAL_INDEX);
      break;
    }
  }
  if (!new_local_node)
    mooseError("In create_local_node_from_global_node could not find global node");

  return new_local_node;
}

EFAnode *
EFAelement::get_global_node_from_local_node(const EFAnode * local_node) const
{
  //Given a local node, find the global node corresponding to that node
  if (local_node->category() != N_CATEGORY_LOCAL_INDEX)
    mooseError("In get_global_node_from_local_node node passed in is not local");

  EFAnode * global_node = _nodes[local_node->id()];

  if (global_node->category() != N_CATEGORY_PERMANENT &&
      global_node->category() != N_CATEGORY_TEMP)
    mooseError("In get_global_node_from_local_node, the node stored by the element is not global");

  return global_node;
}

unsigned int
EFAelement::getLocalNodeIndex(EFAnode * node) const
{
  unsigned int local_node_id = 99999;
  for (unsigned int i = 0; i < _num_nodes; ++i)
  {
    if (_nodes[i] == node)
    {
      local_node_id = i;
      break;
    }
  }
  if (local_node_id == 99999)
    mooseError("In EFAelement::getLocalNodeIndex, cannot find the given node");
  return local_node_id;
}

std::vector<EFAnode*>
EFAelement::get_common_nodes(const EFAelement* other_elem) const
{
  std::set<EFAnode*> e1nodes(_nodes.begin(), _nodes.end());
  std::set<EFAnode*> e2nodes(other_elem->_nodes.begin(), other_elem->_nodes.end());
  std::vector<EFAnode*> common_nodes = get_common_elems(e1nodes, e2nodes);
  return common_nodes;
}

void
EFAelement::set_crack_tip_split()
{
  _crack_tip_split_element = true;
}

bool
EFAelement::is_crack_tip_split() const
{
  return _crack_tip_split_element;
}

unsigned int
EFAelement::num_crack_tip_neighbors() const
{
  return _crack_tip_neighbors.size();
}

unsigned int
EFAelement::get_crack_tip_neighbor(unsigned int index) const
{
  if (index < _crack_tip_neighbors.size())
    return _crack_tip_neighbors[index];
  else
    mooseError("in get_crack_tip_neighbor() index out of bounds");
}

void
EFAelement::add_crack_tip_neighbor(EFAelement * neighbor_elem)
{
  //Find out what side the specified element is on, and add it as a crack tip neighbor
  //element for that side.
  unsigned int neighbor_index = get_neighbor_index(neighbor_elem);
  bool crack_tip_neighbor_exist = false;
  for (unsigned int i = 0; i < _crack_tip_neighbors.size(); ++i)
  {
    if (_crack_tip_neighbors[i] == neighbor_index)
      crack_tip_neighbor_exist = true;
  }
  if (!crack_tip_neighbor_exist)
    _crack_tip_neighbors.push_back(neighbor_index);
}

EFAelement*
EFAelement::parent() const
{
  return _parent;
}

EFAelement*
EFAelement::get_child(unsigned int child_id) const
{
  if (child_id < _children.size())
    return _children[child_id];
  else
    mooseError("child_id out of bounds");
}

void
EFAelement::set_parent(EFAelement* parent)
{
  _parent = parent;
}

unsigned int
EFAelement::num_children() const
{
  return _children.size();
}

void
EFAelement::add_child(EFAelement* child)
{
  _children.push_back(child);
}

void
EFAelement::remove_parent_children()
{
  _parent = NULL;
  _children.clear();
}

void
EFAelement::mergeNodes(EFAnode* &childNode, EFAnode* &childOfNeighborNode, EFAelement* childOfNeighborElem,
                       std::map<unsigned int, EFAnode*> &PermanentNodes, std::map<unsigned int, EFAnode*> &TempNodes,
                       std::map<EFAnode*, std::set<EFAelement*> > &InverseConnectivityMap)
{
  // N.B. "this" must point to a child element that was just created
  if (!_parent)
    mooseError("no parent element for child element " << _id << " in mergeNodes");

  EFAelement* childElem = this;
  if (childNode != childOfNeighborNode)
  {
    if(childNode->category() == N_CATEGORY_PERMANENT)
    {
      if(childOfNeighborNode->category() == N_CATEGORY_PERMANENT)
      {
        if (childOfNeighborNode->parent() == childNode) // merge into childNode
        {
          childOfNeighborElem->switchNode(childNode, childOfNeighborNode, true, InverseConnectivityMap);
          if (!deleteFromMap(PermanentNodes, childOfNeighborNode))
          {
            mooseError("Attempted to delete node: "<<childOfNeighborNode->id()
                        <<" from PermanentNodes, but couldn't find it");
          }
          childOfNeighborNode = childNode;
        }
        else if (childNode->parent() == childOfNeighborNode) // merge into childOfNeighborNode
        {
          childElem->switchNode(childOfNeighborNode, childNode, true, InverseConnectivityMap);
          if (!deleteFromMap(PermanentNodes, childNode))
          {
            mooseError("Attempted to delete node: "<<childNode->id()
                            <<" from PermanentNodes, but couldn't find it");
          }
          childNode = childOfNeighborNode;
        }
        else if (childNode->parent() != NULL && childNode->parent() == childOfNeighborNode->parent())
        {
          // merge into childNode if both nodes are child permanent
          childOfNeighborElem->switchNode(childNode, childOfNeighborNode, true, InverseConnectivityMap);
          if (!deleteFromMap(PermanentNodes, childOfNeighborNode)) // delete childOfNeighborNode
          {
            mooseError("Attempted to delete node: "<<childOfNeighborNode->id()
                        <<" from PermanentNodes, but couldn't find it");
          }
          childOfNeighborNode = childNode;
        }
        else
        {
          mooseError("Attempting to merge nodes: "<<childNode->id()<<" and "
                      <<childOfNeighborNode->id()<<" but both are permanent themselves");
        }
      }
      else
      {
        if (childOfNeighborNode->parent() != childNode &&
            childOfNeighborNode->parent() != childNode->parent())
        {
          mooseError("Attempting to merge nodes "<<childOfNeighborNode->id_cat_str()<<" and "
                      <<childNode->id_cat_str()<<" but neither the 2nd node nor its parent is parent of the 1st");
        }
        childOfNeighborElem->switchNode(childNode, childOfNeighborNode, true, InverseConnectivityMap);
        if (!deleteFromMap(TempNodes, childOfNeighborNode))
        {
          mooseError("Attempted to delete node: "<<childOfNeighborNode->id()<<" from TempNodes, but couldn't find it");
        }
        childOfNeighborNode = childNode;
      }
    }
    else if(childOfNeighborNode->category() == N_CATEGORY_PERMANENT)
    {
      if (childNode->parent() != childOfNeighborNode &&
          childNode->parent() != childOfNeighborNode->parent())
      {
        mooseError("Attempting to merge nodes "<<childNode->id()<<" and "
                    <<childOfNeighborNode->id()<<" but neither the 2nd node nor its parent is parent of the 1st");
      }
      childElem->switchNode(childOfNeighborNode, childNode, true, InverseConnectivityMap);
      if (!deleteFromMap(TempNodes, childNode))
      {
        mooseError("Attempted to delete node: "<<childNode->id()<<" from TempNodes, but couldn't find it");
      }
      childNode = childOfNeighborNode;
    }
    else //both nodes are temporary -- create new permanent node and delete temporary nodes
    {
      unsigned int new_node_id = getNewID(PermanentNodes);
      EFAnode* newNode = new EFAnode(new_node_id,N_CATEGORY_PERMANENT,childNode->parent());
      PermanentNodes.insert(std::make_pair(new_node_id,newNode));

      childOfNeighborElem->switchNode(newNode, childOfNeighborNode, true, InverseConnectivityMap);
      childElem->switchNode(newNode, childNode, true, InverseConnectivityMap);

      if (childNode->parent() != childOfNeighborNode->parent())
      {
        mooseError("Attempting to merge nodes "<<childNode->id()<<" and "
                    <<childOfNeighborNode->id()<<" but they don't share a common parent");
      }

      if (!deleteFromMap(TempNodes, childOfNeighborNode))
      {
        mooseError("Attempted to delete node: "<<childOfNeighborNode->id()<<" from TempNodes, but couldn't find it");
      }
      if (!deleteFromMap(TempNodes, childNode))
      {
        mooseError("Attempted to delete node: "<<childNode->id()<<" from TempNodes, but couldn't find it");
      }
      childOfNeighborNode = newNode;
      childNode = newNode;
    }
  }
}
