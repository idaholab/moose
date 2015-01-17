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

//TODO:
//Clean up error checking in (!found_edge)
//Save fragment for uncut element ahead of crack tip to avoid renumbering if only embedded node
//Improve overlays_elem() check to not pass in edge
//Add common code to compare neighbors & fragments (replace multiple set_intersection calls)

//Handle cases other than 0 or 2 cut edges/elem (include data structure to link cut edges with cracks?)
//Allow for more than one cut on an edge
//Allow for fragments to be cut recursively
//Support other 2d linear elements (tets)
//Support 2d higher order elements
//3D
//Test more cases

#include <algorithm>
#include <iomanip>
#include <cmath>

#include "CutElemMesh.h"

CutElemMesh::edge_t::edge_t(node_t * node1, node_t * node2):
  edge_node1(node1),
  edge_node2(node2),
  embedded_node(NULL),
  intersection_x(-1.0)
{
  consistency_check();
}

CutElemMesh::edge_t::edge_t(const edge_t & other_edge)
{
  edge_node1 = other_edge.edge_node1;
  edge_node2 = other_edge.edge_node2;
  intersection_x = other_edge.intersection_x;
  embedded_node = other_edge.embedded_node;
  consistency_check();
}

CutElemMesh::edge_t::edge_t(const edge_t & other_edge, 
                            element_t* host, bool convert_to_local)
{
  if (convert_to_local)
  {
    //convert edge nodes from global to local indices if needed
    //N.B. here host must be other_edge's host
    if (other_edge.edge_node1->category == N_CATEGORY_PERMANENT ||
        other_edge.edge_node1->category == N_CATEGORY_TEMP)
      edge_node1 = host->create_local_node_from_global_node(other_edge.edge_node1);
    else
      edge_node1 = other_edge.edge_node1;

    if (other_edge.edge_node2->category == N_CATEGORY_PERMANENT ||
        other_edge.edge_node2->category == N_CATEGORY_TEMP)
      edge_node2 = host->create_local_node_from_global_node(other_edge.edge_node2);
    else
      edge_node2 = other_edge.edge_node2;
  }
  else
  {
    //convert from local to global indices if needed
    //N.B. here host must be this edge's host
    if (other_edge.edge_node1->category == N_CATEGORY_LOCAL_INDEX)
      edge_node1 = host->get_global_node_from_local_node(other_edge.edge_node1);
    else
      edge_node1 = other_edge.edge_node1;

    if (other_edge.edge_node2->category == N_CATEGORY_LOCAL_INDEX)
      edge_node2 = host->get_global_node_from_local_node(other_edge.edge_node2);
    else
      edge_node2 = other_edge.edge_node2;
  }

  intersection_x = other_edge.intersection_x;
  embedded_node = other_edge.embedded_node;
  consistency_check();
}

bool
CutElemMesh::edge_t::equivalent(const edge_t & other) const
{
  bool isEqual = false;
  double tol = 1.e-4;
  if (other.edge_node1 == edge_node1 &&
      other.edge_node2 == edge_node2)
  {
    if (embedded_node != NULL)
    {
      if (embedded_node == other.embedded_node && 
          std::abs(intersection_x - other.intersection_x) < tol)
        isEqual = true;
      else
        isEqual = false;
    }
    else
    {
      isEqual = true;
    }
  }
  else if (other.edge_node2 == edge_node1 &&
           other.edge_node1 == edge_node2)
  {
    if (embedded_node != NULL)
    {
      if (embedded_node == other.embedded_node && 
          std::abs(intersection_x - 1.0 + other.intersection_x) < tol)
        isEqual = true;
      else
        isEqual = false;
    }
    else
    {
      isEqual = true;
    }
  }
  else
    isEqual = false;

  return isEqual;
}

