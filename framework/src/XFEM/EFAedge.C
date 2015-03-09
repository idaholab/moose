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

#include "EFAedge.h"

EFAedge::EFAedge(EFAnode * node1, EFAnode * node2):
  _edge_node1(node1),
  _edge_node2(node2),
  _embedded_node(NULL),
  _intersection_x(-1.0)
{
  consistency_check();
}

EFAedge::EFAedge(const EFAedge & other_edge)
{
  _edge_node1 = other_edge._edge_node1;
  _edge_node2 = other_edge._edge_node2;
  _intersection_x = other_edge._intersection_x;
  _embedded_node = other_edge._embedded_node;
  consistency_check();
}

EFAedge::~EFAedge() // do not delete edge node - they will be deleted
{}                  // in element_t's destructor

bool
EFAedge::equivalent(const EFAedge & other) const
{
  bool isEqual = false;
  double tol = 1.e-4;
  if (other._edge_node1 == _edge_node1 &&
      other._edge_node2 == _edge_node2)
  {
    if (_embedded_node != NULL)
    {
      if (_embedded_node == other._embedded_node && 
          std::abs(_intersection_x - other._intersection_x) < tol)
        isEqual = true;
    }
    else if (other._embedded_node == NULL)
      isEqual = true;
  }
  else if (other._edge_node2 == _edge_node1 &&
           other._edge_node1 == _edge_node2)
  {
    if (_embedded_node != NULL)
    {
      if (_embedded_node == other._embedded_node && 
          std::abs(_intersection_x - 1.0 + other._intersection_x) < tol)
        isEqual = true;
    }
    else if (other._embedded_node == NULL)
      isEqual = true;
  }
  else {}

  return isEqual;
}

bool
EFAedge::isOverlapping(const EFAedge &other) const
{
  bool isEqual = false;
  if (other._edge_node1 == _edge_node1 &&
      other._edge_node2 == _edge_node2)
    isEqual = true;
  else if (other._edge_node2 == _edge_node1 &&
           other._edge_node1 == _edge_node2)
    isEqual = true;
  else {}
  return isEqual;
}

bool
EFAedge::isPartialOverlap(const EFAedge & other) const
{
  bool contains = false;
  if (containsEdge(other) || other.containsEdge(*this))
    contains = true;
  return contains;
}

bool
EFAedge::containsEdge(const EFAedge & other) const
{
  bool contains = false;
  if (containsNode(other._edge_node1) &&
      containsNode(other._edge_node2))
    contains = true;
  return contains;
}

bool
EFAedge::getNodeMasters(EFAnode* node, std::vector<EFAnode*> &master_nodes, std::vector<double> &master_weights)
{
  master_nodes.clear();
  master_weights.clear();
  bool masters_found = true;
  if (_edge_node1 == node)
  {
    master_nodes.push_back(_edge_node1);
    master_weights.push_back(1.0);
  }
  else if (_edge_node2 == node)
  {
    master_nodes.push_back(_edge_node2);
    master_weights.push_back(1.0);
  }
  else if (_embedded_node == node) // edge's embedded node == node
  {
    master_nodes.push_back(_edge_node1);
    master_nodes.push_back(_edge_node2);
    master_weights.push_back(1.0-_intersection_x);
    master_weights.push_back(_intersection_x);
  }
  else
    masters_found = false;
  return masters_found;
}

// TODO: Saving because I don't want to throw it away, but it needs more work to be used.
//bool
//CutElemMesh::edge_t::operator < (const edge_t & other) const
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
EFAedge::add_intersection(double position, EFAnode * embedded_node_tmp, EFAnode * from_node)
{
  _embedded_node = embedded_node_tmp;
  if (from_node == _edge_node1)
    _intersection_x = position;
  else if (from_node == _edge_node2)
    _intersection_x = 1.0 - position;
  else
    mooseError("In add_intersection from_node does not exist on edge");
}

EFAnode *
EFAedge::get_node(unsigned int index)
{
  if (index == 0)
    return _edge_node1;
  else if (index == 1)
    return _edge_node2;
  else
    mooseError("In get_node index out of bounds");
}

bool
EFAedge::has_intersection()
{
  return (_embedded_node != NULL);
}

bool
EFAedge::has_intersection_at_position(double position, EFAnode * from_node)
{
  double tol = 1.e-4;
  bool has_int = false;
  if (has_intersection())
  {
    double tmp_intersection_x = -1.0;
    if (from_node == _edge_node1)
      tmp_intersection_x = position;
    else if (from_node == _edge_node2)
      tmp_intersection_x = 1.0 - position;
    else
      mooseError("In has_intersection from_node does not exist on edge");

    if (std::abs(tmp_intersection_x - _intersection_x) < tol)
      has_int = true;
  }
  return has_int;
}

double
EFAedge::get_intersection(EFAnode * from_node)
{
  if (from_node == _edge_node1)
    return _intersection_x;
  else if (from_node == _edge_node2)
    return 1.0 - _intersection_x;
  else
    mooseError("In get_intersection node not in edge");
}

double
EFAedge::get_xi(EFAnode * node)
{
  // get the parametric coords of the input node
  double xi = -100.0;
  if (_edge_node1 == node)
    xi = 0.0;
  else if (_edge_node2 == node)
    xi = 1.0;
  else if (_embedded_node == node)
    xi = _intersection_x;
  else
    mooseError("the given node is not found in the current edge");
  return xi;
}

EFAnode *
EFAedge::get_embedded_node()
{
  return _embedded_node;
}

void
EFAedge::consistency_check()
{
  bool consistent = true;
  if ((_edge_node1->category() == N_CATEGORY_PERMANENT ||
       _edge_node1->category() == N_CATEGORY_TEMP) &&
       _edge_node2->category() == N_CATEGORY_LOCAL_INDEX)
    consistent = false;
  else if ((_edge_node2->category() == N_CATEGORY_PERMANENT ||
            _edge_node2->category() == N_CATEGORY_TEMP) &&
            _edge_node1->category() == N_CATEGORY_LOCAL_INDEX)
    consistent = false;
  if (!consistent)
    mooseError("In consistency_check nodes on edge are not consistent");
}

void
EFAedge::switchNode(EFAnode *new_node, EFAnode *old_node)
{
  if (_edge_node1 == old_node)
    _edge_node1 = new_node;
  else if (_edge_node2 == old_node)
    _edge_node2 = new_node;
  else if (_embedded_node == old_node)
    _embedded_node = new_node;
}

bool
EFAedge::containsNode(EFAnode *node) const
{
  if (_edge_node1 == node ||
      _edge_node2 == node ||
      _embedded_node == node)
    return true;
  else
    return false;
}

bool
EFAedge::is_interior_edge()
{
  if (_edge_node1->category() == N_CATEGORY_EMBEDDED &&
      _edge_node2->category() == N_CATEGORY_EMBEDDED)
    return true;
  else
    return false;
}

bool
EFAedge::is_elem_full_edge()
{
  if (_edge_node1->category() != N_CATEGORY_EMBEDDED &&
      _edge_node2->category() != N_CATEGORY_EMBEDDED)
    return true;
  else
    return false;
}

void
EFAedge::remove_embedded_node()
{
  _embedded_node = NULL;
  _intersection_x = -1.0;
}