bool
CutElemMesh::edge_t::containsEdge(edge_t & other)
{
  bool contains = false;
  if (equivalent(other))
    contains = true;
  else if (embedded_node)
  {
    if (other.containsNode(embedded_node) &&
       (other.containsNode(edge_node1) || other.containsNode(edge_node2)))
      contains = true;
  }
  else {}
  return contains;
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
CutElemMesh::edge_t::add_intersection(double position, node_t * embedded_node_tmp, node_t * from_node)
{
  embedded_node = embedded_node_tmp;
  if (from_node == edge_node1)
    intersection_x = position;
  else if (from_node == edge_node2)
    intersection_x = 1.0 - position;
  else
    CutElemMeshError("In add_intersection from_node does not exist on edge")
}

void
CutElemMesh::edge_t::replace_embedded_node(node_t * embedded_node_tmp)
{
  embedded_node = embedded_node_tmp;
}

CutElemMesh::node_t *
CutElemMesh::edge_t::get_node(unsigned int index)
{
  if (index == 0)
    return edge_node1;
  else if (index == 1)
    return edge_node2;
  else
    CutElemMeshError("In get_node index out of bounds")
}

bool
CutElemMesh::edge_t::has_intersection()
{
  return (embedded_node != NULL);
}

bool
CutElemMesh::edge_t::has_intersection_at_position(double position, node_t * from_node)
{
  double tol = 1.e-4;
  bool has_int = false;
  if (has_intersection())
  {
    double tmp_intersection_x = -1.0;
    if (from_node == edge_node1)
      tmp_intersection_x = position;
    else if (from_node == edge_node2)
      tmp_intersection_x = 1.0 - position;
    else
      CutElemMeshError("In has_intersection from_node does not exist on edge")

    if (std::abs(tmp_intersection_x - intersection_x) < tol)
      has_int = true;
  }
  return has_int;
}

double
CutElemMesh::edge_t::get_intersection(node_t * from_node)
{
  if (from_node == edge_node1)
    return intersection_x;
  else if (from_node == edge_node2)
    return 1.0 - intersection_x;
  else
    CutElemMeshError("In get_intersection node not in edge")
}

CutElemMesh::node_t *
CutElemMesh::edge_t::get_embedded_node()
{
  return embedded_node;
}

void
CutElemMesh::edge_t::consistency_check()
{
  bool consistent = true;
  if ((edge_node1->category == N_CATEGORY_PERMANENT ||
       edge_node1->category == N_CATEGORY_TEMP) &&
      edge_node2->category == N_CATEGORY_LOCAL_INDEX)
    consistent = false;
  else if ((edge_node2->category == N_CATEGORY_PERMANENT ||
            edge_node2->category == N_CATEGORY_TEMP) &&
           edge_node1->category == N_CATEGORY_LOCAL_INDEX)
    consistent = false;
  if (!consistent)
    CutElemMeshError("In consistency_check nodes on edge are not consistent")
}

void
CutElemMesh::edge_t::switchNode(node_t *new_node, node_t *old_node)
{
  if (edge_node1 == old_node)
    edge_node1 = new_node;
  else if (edge_node2 == old_node)
    edge_node2 = new_node;
}

bool
CutElemMesh::edge_t::containsNode(node_t *node)
{
  if (edge_node1 == node ||
      edge_node2 == node ||
      embedded_node == node)
    return true;
  else
    return false;
}

CutElemMesh::fragment_t::fragment_t(element_t * host,
                                    bool create_boundary_edges,
                                    element_t * from_host,
                                    unsigned int fragment_copy_index)
{
  host_elem = host;
  if (create_boundary_edges)
  {
    if (fragment_copy_index == std::numeric_limits<unsigned int>::max())
    {
      if (!from_host)
        from_host = host;
      for (unsigned int i = 0; i < from_host->edges.size(); ++i)
        boundary_edges.push_back(new edge_t(*from_host->edges[i]));
    }
    else if (fragment_copy_index >= 0)
    {
      if (!from_host)
        CutElemMeshError("If fragment_copy_index given, fragment_t constructor must have a from_host to copy from")
      if (from_host->fragments.size() <= fragment_copy_index)
        CutElemMeshError("In fragment_t constructor fragment_copy_index out of bounds")
      for (unsigned int i = 0; i < from_host->fragments[fragment_copy_index]->boundary_edges.size(); ++i)
        boundary_edges.push_back(new edge_t(*from_host->fragments[fragment_copy_index]->boundary_edges[i]));
    }
    else
      CutElemMeshError("Invalid fragment_copy_index in fragment_t constructor")
  }
}

CutElemMesh::fragment_t::fragment_t(const fragment_t & other_frag,
                                    element_t * host,
                                    bool convert_to_local)
{
  host_elem = host;
  if (convert_to_local)
  {
    // convert boundary edges from global to local indeces if needed
    if (!other_frag.host_elem)
      CutElemMeshError("In fragment_t::fragment_t() fragment_t constructing from must have a valid host_elem to convert from global to local nodes")
    for (unsigned int i = 0; i < other_frag.boundary_edges.size(); ++i)
    {
      edge_t * new_edge = new edge_t(*other_frag.boundary_edges[i], other_frag.host_elem, convert_to_local);
      boundary_edges.push_back(new_edge);
    }
  }
  else
  {
    // convert boundary edges from local to global indeces if needed
    for (unsigned int i = 0; i < other_frag.boundary_edges.size(); ++i)
    {
      edge_t * new_edge = new edge_t(*other_frag.boundary_edges[i], host, convert_to_local);
      boundary_edges.push_back(new_edge);
    }
  }
}

CutElemMesh::fragment_t::~fragment_t()
{
  for (unsigned int i = 0; i < boundary_edges.size(); ++i)
  {
    if (boundary_edges[i])
    {
      delete boundary_edges[i];
      boundary_edges[i] = NULL;
    }
  }
}

void
CutElemMesh::fragment_t::switchNode(node_t *new_node, node_t *old_node)
{
  for (unsigned int i = 0; i < boundary_edges.size(); ++i)
    boundary_edges[i]->switchNode(new_node, old_node);
}

bool
CutElemMesh::fragment_t::containsNode(node_t *node)
{
  bool contains = false;
  for (unsigned int i = 0; i < boundary_edges.size(); ++i)
  {
    if (boundary_edges[i]->containsNode(node))
    {
      contains = true;
      break;
    }
  }
  return contains;
}

bool
CutElemMesh::fragment_t::isConnected(fragment_t &other_fragment)
{
  for (unsigned int i = 0; i < boundary_edges.size(); ++i)
    for (unsigned int j = 0; j < other_fragment.boundary_edges.size(); ++j)
      if (boundary_edges[i]->equivalent(*other_fragment.boundary_edges[j]))
        return true;
  return false;
}

std::vector<CutElemMesh::fragment_t*>
CutElemMesh::fragment_t::split()
{
  // This method will split one existing fragment into one or two
  // new fragments and return them

  std::vector<fragment_t *> new_fragments;
  std::vector<unsigned int> cut_edges;
  for (unsigned int iedge = 0; iedge < boundary_edges.size(); ++iedge)
  {
    if(boundary_edges[iedge]->has_intersection())
      cut_edges.push_back(iedge);
  }

  if (cut_edges.size() == 0)
  {} // do nothing
  else if (cut_edges.size() > 2)
    CutElemMeshError("In split() Fragment cannot have more than 2 cut edges")
  else // cut_edges.size () == 1 || 2
  {
    unsigned int iedge=0;
    unsigned int icutedge=0;

    do //loop over new fragments
    {
      fragment_t * new_frag = new fragment_t(host_elem, false);

      do //loop over edges
      {
        if (iedge == cut_edges[icutedge])
        {
          node_t * first_node_on_edge = boundary_edges[iedge]->get_node(0);
          unsigned int iprevedge(iedge>0 ? iedge-1 : boundary_edges.size()-1);
          if (!boundary_edges[iprevedge]->containsNode(first_node_on_edge))
          {
            first_node_on_edge = boundary_edges[iedge]->get_node(1);
            if (!boundary_edges[iprevedge]->containsNode(first_node_on_edge))
              CutElemMeshError("Previous edge does not contain either of the nodes in this edge")
          }
          node_t * embedded_node1 = boundary_edges[iedge]->get_embedded_node();
          new_frag->boundary_edges.push_back(new edge_t(first_node_on_edge, embedded_node1));

          ++icutedge; // jump to next cut edge or jump back to this edge when only 1 cut edge
          if (icutedge == cut_edges.size())
            icutedge = 0;
          iedge = cut_edges[icutedge];
          node_t * embedded_node2 = boundary_edges[iedge]->get_embedded_node();
          if (embedded_node2 != embedded_node1)
            new_frag->boundary_edges.push_back(new edge_t(embedded_node1, embedded_node2));

          node_t * second_node_on_edge = boundary_edges[iedge]->get_node(1);
          unsigned int inextedge(iedge<(boundary_edges.size()-1) ? iedge+1 : 0);
          if (!boundary_edges[inextedge]->containsNode(second_node_on_edge))
          {
            second_node_on_edge = boundary_edges[iedge]->get_node(0);
            if (!boundary_edges[inextedge]->containsNode(second_node_on_edge))
              CutElemMeshError("Previous edge does not contain either of the nodes in this edge")
          }
          new_frag->boundary_edges.push_back(new edge_t(embedded_node2, second_node_on_edge));
        }
        else
          new_frag->boundary_edges.push_back(new edge_t(*boundary_edges[iedge]));

        ++iedge;
        if (iedge == boundary_edges.size())
          iedge = 0;
      }
      while(!boundary_edges[iedge]->containsEdge(*new_frag->boundary_edges[0]));

      if (cut_edges.size() > 1)
      { //set the starting point for the loop over the other part of the element
        iedge = cut_edges[0]+1;
        if (iedge == boundary_edges.size())
          iedge = 0;
      }

      new_fragments.push_back(new_frag);
    }
    while(new_fragments.size() < cut_edges.size());
  }

  return new_fragments;
}

std::vector<CutElemMesh::node_t*>
CutElemMesh::fragment_t::commonNodesWithEdge(edge_t & other_edge)
{
  std::vector<node_t*> common_nodes;
  for (unsigned int i = 0; i < 2; ++i)
  {
    node_t* edge_node = other_edge.get_node(i);
    if (containsNode(edge_node))
      common_nodes.push_back(edge_node);
  }
  return common_nodes;
}

CutElemMesh::element_t::element_t(unsigned int eid):
  id(eid),
  num_nodes(4),
  num_edges(4),
  nodes(num_nodes,NULL),
  edges(num_edges,NULL),
  parent(NULL),
  edge_neighbors(num_edges,std::vector<element_t*>(1,NULL)),
  crack_tip_split_element(false)
{}

CutElemMesh::element_t::~element_t()
{
  for (unsigned int i=0; i<fragments.size(); ++i)
  {
    if (fragments[i])
    {
      delete fragments[i];
      fragments[i] = NULL;
    }
  }
  for (unsigned int i=0; i<edges.size(); ++i)
  {
    if (edges[i])
    {
      delete edges[i];
      edges[i] = NULL;
    }
  }
}

void
CutElemMesh::element_t::createEdges()
{
  for (unsigned int i = 0; i < num_nodes; ++i)
  {
    unsigned int i_plus1(i < (num_nodes-1) ? i+1 : 0);
    edge_t * new_edge = new edge_t(nodes[i], nodes[i_plus1]);
    edges[i] = new_edge;
  }
}

void
CutElemMesh::element_t::switchNode(node_t *new_node,
                                   node_t *old_node,
                                   bool descend_to_parent)
{
  for (unsigned int i=0; i<num_nodes; ++i)
  {
    if (nodes[i] == old_node)
    {
      nodes[i] = new_node;
    }
  }
  for (unsigned int i=0; i<fragments.size(); ++i)
    fragments[i]->switchNode(new_node, old_node);

  for (unsigned int i=0; i<edges.size(); ++i)
    edges[i]->switchNode(new_node, old_node);

  if (parent && descend_to_parent)
  {
    parent->switchNode(new_node,old_node,false);
    for (unsigned int i=0; i<parent->edge_neighbors.size(); ++i)
    {
      for (unsigned int j=0; j<parent->edge_neighbors[i].size(); ++j)
      {
        if (parent->edge_neighbors[i][j])
        {
          element_t* edge_neighbor = parent->edge_neighbors[i][j];
          for (unsigned int k=0; k<edge_neighbor->children.size(); ++k)
          {
            edge_neighbor->children[k]->switchNode(new_node,old_node,false);
          }
        }
      }
    }
  }
}

void
CutElemMesh::element_t::switchEmbeddedNode(node_t *new_node,
                                           node_t *old_node)
{
  for (unsigned int i=0; i<num_edges; ++i)
  {
    if (edges[i]->get_embedded_node() == old_node)
    {
      edges[i]->replace_embedded_node(new_node);
    }
  }
  for (unsigned int i=0; i<fragments.size(); ++i)
    fragments[i]->switchNode(new_node, old_node);
}

bool
CutElemMesh::element_t::is_partial()
{
  bool partial = false;
  if (fragments.size() != 1)
    CutElemMeshError("is_partial() can only operate on elements that have 1 fragment")

  for (unsigned int i=0; i<num_nodes; i++)
  {
    if (!fragments[0]->containsNode(nodes[i]))
    {
      partial = true;
      break;
    }
  }

  return partial;
}

bool
CutElemMesh::element_t::overlays_elem(element_t* other_elem)
{
  bool overlays = false;
  //Find common nodes
  std::set<node_t*> e1nodes(nodes.begin(), nodes.end());
  std::set<node_t*> e2nodes(other_elem->nodes.begin(), other_elem->nodes.end());
  std::vector<node_t*> common_nodes;
  std::set_intersection(e1nodes.begin(), e1nodes.end(),
                        e2nodes.begin(), e2nodes.end(),
                        std::inserter(common_nodes,common_nodes.end()));

  //Find indices of common nodes
  if (common_nodes.size() == 2)
  {
    std::vector< node_t* > common_nodes_vec(common_nodes.begin(),common_nodes.end());

    unsigned int e1n1idx = num_nodes+1;
    unsigned int e1n2idx = num_nodes+1;
    for (unsigned int i=0; i<num_nodes; ++i)
    {
      if (nodes[i] == common_nodes_vec[0])
      {
        e1n1idx = i;
      }
      else if (nodes[i] == common_nodes_vec[1])
      {
        e1n2idx = i;
      }
    }
    if (e1n1idx > num_nodes || e1n2idx > num_nodes)
      CutElemMeshError("in overlays_elem() couldn't find common node")

    bool e1ascend = false;
    unsigned int e1n1idx_plus1(e1n1idx<(num_nodes-1) ? e1n1idx+1 : 0);
    unsigned int e1n1idx_minus1(e1n1idx>0 ? e1n1idx-1 : num_nodes-1);
    if (e1n2idx == e1n1idx_plus1)
    {
      e1ascend = true;
    }
    else
    {
      if (e1n2idx != e1n1idx_minus1)
        CutElemMeshError("in overlays_elem() common nodes must be adjacent to each other")
    }

    unsigned int e2n1idx = other_elem->num_nodes+1;
    unsigned int e2n2idx = other_elem->num_nodes+1;
    for (unsigned int i=0; i<other_elem->num_nodes; ++i)
    {
      if (other_elem->nodes[i] == common_nodes_vec[0])
      {
        e2n1idx = i;
      }
      else if (other_elem->nodes[i] == common_nodes_vec[1])
      {
        e2n2idx = i;
      }
    }
    if (e2n1idx > other_elem->num_nodes || e2n2idx > other_elem->num_nodes)
      CutElemMeshError("in overlays_elem() couldn't find common node")

    bool e2ascend = false;
    unsigned int e2n1idx_plus1(e2n1idx<(num_nodes-1) ? e2n1idx+1 : 0);
    unsigned int e2n1idx_minus1(e2n1idx>0 ? e2n1idx-1 : num_nodes-1);
    if (e2n2idx == e2n1idx_plus1)
    {
      e2ascend = true;
    }
    else
    {
      if (e2n2idx != e2n1idx_minus1)
        CutElemMeshError("in overlays_elem() common nodes must be adjacent to each other")
    }

    //if indices both ascend or descend, they overlay
    if ((e1ascend && e2ascend) ||
        (!e1ascend && !e2ascend))
    {
      overlays = true;
    }
  }
  else if (common_nodes.size() > 2)
  {
    //TODO: We probably need more error checking here.
    overlays = true;
  }
  return overlays;
}

bool
CutElemMesh::element_t::overlays_elem(node_t* other_edge_node1, node_t* other_edge_node2)
{
  bool overlays = false;
  bool found_common_edge = false;
  for (unsigned int edge_iter = 0; edge_iter < num_edges; ++edge_iter)
  {
    if (nodes[edge_iter] == other_edge_node1)
    {
      unsigned int edge_iter_plus1(edge_iter<(num_edges-1) ? edge_iter+1 : 0);
      if (nodes[edge_iter_plus1] == other_edge_node2)
      {
        overlays = true;
      }
      else
      {
        unsigned int edge_iter_minus1(edge_iter>0 ? edge_iter-1 : num_edges-1);
        if (nodes[edge_iter_minus1] != other_edge_node2)
          CutElemMeshError("in overlays_elem() element does not have an edge that contains the provided nodes")
      }
      found_common_edge = true;
      break;
    }
  }
  if (!found_common_edge)
    CutElemMeshError("in overlays_elem() element does not have an edge that contains the provided nodes")

  return overlays;
}

unsigned int
CutElemMesh::element_t::get_neighbor_index(element_t * neighbor_elem)
{
  for (unsigned int i=0; i<num_edges; ++i)
  {
    for (unsigned int j=0; j<edge_neighbors[i].size(); ++j)
    {
      if (edge_neighbors[i][j] == neighbor_elem)
      {
        return i;
      }
    }
  }
  CutElemMeshError("in get_neighbor_index() element: "<<id<<" does not have neighbor: "<<neighbor_elem->id)
}

void
CutElemMesh::element_t::add_crack_tip_neighbor(element_t * neighbor_elem)
{
  unsigned int neighbor_index = get_neighbor_index(neighbor_elem);
  for (unsigned int i=0; i<crack_tip_neighbors.size(); ++i)
  {
    if (crack_tip_neighbors[i] == neighbor_index)
    {
      CutElemMeshError("in add_crack_tip_neighbor() element: "<<id
                       <<" already has a crack tip neighbor set on side: "<<neighbor_index)
    }
  }
  crack_tip_neighbors.push_back(neighbor_index);
  if (crack_tip_neighbors.size() > 2)
  {
    for (unsigned int j=0; j<crack_tip_neighbors.size(); ++j)
    {
      std::cout<<"Neighbor: "<<crack_tip_neighbors[j]<<std::endl;
    }
    CutElemMeshError("in add_crack_tip_neighbor() element: "
                     <<id<<" cannot have more than 2 crack tip neighbors")
  }
}

bool
CutElemMesh::element_t::will_crack_tip_extend(std::vector<unsigned int> &split_neighbors)
{
  bool will_extend = false;
  if (fragments.size() == 1)
  {
    if (crack_tip_split_element)
    {
      for (unsigned int i=0; i<crack_tip_neighbors.size(); ++i)
      {
        unsigned int neigh_idx = crack_tip_neighbors[i];
        if (edge_neighbors[neigh_idx].size() != 1)
        {
          CutElemMeshError("in will_crack_tip_extend() element: "<<id<<" has: "
                           <<edge_neighbors[neigh_idx].size()<<" on edge: "<<neigh_idx)
        }
        element_t * neighbor_elem = edge_neighbors[neigh_idx][0];
        unsigned int num_cuts_in_neighbor = 0;
        for (unsigned int j=0; j<neighbor_elem->num_edges; ++j)
        {
          if (neighbor_elem->edges[j]->has_intersection())
          {
            ++num_cuts_in_neighbor;
          }
        }
        if (num_cuts_in_neighbor > 2)
        {
          CutElemMeshError("in will_crack_tip_extend() element: "<<neighbor_elem->id<<" has: "
                           <<num_cuts_in_neighbor<<" cut edges")
        }
        else if (num_cuts_in_neighbor == 2)
        {
          will_extend = true;
          split_neighbors.push_back(neigh_idx);
        }
      }
    }
  }
  return will_extend;
}

void
CutElemMesh::element_t::get_nodes_on_edge(unsigned int edge_idx,
                                          std::vector<node_t*> &edge_nodes)
{
  edge_nodes.push_back(edges[edge_idx]->get_node(0));
  edge_nodes.push_back(edges[edge_idx]->get_node(1));
}

void
CutElemMesh::element_t::get_non_physical_nodes(std::set<node_t*> &non_physical_nodes)
{
  //Any nodes that don't belong to any fragment are non-physical

  //First add all nodes in the element to the set
  for (unsigned int i=0; i<nodes.size(); ++i)
  {
    non_physical_nodes.insert(nodes[i]);
  }

  //Now delete any nodes that are contained in fragments
  std::set<node_t*>::iterator sit;
  for (sit=non_physical_nodes.begin(); sit != non_physical_nodes.end(); ++sit)
  {
    for (unsigned int i=0; i<fragments.size(); ++i)
    {
      if (fragments[i]->containsNode(*sit))
      {
        non_physical_nodes.erase(sit);
        break;
      }
    }
  }
}

bool
CutElemMesh::element_t::should_duplicate_for_crack_tip()
{
  bool should_duplicate = false;
  if (fragments.size() == 1)
  {
    std::vector<unsigned int> split_neighbors;
    if (will_crack_tip_extend(split_neighbors))
    {
      should_duplicate = true;
    }
    else
    {
      //The element may not be at the crack tip, but could have a non-physical node
      //connected to a crack tip face (on a neighbor element) that will be split.  We need to 
      //duplicate in that case as well.

      //Get the set of nodes in neighboring elements that are on a crack tip face that will be split
      std::set<node_t*> crack_tip_face_nodes;
      for (unsigned int i=0; i<edge_neighbors.size(); ++i)
      {
        for (unsigned int j=0; j<edge_neighbors[i].size(); ++j)
        {
          if (edge_neighbors[i][j])
          {
            std::vector<unsigned int> neighbor_split_neighbors;
            if (edge_neighbors[i][j]->will_crack_tip_extend(neighbor_split_neighbors))
            {
              for (unsigned int k=0; k<neighbor_split_neighbors.size(); ++k)
              {
                //Get the nodes on the crack tip edge
                std::vector<node_t*> edge_nodes;
                edge_neighbors[i][j]->get_nodes_on_edge(neighbor_split_neighbors[k],edge_nodes);
                for (unsigned int l=0; l<edge_nodes.size(); ++l)
                {
                  crack_tip_face_nodes.insert(edge_nodes[l]);
                }
              }
            }
          }
        }
      }

      //See if any of those nodes are in the non-physical part of this element.

      //Create a set of all non-physical elements
      std::set<node_t*> non_physical_nodes;
      get_non_physical_nodes(non_physical_nodes);

      std::vector<node_t*> common_nodes;
      std::set_intersection(crack_tip_face_nodes.begin(), crack_tip_face_nodes.end(),
                            non_physical_nodes.begin(), non_physical_nodes.end(),
                            std::inserter(common_nodes,common_nodes.end()));
      if (common_nodes.size() > 0)
      {
        should_duplicate = true;
      }
    }
  }
  return should_duplicate;
}

CutElemMesh::node_t *
CutElemMesh::element_t::create_local_node_from_global_node(const node_t * global_node)
{
  if (global_node->category != N_CATEGORY_PERMANENT &&
      global_node->category != N_CATEGORY_TEMP)
    CutElemMeshError("In create_local_node_from_global_node node is not global")

  node_t * new_local_node = NULL;
  unsigned int inode = 0;
  for (; inode < nodes.size(); ++inode)
  {
    if (nodes[inode] == global_node)
    {
      new_local_node = new node_t(inode, N_CATEGORY_LOCAL_INDEX);
      break;
    }
  }
  if (!new_local_node)
    CutElemMeshError("In create_local_node_from_global_node could not find global node")

  return new_local_node;
}

CutElemMesh::node_t *
CutElemMesh::element_t::get_global_node_from_local_node(const node_t * local_node)
{
  if (local_node->category != N_CATEGORY_LOCAL_INDEX)
    CutElemMeshError("In get_global_node_from_local_node node passed in is not local")

  node_t * global_node = nodes[local_node->id];

  if (global_node->category != N_CATEGORY_PERMANENT &&
      global_node->category != N_CATEGORY_TEMP)
    CutElemMeshError("In get_global_node_from_local_node, the node stored by the element is not global")

  return global_node;
}

CutElemMesh::CutElemMesh()
{}

CutElemMesh::~CutElemMesh()
{
  std::map<unsigned int, node_t*>::iterator mit;
  for (mit = PermanentNodes.begin(); mit != PermanentNodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  for (mit = EmbeddedNodes.begin(); mit != EmbeddedNodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  for (mit = TempNodes.begin(); mit != TempNodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  std::map<unsigned int, element_t*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit )
  {
    delete eit->second;
    eit->second = NULL;
  }
}

template <typename T> bool
CutElemMesh::deleteFromMap(std::map<unsigned int, T*> &theMap, T* elemToDelete)
{
  bool didIt = false;
  typename std::map<unsigned int, T*>::iterator i=theMap.find(elemToDelete->id);
  if (i != theMap.end())
  {
    delete i->second;
    theMap.erase(i);
    elemToDelete=NULL;
    didIt = true;
  }
  return didIt;
}

template <typename T> unsigned int
CutElemMesh::getNewID(std::map<unsigned int, T*> &theMap)
{
  typename std::map<unsigned int, T*>::reverse_iterator last_elem = theMap.rbegin();
  unsigned int new_elem_id = 0;
  if (last_elem != theMap.rend())
    new_elem_id=last_elem->first+1;
  return new_elem_id;
}

unsigned int CutElemMesh::addElements( std::vector< std::vector<unsigned int> > &quads )
{
  unsigned int first_id = 0;
  unsigned int num_nodes = 4;

  if (quads.size() == 0)
    CutElemMeshError("addElements called with empty vector of quads")

  for(unsigned int i = 0; i < quads.size(); ++i) {
    unsigned int new_elem_id = getNewID(Elements);
    element_t* newElem = new element_t(new_elem_id);
    Elements.insert(std::make_pair(new_elem_id,newElem));

    if (i == 0)
      first_id = new_elem_id;

    for (unsigned int j=0; j != num_nodes; j++) {
      node_t * currNode = NULL;
      std::map<unsigned int, node_t*>::iterator mit = PermanentNodes.find(quads[i][j]);
      if (mit == PermanentNodes.end()) {
        currNode = new node_t(quads[i][j],N_CATEGORY_PERMANENT);
        PermanentNodes.insert(std::make_pair(quads[i][j],currNode));
      } else {
        currNode = mit->second;
      }
      newElem->nodes[j] = currNode;
      InverseConnectivityMap[currNode].insert(newElem);
    }
    newElem->createEdges();
  }
  return first_id;
}

CutElemMesh::element_t* CutElemMesh::addElement( std::vector<unsigned int> quad, unsigned int id )
{
  unsigned int num_nodes = 4;

  std::map<unsigned int, element_t*>::iterator mit = Elements.find(id);
  if (mit != Elements.end())
    CutElemMeshError("In addElement element with id: "<<id<<" already exists")

  element_t* newElem = new element_t(id);
  Elements.insert(std::make_pair(id,newElem));

  for (unsigned int j=0; j != num_nodes; j++) {
    node_t * currNode = NULL;
    std::map<unsigned int, node_t*>::iterator mit = PermanentNodes.find(quad[j]);
    if (mit == PermanentNodes.end()) {
      currNode = new node_t(quad[j],N_CATEGORY_PERMANENT);
      PermanentNodes.insert(std::make_pair(quad[j],currNode));
    } else {
      currNode = mit->second;
    }
    newElem->nodes[j] = currNode;
    InverseConnectivityMap[currNode].insert(newElem);
  }
  newElem->createEdges();
  return newElem;
}

void CutElemMesh::updateEdgeNeighbors()
{
  std::map<unsigned int, element_t*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    element_t* elem = eit->second;
    for (unsigned int edge_iter = 0; edge_iter < elem->num_edges; ++edge_iter)
    {
      elem->edge_neighbors[edge_iter] = std::vector<element_t*>(1,NULL);
    }
  }
  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    element_t* curr_elem = eit->second;
    std::vector<node_t*> nodes;

    std::set<element_t*> neighbor_elements;
    for (unsigned int inode=0; inode<curr_elem->num_nodes; ++inode)
    {
      std::set<element_t*> this_node_connected_elems = InverseConnectivityMap[curr_elem->nodes[inode]];
      neighbor_elements.insert(this_node_connected_elems.begin(), this_node_connected_elems.end());
    }

    std::set<element_t*>::iterator eit2;
    for (eit2 = neighbor_elements.begin(); eit2 != neighbor_elements.end(); ++eit2)
    {
      if (*eit2 != curr_elem)
      {
        element_t *neigh_elem = *eit2;
        std::vector<node_t*> common_nodes;

        std::set<node_t*> curr_elem_nodes;
        for (unsigned int k=0; k < curr_elem->nodes.size(); k++)
          curr_elem_nodes.insert( curr_elem->nodes[k] );

        std::set<node_t*> neigh_elem_nodes;
        for (unsigned int k=0; k < neigh_elem->nodes.size(); k++)
          neigh_elem_nodes.insert( neigh_elem->nodes[k] );

        std::set_intersection(curr_elem_nodes.begin(), curr_elem_nodes.end(),
                              neigh_elem_nodes.begin(), neigh_elem_nodes.end(),
                              std::inserter(common_nodes,common_nodes.end()));
        if (common_nodes.size() >= 2)
        {
          bool found_edge = false;
          for (unsigned int edge_iter = 0; edge_iter < curr_elem->num_edges; ++edge_iter)
          {
            std::set<node_t*> edge_nodes;
            node_t* edge_node1 = curr_elem->edges[edge_iter]->get_node(0);
            node_t* edge_node2 = curr_elem->edges[edge_iter]->get_node(1);
            edge_nodes.insert(edge_node1);
            edge_nodes.insert(edge_node2);

            std::vector<node_t*> common_nodes_this_edge;
            std::set_intersection(edge_nodes.begin(), edge_nodes.end(),
                                  common_nodes.begin(), common_nodes.end(),
                                  std::inserter(common_nodes_this_edge,common_nodes_this_edge.end()));

            bool is_edge_neighbor = false;

            //Must share nodes on this edge
            if (common_nodes_this_edge.size() == 2)
            {
              //Must not overlay
              if (!neigh_elem->overlays_elem(edge_node1,edge_node2))
              {
                //Fragments must match up.
                if ((curr_elem->fragments.size()>1) ||
                    (neigh_elem->fragments.size()>1))
                {
                  CutElemMeshError("in updateEdgeNeighbors: Cannot have more than 1 fragment")
                }
                else if ((curr_elem->fragments.size()==1) &&
                         (neigh_elem->fragments.size()==1))
                {
                  if (curr_elem->fragments[0]->isConnected(*neigh_elem->fragments[0]))
                    is_edge_neighbor = true;
                }
                else //If there are no fragments to match up, consider them neighbors
                     //BWS TODO: This would occur at crack tip -- maybe we want to save that fragment after all
                {
                  is_edge_neighbor = true;
                }
              }
            }

            if (is_edge_neighbor)
            {
              if (curr_elem->edge_neighbors[edge_iter][0])
              {
                if (curr_elem->edge_neighbors[edge_iter].size() > 1)
                {
                  std::cout<<"Neighbor: "<<neigh_elem->id<<std::endl;
                  CutElemMeshError("Element "<<curr_elem->id<<" already has 2 edge neighbors: "
                                   <<curr_elem->edge_neighbors[edge_iter][0]->id<<" "
                                   <<curr_elem->edge_neighbors[edge_iter][1]->id)
                }
                curr_elem->edge_neighbors[edge_iter].push_back(neigh_elem);
              }
              else
              {
                curr_elem->edge_neighbors[edge_iter][0] = neigh_elem;
              }
              found_edge = true;
            }
          }
//BWS TODO: clean this error checking up
//          if (!found_edge)
//          {
//            printMesh();
//            std::cout<<"BWS curr: "<<curr_elem->id<<" neigh: "<<neigh_elem->id<<std::endl;
//            CutElemMeshError("Could not find edge for common nodes in element " << curr_elem->id);
//          }
        }
      }
    }
  }

  //Sanity check:
  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    element_t *curr_elem = eit->second;
    for (unsigned int edge_iter = 0; edge_iter < curr_elem->num_edges; ++edge_iter)
    {
      for (unsigned int en_iter = 0; en_iter < curr_elem->edge_neighbors[edge_iter].size(); ++en_iter)
      {
        element_t* neigh_elem = curr_elem->edge_neighbors[edge_iter][en_iter];
        if (neigh_elem != NULL)
        {
          bool found_neighbor = false;
          for (unsigned int edge_iter2 = 0; edge_iter2 < neigh_elem->num_edges; ++edge_iter2)
          {
            for (unsigned int en_iter2 = 0; en_iter2 < neigh_elem->edge_neighbors[edge_iter2].size(); ++en_iter2)
            {
              if (neigh_elem->edge_neighbors[edge_iter2][en_iter2] == curr_elem)
              {
                if ((en_iter2 > 1) && (en_iter > 1))
                {
                  CutElemMeshError("Element and neighbor element cannot both have >1 neighbors on a common edge")
                }
                found_neighbor = true;
                break;
              }
            }
          }
          if (!found_neighbor)
          {
            CutElemMeshError("Neighbor element doesn't recognize current element as neighbor")
          }
        }
      }
    }
  }
}

void CutElemMesh::initCrackTipTopology()
{
  CrackTipElements.clear();
  std::map<unsigned int, element_t*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    element_t *curr_elem = eit->second;
    unsigned int num_edges = curr_elem->num_edges;
    for (unsigned int edge_iter = 0; edge_iter < num_edges; ++edge_iter)
    {
      std::vector<element_t*> &edge_neighbors = curr_elem->edge_neighbors[edge_iter];
      if ((edge_neighbors.size() == 2) &&
          (curr_elem->edges[edge_iter]->has_intersection()))
      {
        //Neither neighbor overlays current element.  We are on the uncut element ahead of the tip.
        //Flag neighbors as crack tip elements and add this element as their crack tip neighbor.

        node_t* edge_node1 = curr_elem->edges[edge_iter]->get_node(0);
        node_t* edge_node2 = curr_elem->edges[edge_iter]->get_node(1);

        if ((edge_neighbors[0]->overlays_elem(edge_node1,edge_node2)) ||
            (edge_neighbors[1]->overlays_elem(edge_node1,edge_node2)))
        {
          CutElemMeshError("Element has a neighbor that overlays itself")
        }

        //Make sure the current elment hasn't been flagged as a tip element
        if (curr_elem->crack_tip_split_element)
        {
          CutElemMeshError("crack_tip_split_element already flagged.  In elem: "<<curr_elem->id
                           << " flags: "<<curr_elem->crack_tip_split_element
                           <<" "<<edge_neighbors[0]->crack_tip_split_element
                           <<" "<<edge_neighbors[1]->crack_tip_split_element)
        }

        CrackTipElements.insert(curr_elem);

        edge_neighbors[0]->crack_tip_split_element = true;
        edge_neighbors[1]->crack_tip_split_element = true;

        edge_neighbors[0]->add_crack_tip_neighbor(curr_elem);
        edge_neighbors[1]->add_crack_tip_neighbor(curr_elem);
      }
    }
  }
}

void CutElemMesh::addEdgeIntersection( unsigned int elemid, unsigned int edgeid, double position )
{
  std::map<unsigned int, element_t*>::iterator eit = Elements.find(elemid);
  if (eit == Elements.end())
    CutElemMeshError("Could not find element with id: "<<elemid<<" in addEdgeIntersection")

  element_t *curr_elem = eit->second;
  addEdgeIntersection( curr_elem, edgeid, position );
}

void CutElemMesh::addEdgeIntersection( element_t * elem, unsigned int edgeid, double position, node_t * embedded_node)
{

  node_t* local_embedded_node = NULL;
  node_t* first_node_on_edge = elem->nodes[edgeid];
  if (embedded_node)
  {
    //use the existing embedded node if it was passed in
    local_embedded_node = embedded_node;
  }

  if (elem->edges[edgeid]->has_intersection())
  {
    if (!elem->edges[edgeid]->has_intersection_at_position(position,first_node_on_edge) ||
        (embedded_node && elem->edges[edgeid]->get_embedded_node() != embedded_node))
    {
      CutElemMeshError("Attempting to add edge intersection when one already exists with different position or node."
                       << " elem: "<<elem->id<<" edge: "<<edgeid<<" position: "<<position<<" old position: "<<elem->edges[edgeid]->get_intersection(first_node_on_edge))
    }
    local_embedded_node = elem->edges[edgeid]->get_embedded_node();
  }
  else
  {
    if (!local_embedded_node)
    {
      //create the embedded node
      unsigned int new_node_id = getNewID(EmbeddedNodes);
      local_embedded_node = new node_t(new_node_id,N_CATEGORY_EMBEDDED);
      EmbeddedNodes.insert(std::make_pair(new_node_id,local_embedded_node));
    }
    elem->edges[edgeid]->add_intersection(position, local_embedded_node, first_node_on_edge);
  }

  for (unsigned int en_iter = 0; en_iter < elem->edge_neighbors[edgeid].size(); ++en_iter)
  {
    element_t *edge_neighbor = elem->edge_neighbors[edgeid][en_iter];
    if (edge_neighbor)
    {
      unsigned int num_edges_neigh = edge_neighbor->num_edges;
      bool found = false;
      for (unsigned int j=0; j<num_edges_neigh; j++)
      {
        for (unsigned int k=0; k<edge_neighbor->edge_neighbors[j].size(); k++)
        {
          if (edge_neighbor->edge_neighbors[j][k] == elem)
          {
            found = true;

            if (edge_neighbor->edges[j]->has_intersection())
            {
              if (!edge_neighbor->edges[j]->equivalent(*elem->edges[edgeid]))
              {
                CutElemMeshError("Attempting to add edge intersection when neighbor already has one that is incompatible."
                                 << " elem: "<<elem->id<<" edge: "<<edgeid
                                 <<" neighbor: "<<edge_neighbor->id<<" neighbor edge: "<<j)
              }
            }
            else
              edge_neighbor->edges[j]->add_intersection(position, local_embedded_node, first_node_on_edge);
            break;
          }
        }
      }
      if (!found)
      {
        CutElemMeshError("Neighbor Element " << edge_neighbor->id
                         << " on edge " << edgeid << " of Element " << elem->id << "isn't set ")
      }
    }
  }
}

void CutElemMesh::updatePhysicalLinksAndFragments()
{

  //loop over the elements in the mesh
  std::map<unsigned int, element_t*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    element_t *curr_elem = eit->second;
    unsigned int num_edges = curr_elem->num_edges;
    unsigned int num_cut_edges = 0;
    for (unsigned int iedge = 0; iedge < num_edges; ++iedge)
      if(curr_elem->edges[iedge]->has_intersection())
        ++num_cut_edges;

    if (num_cut_edges == 0)
      continue; // skip this element if no cut edge
    if (num_cut_edges > 2)
      CutElemMeshError("In element "<<curr_elem->id<<" more than 2 cut edges")

    if (curr_elem->fragments.size() > 0)
    {
      //Element has already been cut. Don't recreate fragments because we
      //would create multiple fragments to cover the entire element and
      //lose the information about what part of this element is physical.
      if (curr_elem->is_partial())
        continue; // skip to next element
      else//Clear the links and recreate to allow for a whole element to be split.
        curr_elem->fragments.clear();
    }

    if (curr_elem->fragments.size() == 0) // create a frag that covers the whole elem
        curr_elem->fragments.push_back(new fragment_t(curr_elem, true));

    if (curr_elem->fragments.size() != 1)
      CutElemMeshError("Element "<<curr_elem->id<<" must have 1 fragment at this point")

    std::vector<fragment_t *> new_frags = curr_elem->fragments[0]->split();

    if (new_frags.size() == 1 || new_frags.size() == 2)
    {
      delete curr_elem->fragments[0]; // delete the old fragment
      curr_elem->fragments.clear();
      for (unsigned int i = 0; i < new_frags.size(); ++i)
        curr_elem->fragments.push_back(new_frags[i]);
    }
    else
      CutElemMeshError("Number of fragments must be 1 or 2 at this point")

    physicalLinkAndFragmentSanityCheck(curr_elem);   
  }
}

void CutElemMesh::physicalLinkAndFragmentSanityCheck(element_t *currElem)
{
  unsigned int num_edges = currElem->num_edges;
  std::vector<unsigned int> cut_edges;
  for (unsigned int iedge=0; iedge<num_edges; ++iedge)
  {
    if(currElem->edges[iedge]->has_intersection())
    {
      cut_edges.push_back(iedge);
    }
  }
  if (cut_edges.size() > 2)
    CutElemMeshError("In element "<<currElem->id<<" more than 2 cut edges")

  std::vector<unsigned int> num_emb;
  std::vector<unsigned int> num_perm;
  for (unsigned int i=0; i<currElem->fragments.size(); ++i)
  {
    num_emb.push_back(0);
    num_perm.push_back(0);
    std::set<node_t*> perm_nodes;
    std::set<node_t*> emb_nodes;
    for (unsigned int j=0; j<currElem->fragments[i]->boundary_edges.size(); ++j)
    {
      for (unsigned int k = 0; k < 2; ++k)
      {
        node_t * temp_node = currElem->fragments[i]->boundary_edges[j]->get_node(k);
        if (temp_node->category == N_CATEGORY_PERMANENT)
          perm_nodes.insert(temp_node);
        else if (temp_node->category == N_CATEGORY_EMBEDDED)
          emb_nodes.insert(temp_node);
        else
          CutElemMeshError("Invalid node category")
      }
    }
    num_perm[i] = perm_nodes.size();
    num_emb[i] = emb_nodes.size();
  }

  if (cut_edges.size() == 0)
  {
    if (currElem->fragments.size() != 1 ||
        currElem->fragments[0]->boundary_edges.size() != num_edges)
    {
      CutElemMeshError("Incorrect link size for element with 0 cuts")
    }
    if (num_emb[0] != 0 || num_perm[0] != num_edges)
    {
      CutElemMeshError("Incorrect node category for element with 0 cuts")
    }
  }
  else if (cut_edges.size() == 1)
  {
    if (currElem->fragments.size() != 1 ||
        currElem->fragments[0]->boundary_edges.size() != num_edges+1)
    {
      CutElemMeshError("Incorrect link size for element with 1 cut")
    }
    if (num_emb[0] != 1 || num_perm[0] != num_edges)
    {
      CutElemMeshError("Incorrect node category for element with 1 cut")
    }
  }
  else if (cut_edges.size() == 2)
  {
    if (currElem->fragments.size() != 2 ||
        (currElem->fragments[0]->boundary_edges.size()+currElem->fragments[1]->boundary_edges.size()) != num_edges+4)
    {
      CutElemMeshError("Incorrect link size for element with 2 cuts")
    }
    if (num_emb[0] != 2 || num_emb[1] != 2 || (num_perm[0]+num_perm[1]) != num_edges)
    {
      CutElemMeshError("Incorrect node category for element with 2 cuts")
    }
  }
}

void CutElemMesh::updateTopology(bool mergeUncutVirtualEdges)
{
  // If mergeUncutVirtualEdges=true, this algorithm replicates the
  // behavior of classical XFEM.  If false, it gives the behavior of
  // the Richardson et. al. (2011) paper

  NewNodes.clear();
  ChildElements.clear();
  ParentElements.clear();
  MergedEdgeMap.clear();

  unsigned int first_new_node_id = getNewID(PermanentNodes);

  createChildElements();
  connectFragments(mergeUncutVirtualEdges);
  sanityCheck();
  findCrackTipElements();

  std::map<unsigned int, node_t*>::iterator mit;
  for (mit = PermanentNodes.begin(); mit != PermanentNodes.end(); ++mit )
  {
    if (mit->first >= first_new_node_id)
    {
      NewNodes.push_back(mit->second);
    }
  }
}

void CutElemMesh::reset()
{
  NewNodes.clear();
  ChildElements.clear();
  ParentElements.clear();
  MergedEdgeMap.clear();
  CrackTipElements.clear();
  InverseConnectivityMap.clear();

  std::map<unsigned int, node_t*>::iterator mit;
  for (mit = PermanentNodes.begin(); mit != PermanentNodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  PermanentNodes.clear();
//  for (mit = EmbeddedNodes.begin(); mit != EmbeddedNodes.end(); ++mit )
//  {
//    delete mit->second;
//    mit->second = NULL;
//  }
  for (mit = TempNodes.begin(); mit != TempNodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  TempNodes.clear();
  std::map<unsigned int, element_t*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit )
  {
    delete eit->second;
    eit->second = NULL;
  }
  Elements.clear();
}

void CutElemMesh::clearAncestry()
{
  InverseConnectivityMap.clear();
  for (unsigned int i=0; i<ParentElements.size(); ++i)
  {
    if (!deleteFromMap(Elements, ParentElements[i]))
    {
      CutElemMeshError("Attempted to delete parent element: "<<ParentElements[i]->id
                       <<" from Elements, but couldn't find it")
    }
  }
  ParentElements.clear();

  std::map<unsigned int, element_t*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit )
  {
    element_t *curr_elem = eit->second;
    curr_elem->parent=NULL;
    curr_elem->children.clear();
    for (unsigned int j=0; j != curr_elem->num_nodes; j++)
    {
      node_t *curr_node = curr_elem->nodes[j];
      InverseConnectivityMap[curr_node].insert(curr_elem);
    }
  }

  for (unsigned int i=0; i<PermanentNodes.size(); ++i)
  {
    PermanentNodes[i]->parent=NULL;
  }

  std::map<unsigned int, node_t*>::iterator mit;
  for (mit = TempNodes.begin(); mit != TempNodes.end(); ++mit )
  {
    delete mit->second;
    mit->second = NULL;
  }
  TempNodes.clear();

  NewNodes.clear();
  ChildElements.clear();

  //TODO: Sanity check to make sure that there are no nodes that are not connected
  //      to an element -- there shouldn't be any
}

void CutElemMesh::restoreFragmentInfo(CutElemMesh::element_t * const elem,
                                      fragment_t & from_frag)
{
  fragment_t * new_fragment = new fragment_t(from_frag, elem, false);
  if (elem->fragments.size() != 0)
    CutElemMeshError("in restoreFragmentInfo elements must not have any pre-existing fragments")
  elem->fragments.push_back(new_fragment);
}

void CutElemMesh::restoreEdgeIntersections(CutElemMesh::element_t * const elem,
                                           const std::vector<bool> &local_edge_has_intersection,
                                           const std::vector<CutElemMesh::node_t*> &embedded_nodes_on_edge,
                                           const std::vector<double> &intersection_x)
{
  unsigned int num_edges = elem->num_edges;
  for (unsigned int i=0; i<num_edges; ++i)
  {
    if (local_edge_has_intersection[i])
    {
      addEdgeIntersection(elem, i, intersection_x[i], embedded_nodes_on_edge[i]);
    }
  }
}

void CutElemMesh::createChildElements()
{
  //temporary container for new elements -- will be merged with Elements
  std::map<unsigned int, element_t*> newChildElements;

  //loop over the original elements in the mesh
  std::map<unsigned int, element_t*>::iterator eit;
  std::map<unsigned int, element_t*>::iterator ElementsEnd = Elements.end();
  for (eit = Elements.begin(); eit != ElementsEnd; ++eit)
  {
    element_t *curr_elem = eit->second;
    if (curr_elem->children.size() != 0)
      CutElemMeshError("Element cannot have existing children in createChildElements")

    unsigned int num_edges = curr_elem->num_edges;
    unsigned int num_nodes = curr_elem->num_nodes;
    unsigned int num_links = curr_elem->fragments.size();
    if (num_links > 1 || curr_elem->should_duplicate_for_crack_tip())
    {
      if (num_links > 2)
        CutElemMeshError("More than 2 fragments not yet supported")

      //set up the children
      ParentElements.push_back(curr_elem);
      for (unsigned int ichild=0; ichild<num_links; ++ichild)
      {
        unsigned int new_elem_id;
        if (newChildElements.size() == 0)
        {
          new_elem_id = getNewID(Elements);
        }
        else
        {
          new_elem_id = getNewID(newChildElements);
        }
        element_t* childElem = new element_t(new_elem_id);
        newChildElements.insert(std::make_pair(new_elem_id,childElem));

        ChildElements.push_back(childElem);
        childElem->parent = curr_elem;
        curr_elem->children.push_back(childElem);
        for (unsigned int j=0; j<num_nodes; j++) // get child nodes
        {
          bool node_in_fragment = false;
          for (unsigned int k=0; k<curr_elem->fragments[ichild]->boundary_edges.size(); ++k)
          {
            if (curr_elem->fragments[ichild]->boundary_edges[k]->containsNode(curr_elem->nodes[j]))
            {
              node_in_fragment = true;
              break;
            }
          }
          if (node_in_fragment)
          {
            childElem->nodes[j] = curr_elem->nodes[j]; // inherit parent's node
          }
          else // parent element's node is not in fragment
          {
            unsigned int new_node_id = getNewID(TempNodes);
            node_t* newNode = new node_t(new_node_id,N_CATEGORY_TEMP,curr_elem->nodes[j]);
            TempNodes.insert(std::make_pair(new_node_id,newNode));
            childElem->nodes[j] = newNode; // be a temp node
          }
        }

        for (unsigned int j=0; j<num_edges; j++) // get child edges
        {
          unsigned int jplus1(j < (num_edges-1) ? j+1 : 0);
          edge_t * new_edge = new edge_t(childElem->nodes[j], childElem->nodes[jplus1]);
          if (curr_elem->edges[j]->has_intersection())
          {
            double child_position = curr_elem->edges[j]->get_intersection(curr_elem->nodes[j]);
            node_t * child_embedded_node = curr_elem->edges[j]->get_embedded_node();
            new_edge->add_intersection(child_position, child_embedded_node, childElem->nodes[j]);
          }
          childElem->edges[j] = new_edge;
        }

        fragment_t * new_frag = new fragment_t(childElem, true, curr_elem, ichild); // HERE may be problematic
        childElem->fragments.push_back(new_frag); // get child fragment
      }
    }
    else //num_links == 1
    {
      //child is itself - but don't insert into the list of ChildElements!!!
      curr_elem->children.push_back(curr_elem);
    }
  }
  //Merge newChildElements back in with Elements
  Elements.insert(newChildElements.begin(),newChildElements.end());
}

void CutElemMesh::connectFragments(bool mergeUncutVirtualEdges)
{
  //now perform the comparison on the children
  for (unsigned int elem_iter = 0; elem_iter < ChildElements.size(); elem_iter++)
  {
    element_t *childElem = ChildElements[elem_iter];
    element_t *parentElem = childElem->parent;
    unsigned int num_edges = childElem->num_edges;

    //Gather the neighbor elements and edge ids of the common edges for those neighbors
    std::vector<std::vector<element_t *> > NeighborElem;
    std::vector<std::vector<unsigned int> > neighbor_common_edge;
    for (unsigned int j=0; j<num_edges; ++j)
    {
      //loop over the children for the neighbor on this edge
      unsigned int num_edge_neighbors=parentElem->edge_neighbors[j].size();
      NeighborElem.push_back(std::vector<element_t*> (num_edge_neighbors,NULL));
      neighbor_common_edge.push_back(std::vector<unsigned int> (num_edge_neighbors,99999)); //big number to indicate edge not found
      for (unsigned int k=0; k<num_edge_neighbors; ++k)
      {
        if (parentElem->edge_neighbors[j][k])
        {
          NeighborElem[j][k] = parentElem->edge_neighbors[j][k];
          //identify the common cut edge
          bool found(false);
          unsigned int neighbor_num_edges = NeighborElem[j][k]->num_edges;
          for (unsigned int m=0; m<neighbor_num_edges; ++m)
          {
            for (unsigned int n=0; n<NeighborElem[j][k]->edge_neighbors[m].size(); ++n)
            {
              if (NeighborElem[j][k]->edge_neighbors[m][n] == parentElem)
              {
                neighbor_common_edge[j][k] = m;
                found = true;
                break;
              }
            }
            if (found) break;
          }
          if (!found)
          {
            CutElemMeshError("Parent element: "<<parentElem->id
                             <<" is not a neighbor of its neighbor element: " << NeighborElem[j][k]->id)
          }
        }
      }
    }

    //First loop through edges and merge nodes with neighbors as appropriate
    for (unsigned int j=0; j<num_edges; ++j)
    {
      for (unsigned int k=0; k<NeighborElem[j].size(); ++k)
      {
        if (NeighborElem[j][k])
        {
          //get nodes on this edge of childElem
          std::vector<node_t*> childEdgeNodes;
          childEdgeNodes.push_back(childElem->edges[j]->get_node(0));
          childEdgeNodes.push_back(childElem->edges[j]->get_node(1));

          if (childElem->edges[j]->has_intersection())
          {
            for (unsigned int l=0; l < NeighborElem[j][k]->children.size(); l++)
            {
              element_t *childOfNeighborElem = NeighborElem[j][k]->children[l];

              //get nodes on this edge of childOfNeighborElem
              std::vector<node_t*> childOfNeighborEdgeNodes;
              childOfNeighborEdgeNodes.push_back(childOfNeighborElem->edges[neighbor_common_edge[j][k]]->get_node(0));
              childOfNeighborEdgeNodes.push_back(childOfNeighborElem->edges[neighbor_common_edge[j][k]]->get_node(1));

              //Check to see if the nodes are already merged.  There's nothing else to do in that case.
              if (childEdgeNodes[0] == childOfNeighborEdgeNodes[1] &&
                  childEdgeNodes[1] == childOfNeighborEdgeNodes[0])
              {
                addToMergedEdgeMap(childEdgeNodes[0], childEdgeNodes[1], childElem, childOfNeighborElem);
                continue;
              }

              if (childElem->fragments[0]->isConnected(*childOfNeighborElem->fragments[0]))
              {
                std::vector<node_t*> merged_nodes;
                unsigned int num_edge_nodes = 2;
                for (unsigned int i=0; i<num_edge_nodes; ++i)
                {
                  unsigned int childNodeIndex = i;
                  unsigned int neighborChildNodeIndex = num_edge_nodes - 1 - childNodeIndex;

                  node_t* childNode = childEdgeNodes[childNodeIndex];
                  node_t* childOfNeighborNode = childOfNeighborEdgeNodes[neighborChildNodeIndex];

                  mergeNodes(childNode,childOfNeighborNode,childElem,childOfNeighborElem);
                  merged_nodes.push_back(childNode);
                }
                addToMergedEdgeMap(merged_nodes[0], merged_nodes[1], childElem, childOfNeighborElem);

                duplicateEmbeddedNode(childElem,childOfNeighborElem,j,neighbor_common_edge[j][k]);
              }
            }
          }
          else //No edge intersection -- optionally merge non-material nodes if they share a common parent
          {
            if (mergeUncutVirtualEdges && NeighborElem[j][k])
            {
              for (unsigned int l=0; l < NeighborElem[j][k]->children.size(); l++)
              {
                element_t *childOfNeighborElem = NeighborElem[j][k]->children[l];

                //get nodes on this edge of childOfNeighborElem
                std::vector<node_t*> childOfNeighborEdgeNodes;
                childOfNeighborEdgeNodes.push_back(childOfNeighborElem->edges[neighbor_common_edge[j][k]]->get_node(0));
                childOfNeighborEdgeNodes.push_back(childOfNeighborElem->edges[neighbor_common_edge[j][k]]->get_node(1));

                //Check to see if the nodes are already merged.  There's nothing else to do in that case.
                if (childEdgeNodes[0] == childOfNeighborEdgeNodes[1] &&
                    childEdgeNodes[1] == childOfNeighborEdgeNodes[0])
                  continue;

                unsigned int num_edge_nodes = 2;
                for (unsigned int i=0; i<num_edge_nodes; ++i)
                {
                  unsigned int childNodeIndex = i;
                  unsigned int neighborChildNodeIndex = num_edge_nodes - 1 - childNodeIndex;

                  node_t* childNode = childEdgeNodes[childNodeIndex];
                  node_t* childOfNeighborNode = childOfNeighborEdgeNodes[neighborChildNodeIndex];

                  if (childNode->parent != NULL &&
                      childNode->parent == childOfNeighborNode->parent) //non-material node and both come from same parent
                  {
                    mergeNodes(childNode,childOfNeighborNode,childElem,childOfNeighborElem);
                  }
                }
              }
            }
          }
        }
      }
    }

    //Now do a second loop through edges and convert remaining nodes to permanent nodes.
    //If there is no neighbor on that edge, also duplicate the embedded node if it exists
    {
      for (unsigned int j=0; j<num_edges; j++)
      {
        node_t* childNode = childElem->nodes[j];

        if(childNode->category == N_CATEGORY_TEMP)
        {
          unsigned int new_node_id = getNewID(PermanentNodes);
          node_t* newNode = new node_t(new_node_id,N_CATEGORY_PERMANENT,childNode->parent);
          PermanentNodes.insert(std::make_pair(new_node_id,newNode));

          childElem->switchNode(newNode, childNode);
          if (!deleteFromMap(TempNodes, childNode))
          {
            CutElemMeshError("Attempted to delete node: "<<childNode->id
                             <<" from TempNodes, but couldn't find it")
          }
        }
        if (NeighborElem[j].size() == 1 &&
            !NeighborElem[j][0]) //No neighbor of parent on this edge -- free edge -- need to convert to permanent nodes
        {
          duplicateEmbeddedNode(childElem,j);
        }
      }
    }
  }

  std::vector<element_t*>::iterator vit;
  for (vit=ChildElements.begin(); vit!=ChildElements.end(); )
  {
    if (*vit==NULL)
    {
      vit=ChildElements.erase(vit);
    }
    else
    {
      ++vit;
    }
  }
}

void CutElemMesh::mergeNodes(node_t*  &childNode,
                             node_t* &childOfNeighborNode,
                             element_t* childElem,
                             element_t* childOfNeighborElem)
{
  if (childNode != childOfNeighborNode)
  {
    if(childNode->category == N_CATEGORY_PERMANENT)
    {
      if(childOfNeighborNode->category == N_CATEGORY_PERMANENT)
      {
        if (childOfNeighborNode->parent == childNode)
        {
          childOfNeighborElem->switchNode(childNode, childOfNeighborNode);
          if (!deleteFromMap(PermanentNodes, childOfNeighborNode))
          {
            CutElemMeshError("Attempted to delete node: "<<childOfNeighborNode->id
                             <<" from PermanentNodes, but couldn't find it")
          }
          childOfNeighborNode = childNode;
        }
        else if (childNode->parent == childOfNeighborNode)
        {
          childElem->switchNode(childOfNeighborNode, childNode);
          if (!deleteFromMap(PermanentNodes, childNode))
          {
            CutElemMeshError("Attempted to delete node: "<<childNode->id
                             <<" from PermanentNodes, but couldn't find it")
          }
          childNode = childOfNeighborNode;
        }
        else
        {
          CutElemMeshError("Attempting to merge nodes: "<<childNode->id<<" and "
                           <<childOfNeighborNode->id<<" but both are permanent")
        }
      }
      else
      {
        if (childOfNeighborNode->parent != childNode &&
            childOfNeighborNode->parent != childNode->parent)
        {
          CutElemMeshError("Attempting to merge nodes "<<childOfNeighborNode->id_cat_str()<<" and "
                           <<childNode->id_cat_str()<<" but neither the 2nd node nor its parent is parent of the 1st")
        }
        childOfNeighborElem->switchNode(childNode, childOfNeighborNode);
        if (!deleteFromMap(TempNodes, childOfNeighborNode))
        {
          CutElemMeshError("Attempted to delete node: "<<childOfNeighborNode->id
                           <<" from TempNodes, but couldn't find it")
        }
        childOfNeighborNode = childNode;
      }
    }
    else if(childOfNeighborNode->category == N_CATEGORY_PERMANENT)
    {
      if (childNode->parent != childOfNeighborNode &&
          childNode->parent != childOfNeighborNode->parent)
      {
        CutElemMeshError("Attempting to merge nodes "<<childNode->id<<" and "
                         <<childOfNeighborNode->id<<" but neither the 2nd node nor its parent is parent of the 1st")
      }
      childElem->switchNode(childOfNeighborNode, childNode);
      if (!deleteFromMap(TempNodes, childNode))
      {
        CutElemMeshError("Attempted to delete node: "<<childNode->id<<" from TempNodes, but couldn't find it")
      }
      childNode = childOfNeighborNode;
    }
    else //both nodes are temporary -- create new permanent node and delete temporary nodes
    {
      unsigned int new_node_id = getNewID(PermanentNodes);
      node_t* newNode = new node_t(new_node_id,N_CATEGORY_PERMANENT,childNode->parent);
      PermanentNodes.insert(std::make_pair(new_node_id,newNode));

      childOfNeighborElem->switchNode(newNode, childOfNeighborNode);
      childElem->switchNode(newNode, childNode);

      if (childNode->parent != childOfNeighborNode->parent)
      {
        CutElemMeshError("Attempting to merge nodes "<<childNode->id<<" and "
                         <<childOfNeighborNode->id<<" but they don't share a common parent")
      }

      if (!deleteFromMap(TempNodes, childOfNeighborNode))
      {
        CutElemMeshError("Attempted to delete node: "<<childOfNeighborNode->id
                         <<" from TempNodes, but couldn't find it")
      }
      if (!deleteFromMap(TempNodes, childNode))
      {
        CutElemMeshError("Attempted to delete node: "<<childNode->id
                         <<" from TempNodes, but couldn't find it")
      }
      childOfNeighborNode = newNode;
      childNode = newNode;
    }
  }
}

void CutElemMesh::addToMergedEdgeMap(node_t* node1,
                                     node_t* node2,
                                     element_t* elem1,
                                     element_t* elem2)
{
  std::set<node_t*> edge_nodes;
  edge_nodes.insert(node1);
  edge_nodes.insert(node2);
  MergedEdgeMap[edge_nodes].insert(elem1);
  MergedEdgeMap[edge_nodes].insert(elem2);
}

//This version is for cases where there is a neighbor element
void CutElemMesh::duplicateEmbeddedNode(element_t* currElem,
                                        element_t* neighborElem,
                                        unsigned int edgeID,
                                        unsigned int neighborEdgeID)
{
  node_t* embeddedNode = currElem->edges[edgeID]->get_embedded_node();
  node_t* neighEmbeddedNode = neighborElem->edges[neighborEdgeID]->get_embedded_node();

  if (embeddedNode != neighEmbeddedNode)
    CutElemMeshError("Embedded nodes on merged edge must match")

  //Check to see whether the embedded node can be duplicated for both the current element
  //and the neighbor element.

  bool current_split = false;   //Is the current element being split on this edge?
  bool current_tip = false;     //Is the current element a crack tip element with a split on this edge?
  bool neighbor_split = false;  //Is the neighbor element being split on this edge?
  bool neighbor_tip = false;    //Is the neighbor element a crack tip element with a split on this edge?

  if (currElem->parent &&
      currElem->parent->children.size() >= 1)
  {
    if (currElem->parent->children.size() > 1)
    {
      bool hasSiblingWithSameEmbeddedNode = false;
      for (unsigned int i=0; i<currElem->parent->children.size(); ++i)
      {
        if (currElem->parent->children[i] != currElem &&
            currElem->parent->children[i]->edges[edgeID]->get_embedded_node() == embeddedNode)
        {
          hasSiblingWithSameEmbeddedNode = true;
          break;
        }
      }
      if (hasSiblingWithSameEmbeddedNode)
        current_split = true;
    }
    else if (currElem->parent->crack_tip_split_element)
    {
      for (unsigned int i=0; i<currElem->parent->crack_tip_neighbors.size(); ++i)
      {
        if (currElem->parent->crack_tip_neighbors[i] == edgeID)
        {
          current_tip = true;
          break;
        }
      }
    }
  }

  if (neighborElem->parent &&
      neighborElem->parent->children.size() >= 1)
  {
    if (neighborElem->parent->children.size() > 1)
    {
      bool hasSiblingWithSameEmbeddedNode = false;
      for (unsigned int i=0; i<neighborElem->parent->children.size(); ++i)
      {
        if (neighborElem->parent->children[i] != neighborElem &&
            neighborElem->parent->children[i]->edges[neighborEdgeID]->get_embedded_node() == embeddedNode)
        {
          hasSiblingWithSameEmbeddedNode = true;
          break;
        }
      }
      if (hasSiblingWithSameEmbeddedNode)
        neighbor_split = true;
    }
    else if (neighborElem->parent->crack_tip_split_element)
    {
      for (unsigned int i=0; i<neighborElem->parent->crack_tip_neighbors.size(); ++i)
      {
        if (neighborElem->parent->crack_tip_neighbors[i] == neighborEdgeID)
        {
          neighbor_tip = true;
          break;
        }
      }
    }
  }

  bool can_dup = false;
  // Don't duplicate if current and neighbor are both crack tip elems
  // because that embedded node has already been duplicated
  if ((current_split && neighbor_split) ||
      (current_split && neighbor_tip) ||
      (neighbor_split && current_tip))
  {
    can_dup = true;
  }

  if (can_dup)
  {
    // Determine whether the split occurs on the current embedded node.
    // This would happen if the link set contains the embedded node
    // and only one of the nodes on the edge.
    std::vector<node_t*> currCommonNodes = currElem->fragments[0]->commonNodesWithEdge(*currElem->edges[edgeID]);
    std::vector<node_t*> neighCommonNodes = neighborElem->fragments[0]->commonNodesWithEdge(*currElem->edges[edgeID]);   

    if (currElem->fragments[0]->containsNode(embeddedNode) &&
        neighborElem->fragments[0]->containsNode(embeddedNode) &&
        currCommonNodes.size() == 1 && neighCommonNodes.size() == 1)
    {
      // Duplicate this embedded node
      unsigned int new_node_id = getNewID(EmbeddedNodes);
      node_t* newNode = new node_t(new_node_id,N_CATEGORY_EMBEDDED);
      EmbeddedNodes.insert(std::make_pair(new_node_id,newNode));

      currElem->switchEmbeddedNode(newNode, embeddedNode);
      neighborElem->switchEmbeddedNode(newNode, embeddedNode);
    }
  }
}

//This version is for cases when there is no neighbor
void CutElemMesh::duplicateEmbeddedNode(element_t* currElem,
                                        unsigned int edgeID)
{
  node_t* embeddedNode = currElem->edges[edgeID]->get_embedded_node();

  // Do elements on both side of the common edge have siblings?
  if (currElem->parent &&
      currElem->parent->children.size() > 1)
  {
    // Determine whether any of the sibling child elements have the same
    // embedded node.  Only duplicate if that is the case.

    bool currElemHasSiblingWithSameEmbeddedNode = false;
    for (unsigned int i=0; i<currElem->parent->children.size(); ++i)
    {
      if (currElem->parent->children[i] != currElem &&
          currElem->parent->children[i]->edges[edgeID]->get_embedded_node() == embeddedNode)
      {
        currElemHasSiblingWithSameEmbeddedNode = true;
        break;
      }
    }

    if (currElemHasSiblingWithSameEmbeddedNode)
    {
      // Determine whether the split occurs on the current embedded node.
      // This would happen if the link set contains the embedded node
      // and only one of the nodes on the edge.

      std::vector<node_t*> currCommonNodes = currElem->fragments[0]->commonNodesWithEdge(*currElem->edges[edgeID]);
      
      if (currCommonNodes.size() == 1 &&
          currElem->fragments[0]->containsNode(embeddedNode))
      {
        // Duplicate this embedded node
        unsigned int new_node_id = getNewID(EmbeddedNodes);
        node_t* newNode = new node_t(new_node_id,N_CATEGORY_EMBEDDED);
        EmbeddedNodes.insert(std::make_pair(new_node_id,newNode));

        currElem->switchEmbeddedNode(newNode, embeddedNode);
      }
    }
  }
}

void CutElemMesh::sanityCheck()
{
  //Make sure there are no remaining TempNodes
  if (TempNodes.size()>0)
  {
    std::cout<<"TempNodes size > 0.  size="<<TempNodes.size()<<std::endl;
    printMesh();
    exit(1);
  }
}

void CutElemMesh::findCrackTipElements()
{
  std::set<element_t*>::iterator sit;
  //Delete all elements that were previously flagged as crack tip elements if they have
  //been split (and hence appear in ParentElements).
  for (unsigned int i=0; i<ParentElements.size(); ++i)
  {
    sit = CrackTipElements.find(ParentElements[i]);
    if (sit != CrackTipElements.end())
    {
      CrackTipElements.erase(sit);
    }
  }

  //Debug: print MergedEdgeMap
  std::map<std::set<node_t*>, std::set<element_t*> >::iterator memit;
  //std::cout<<"MergedEdgeMap:"<<std::endl;
  //for (memit = MergedEdgeMap.begin(); memit != MergedEdgeMap.end(); ++memit)
  //{
  //  std::cout<<"Elems: ";
  //  std::set<element_t*> conn_elems = memit->second;
  //  std::set<element_t*>::iterator setit;
  //  for (setit = conn_elems.begin(); setit != conn_elems.end(); ++setit)
  //  {
  //    std::cout<<(*setit)->id<<" ";
  //  }
  //  std::cout<<std::endl;
  //}

  //Go through MergedEdgeMap to find elements that are newly at the crack tip due to
  //crack growth.
  for (memit = MergedEdgeMap.begin(); memit != MergedEdgeMap.end(); ++memit)
  {
    if (memit->second.size() < 2)
    {
      CutElemMeshError("in findCrackTipElements() cannot have <2 elements on common edge")
    }
    else if (memit->second.size() == 2)
    {
    }
    else if (memit->second.size() == 3)
    {
      std::vector< element_t* > this_tip_elems(memit->second.begin(),memit->second.end());
      bool olay01 = this_tip_elems[0]->overlays_elem(this_tip_elems[1]);
      bool olay12 = this_tip_elems[1]->overlays_elem(this_tip_elems[2]);
      bool olay20 = this_tip_elems[2]->overlays_elem(this_tip_elems[0]);

      if (olay01)
      {
        if (olay12 || olay20)
        {
          CutElemMeshError("in findCrackTipElements() only 2 of elements on tip edge can overlay")
        }
        CrackTipElements.insert(this_tip_elems[2]);
      }
      else if (olay12)
      {
        if (olay01 || olay20)
        {
          CutElemMeshError("in findCrackTipElements() only 2 of elements on tip edge can overlay")
        }
        CrackTipElements.insert(this_tip_elems[0]);
      }
      else if (olay20)
      {
        if (olay01 || olay12)
        {
          CutElemMeshError("in findCrackTipElements() only 2 of elements on tip edge can overlay")
        }
        CrackTipElements.insert(this_tip_elems[1]);
      }
    }
  }
  //std::cout<<"Crack tip elements: ";
  //for (sit=CrackTipElements.begin(); sit!=CrackTipElements.end(); ++sit)
  //{
  //  std::cout<<(*sit)->id<<" ";
  //}
  //std::cout<<std::endl;
}

void CutElemMesh::printMesh()
{
  std::cout<<"============================================================"
           <<"=================================================="<<std::endl;
  std::cout<<"                                            CutElemMesh Data"<<std::endl;
  std::cout<<"============================================================"
           <<"=================================================="<<std::endl;
  std::cout << "Permanent Nodes:" << std::endl;
  std::map<unsigned int, node_t*>::iterator mit;
  for (mit = PermanentNodes.begin(); mit != PermanentNodes.end(); ++mit )
    std::cout << "  " << mit->second->id << std::endl;
  std::cout << "Temp Nodes:" << std::endl;
  for (mit = TempNodes.begin(); mit != TempNodes.end(); ++mit )
    std::cout << "  " << mit->second->id << std::endl;
  std::cout << "Embedded Nodes:" << std::endl;
  for (mit = EmbeddedNodes.begin(); mit != EmbeddedNodes.end(); ++mit )
    std::cout << "  " << mit->second->id << std::endl;
  std::cout << "Parent Elements:" << std::endl;
  for (unsigned int i=0; i<ParentElements.size(); ++i)
    std::cout << " " << ParentElements[i]->id << std::endl;
  std::cout << "Child Elements:" << std::endl;
  for (unsigned int i=0; i<ChildElements.size(); ++i)
    std::cout << " " << ChildElements[i]->id << std::endl;
  std::cout << "Elements:" << std::endl;
  std::cout << "  id "
            << "|  nodes                "
            << "|  embedded nodes       "
            << "|  edge neighbors       "
            << "|  frag "
            << "|  frag link      ...   "
            << std::endl;
  std::cout<<"------------------------------------------------------------"
           <<"--------------------------------------------------"<<std::endl;
  std::map<unsigned int, element_t*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit )
  {
    element_t* currElem = eit->second;
    std::cout << std::setw(4);
    std::cout << currElem->id << " | ";
    for (unsigned int j=0; j<currElem->num_nodes; j++)
    {
      std::cout << std::setw(5) << currElem->nodes[j]->id_cat_str();
    }

    std::cout << "  | ";
    for (unsigned int j=0; j<currElem->num_edges; j++)
    {
      std::cout<<std::setw(4);
      if (currElem->edges[j]->has_intersection())
        std::cout << currElem->edges[j]->get_embedded_node()->id << " ";
      else
        std::cout << "  -- ";
    }
    std::cout << "  | ";
    for (unsigned int j=0; j<currElem->num_edges; j++)
    {
      if (currElem->edge_neighbors[j].size() > 1)
      {
        std::cout << "[";
        for (unsigned int k=0; k<currElem->edge_neighbors[j].size(); ++k)
        {
          std::cout << currElem->edge_neighbors[j][k]->id;
          if (k == currElem->edge_neighbors[j].size()-1)
            std::cout<<"]";
          else
            std::cout<<" ";
        }
        std::cout<< " ";
      }
      else
      {
        std::cout<<std::setw(4);
        if (currElem->edge_neighbors[j][0])
          std::cout << currElem->edge_neighbors[j][0]->id << " ";
        else
          std::cout << "  -- ";
      }
    }
    std::cout << "  | ";
    for (unsigned int j=0; j!= currElem->fragments.size(); j++)
    {
      std::cout<<std::setw(4);
      std::cout << " " << j << " | ";
      for (unsigned int k=0; k < currElem->fragments[j]->boundary_edges.size(); k++)
      {
        node_t* prt_node = currElem->fragments[j]->boundary_edges[k]->get_node(0);
        unsigned int kprev(k>0 ? k-1 : currElem->fragments[j]->boundary_edges.size()-1);
        if (!currElem->fragments[j]->boundary_edges[kprev]->containsNode(prt_node))
          prt_node = currElem->fragments[j]->boundary_edges[k]->get_node(1);
        std::cout << std::setw(4) << prt_node->id_cat_str();
      }
    }
    std::cout << std::endl;
  }
}

CutElemMesh::element_t* CutElemMesh::getElemByID(unsigned int id)
{
  std::map<unsigned int, CutElemMesh::element_t*>::iterator mit = Elements.find(id);
  if (mit == Elements.end())
    CutElemMeshError("in getElemByID() could not find element: "<<id)
  return mit->second;
}
