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
  for (unsigned int i=0; i<interior_links.size(); ++i)
  {
    for (unsigned int j=0; j<interior_links[i].size(); ++j)
    {
      if (interior_links[i][j] == old_node)
      {
        interior_links[i][j] = new_node;
      }
    }
  }
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
    if (embedded_nodes_on_edge[i] == old_node)
    {
      embedded_nodes_on_edge[i] = new_node;
    }
  }
  for (unsigned int i=0; i<interior_links.size(); ++i)
  {
    for (unsigned int j=0; j<interior_links[i].size(); ++j)
    {
      if (interior_links[i][j] == old_node)
      {
        interior_links[i][j] = new_node;
      }
    }
  }
}

bool
CutElemMesh::element_t::is_partial()
{
  bool partial = false;
  if (interior_links.size() != 1)
  {
    CutElemMeshError("is_partial() can only operate on elements that have 1 link");
  }

  for (unsigned int i=0; i<num_nodes; i++)
  {
    bool node_in_fragment = false;
    for (unsigned int j=0; j<interior_links[0].size(); ++j)
    {
      if (nodes[i] == interior_links[0][j])
      {
        node_in_fragment = true;
        break;
      }
    }
    if (!node_in_fragment)
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
    {
      CutElemMeshError("in overlays_elem() couldn't find common node");
    }
    bool e1ascend = true;
    if (e1n2idx < e1n1idx)
    {
      if (!(e1n2idx == 0 && e1n1idx == num_nodes-1))
      {
        e1ascend=false;
      }
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
    {
      CutElemMeshError("in overlays_elem() couldn't find common node");
    }
    bool e2ascend = true;
    if (e2n2idx < e2n1idx)
    {
      if (!(e2n2idx == 0 && e2n1idx == other_elem->num_nodes-1))
      {
        e2ascend=false;
      }
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
    CutElemMeshError("in overlays_elem() >2 common nodes");
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
        {
          CutElemMeshError("in overlays_elem() element does not have an edge that contains the provided nodes");
        }
      }
      found_common_edge = true;
      break;
    }
  }
  if (!found_common_edge)
  {
    CutElemMeshError("in overlays_elem() element does not have an edge that contains the provided nodes");
  }
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
  CutElemMeshError("in get_neighbor_index() element: "<<id<<" does not have neighbor: "<<neighbor_elem->id);
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
                       <<" already has a crack tip neighbor set on side: "<<neighbor_index);
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
                     <<id<<" cannot have more than 2 crack tip neighbors");
  }
}

bool
CutElemMesh::element_t::will_crack_tip_extend(std::vector<unsigned int> &split_neighbors)
{
  bool will_extend = false;
  if (interior_links.size() == 1)
  {
    if (crack_tip_split_element)
    {
      for (unsigned int i=0; i<crack_tip_neighbors.size(); ++i)
      {
        unsigned int neigh_idx = crack_tip_neighbors[i];
        if (edge_neighbors[neigh_idx].size() != 1)
        {
          CutElemMeshError("in will_crack_tip_extend() element: "<<id<<" has: "
                           <<edge_neighbors[neigh_idx].size()<<" on edge: "<<neigh_idx);
        }
        element_t * neighbor_elem = edge_neighbors[neigh_idx][0];
        unsigned int num_cuts_in_neighbor = 0;
        for (unsigned int j=0; j<neighbor_elem->num_edges; ++j)
        {
          if (neighbor_elem->local_edge_has_intersection[j])
          {
            ++num_cuts_in_neighbor;
          }
        }
        if (num_cuts_in_neighbor > 2)
        {
          CutElemMeshError("in will_crack_tip_extend() element: "<<neighbor_elem->id<<" has: "
                           <<num_cuts_in_neighbor<<" cut edges");
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
  unsigned int edge_idx_plus1(edge_idx<(num_edges-1) ? edge_idx+1 : 0);
  edge_nodes.push_back(nodes[edge_idx]);
  edge_nodes.push_back(nodes[edge_idx_plus1]);
}

void
CutElemMesh::element_t::get_non_physical_nodes(std::set<node_t*> &non_physical_nodes)
{
  //Any nodes that don't belong to any fragment are non-physical
  std::set<node_t*>fragment_nodes;

  for (unsigned int i=0; i<interior_links.size(); ++i)
  {
    for (unsigned int j=0; j<interior_links[i].size(); ++j)
    {
      fragment_nodes.insert(interior_links[i][j]);
    }
  }

  for (unsigned int i=0; i<nodes.size(); ++i)
  {
    if (fragment_nodes.find(nodes[i]) == fragment_nodes.end())
    {
      non_physical_nodes.insert(nodes[i]);
    }
  }
}

bool
CutElemMesh::element_t::should_duplicate_for_crack_tip()
{
  bool should_duplicate = false;
  if (interior_links.size() == 1)
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
  {
    CutElemMeshError("addElements called with empty vector of quads");
  }

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
    }
  }
  return first_id;
}

CutElemMesh::element_t* CutElemMesh::addElement( std::vector<unsigned int> quad, unsigned int id )
{
  unsigned int num_nodes = 4;

  std::map<unsigned int, element_t*>::iterator mit = Elements.find(id);
  if (mit != Elements.end()) {
    CutElemMeshError("In addElement element with id: "<<id<<" already exists");
  }

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
  }
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

    std::map<unsigned int, element_t*>::iterator eit2;
    for (eit2 = Elements.begin(); eit2 != Elements.end(); ++eit2)
    {
      if (eit2 != eit)
      {
        element_t *neigh_elem = eit2->second;
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
            unsigned int edge_iter_plus1(edge_iter<(curr_elem->num_edges-1) ? edge_iter+1 : 0);

            std::set<node_t*> edge_nodes;
            edge_nodes.insert(curr_elem->nodes[edge_iter]);
            edge_nodes.insert(curr_elem->nodes[edge_iter_plus1]);
            node_t* edge_node1 = curr_elem->nodes[edge_iter];
            node_t* edge_node2 = curr_elem->nodes[edge_iter_plus1];

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
                if ((curr_elem->interior_links.size()>1) ||
                    (neigh_elem->interior_links.size()>1))
                {
                  CutElemMeshError("in updateEdgeNeighbors: Cannot have more than 1 interior link")
                }
                else if ((curr_elem->interior_links.size()==1) &&
                         (neigh_elem->interior_links.size()==1))
                {
                  //Create a set of the link nodes in the current element
                  std::set<node_t*> curr_link_nodes;
                  for (unsigned int l=0; l < curr_elem->interior_links[0].size(); l++)
                    curr_link_nodes.insert( curr_elem->interior_links[0][l] );

                  //Create a set of the link nodes in the neighboring element
                  std::set<node_t*> neigh_link_nodes;
                  for (unsigned int n=0; n < neigh_elem->interior_links[0].size(); n++)
                    neigh_link_nodes.insert( neigh_elem->interior_links[0][n] );

                  //Compare the sets of link nodes to see if more than one are common
                  std::vector<node_t*> common_nodes;
                  std::set_intersection(curr_link_nodes.begin(), curr_link_nodes.end(),
                                        neigh_link_nodes.begin(), neigh_link_nodes.end(),
                                        std::inserter(common_nodes,common_nodes.end()));

                  if (common_nodes.size() > 1)
                  {
                    is_edge_neighbor = true;
                  }
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
                                   <<curr_elem->edge_neighbors[edge_iter][1]->id);
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
                  CutElemMeshError("Element and neighbor element cannot both have >1 neighbors on a common edge");
                }
                found_neighbor = true;
                break;
              }
            }
          }
          if (!found_neighbor)
          {
            CutElemMeshError("Neighbor element doesn't recognize current element as neighbor");
          }
        }
      }
    }
  }
}

void CutElemMesh::initCrackTipTopology()
{
  std::map<unsigned int, element_t*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit)
  {
    element_t *curr_elem = eit->second;
    unsigned int num_edges = curr_elem->num_edges;
    for (unsigned int edge_iter = 0; edge_iter < num_edges; ++edge_iter)
    {
      std::vector<element_t*> &edge_neighbors = curr_elem->edge_neighbors[edge_iter];
      if ((edge_neighbors.size() == 2) &&
          (curr_elem->local_edge_has_intersection[edge_iter]))
      {
        //Neither neighbor overlays current element.  We are on the uncut element ahead of the tip.
        //Flag neighbors as crack tip elements and add this element as their crack tip neighbor.

        unsigned int edge_iter_plus1(edge_iter<(num_edges-1) ? edge_iter+1 : 0);
        node_t* edge_node1 = curr_elem->nodes[edge_iter];
        node_t* edge_node2 = curr_elem->nodes[edge_iter_plus1];

        if ((edge_neighbors[0]->overlays_elem(edge_node1,edge_node2)) ||
            (edge_neighbors[1]->overlays_elem(edge_node1,edge_node2)))
        {
          CutElemMeshError("Element has a neighbor that overlays itself");
        }

        //Make sure the current elment hasn't been flagged as a tip element
        if (curr_elem->crack_tip_split_element)
        {
          CutElemMeshError("crack_tip_split_element already flagged.  In elem: "<<curr_elem->id
                           << " flags: "<<curr_elem->crack_tip_split_element
                           <<" "<<edge_neighbors[0]->crack_tip_split_element
                           <<" "<<edge_neighbors[1]->crack_tip_split_element);
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
  {
    CutElemMeshError("Could not find element with id: "<<elemid<<" in addEdgeIntersection");
  }
  element_t *curr_elem = eit->second;
  addEdgeIntersection( curr_elem, edgeid, position );
}

void CutElemMesh::addEdgeIntersection( element_t * elem, unsigned int edgeid, double position, node_t * embedded_node)
{

  node_t* local_embedded_node = NULL;
  if (embedded_node)
  {
    //use the existing embedded node if it was passed in
    local_embedded_node = embedded_node;
  }

  double tol = 1.e-4;

  if (elem->local_edge_has_intersection[edgeid])
  {
    if ((std::abs(elem->intersection_x[edgeid] - position) > tol)||
        (embedded_node && elem->embedded_nodes_on_edge[edgeid] != embedded_node))
    {
      CutElemMeshError("Attempting to add edge intersection when one already exists with different position or node."
                       << " elem: "<<elem->id<<" edge: "<<edgeid<<" position: "<<position<<" old position: "<<elem->intersection_x[edgeid]);
    }
    local_embedded_node = elem->embedded_nodes_on_edge[edgeid];
  }
  else
  {
    elem->local_edge_has_intersection[edgeid] = true;
    elem->intersection_x[edgeid] = position;

    if (!local_embedded_node)
    {
      //create the embedded node
      unsigned int new_node_id = getNewID(EmbeddedNodes);
      local_embedded_node = new node_t(new_node_id,N_CATEGORY_EMBEDDED);
      EmbeddedNodes.insert(std::make_pair(new_node_id,local_embedded_node));

    }
    //insert the embedded node on the edge
    elem->embedded_nodes_on_edge[edgeid] = local_embedded_node;
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

            unsigned int edgeid_plus1(edgeid<(elem->num_edges-1) ? edgeid+1 : 0);
            node_t* edge_node1 = elem->nodes[edgeid];
            node_t* edge_node2 = elem->nodes[edgeid_plus1];
            double neighbor_position;
            //BWSolay TODO: we shouldn't ever hit this case now
            if (edge_neighbor->overlays_elem(edge_node1,edge_node2))
            {
              printMesh();
              CutElemMeshError("in addEdgeIntersection: neighbor overlays current element.  "
                               <<"curr: "<<elem->id<<" neigh: "<<edge_neighbor->id);
              //neighbor_position = position;
            }
            else
            {
              neighbor_position = 1.0 - position;
            }

            if (edge_neighbor->local_edge_has_intersection[j])
            {
              if ((edge_neighbor->embedded_nodes_on_edge[j] != local_embedded_node) ||
                  (std::abs(edge_neighbor->intersection_x[j] - neighbor_position) > tol))
              {
//                printMesh();
//                std::cout<<"neigh node: "<<edge_neighbor->embedded_nodes_on_edge[j]->id<<std::endl;
//                std::cout<<"local node: "<<local_embedded_node->id<<std::endl;
//                std::cout<<"neigh int: "<<edge_neighbor->intersection_x[j]<<std::endl;
//                std::cout<<"local int: "<<neighbor_position<<std::endl;
                CutElemMeshError("Attempting to add edge intersection when neighbor already has one that is incompatible."
                                 << " elem: "<<elem->id<<" edge: "<<edgeid
                                 <<" neighbor: "<<edge_neighbor->id<<" neighbor edge: "<<j);
              }
            }
            edge_neighbor->local_edge_has_intersection[j] = true;
            edge_neighbor->intersection_x[j] = neighbor_position;
            edge_neighbor->embedded_nodes_on_edge[j] = local_embedded_node;
            break;
          }
        }
      }
      if (!found)
      {
        CutElemMeshError("Neighbor Element " << edge_neighbor->id
                         << " on edge " << edgeid << " of Element " << elem->id << "isn't set ");
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

    std::vector<unsigned int> cut_edges;
    for (unsigned int iedge=0; iedge<num_edges; ++iedge)
    {
      if(curr_elem->embedded_nodes_on_edge[iedge])
      {
        cut_edges.push_back(iedge);
      }
    }
    if (cut_edges.size() == 0)
    {
      continue;
    }
    if (cut_edges.size() > 2)
    {
      CutElemMeshError("In element "<<curr_elem->id<<" more than 2 cut edges");
    }

    if (curr_elem->interior_links.size() > 0)
    {
      if (curr_elem->is_partial())
      { //Element has already been cut. Don't recreate fragments because we
        //would create multiple fragments to cover the entire element and
        //lose the information about what part of this element is physical.
        continue;
      }
      else
      { //Clear the links and recreate to allow for a whole element to be split.
        curr_elem->interior_links.clear();
      }
    }

    unsigned int iedge=0;
    unsigned int icutedge=0;

    do //loop over link sets
    {
      std::vector<node_t*> link_nodes;

      do //loop over edges
      {
        link_nodes.push_back(curr_elem->nodes[iedge]);
        if (iedge == cut_edges[icutedge])
        {
          link_nodes.push_back(curr_elem->embedded_nodes_on_edge[iedge]);
          if (cut_edges.size() == 2)
          {
            ++icutedge;
            if (icutedge == cut_edges.size())
              icutedge = 0;
            iedge = cut_edges[icutedge];
            link_nodes.push_back(curr_elem->embedded_nodes_on_edge[iedge]);
          }
        }
        ++iedge;
        if (iedge == num_edges)
          iedge = 0;
      }
      while(link_nodes[0] != curr_elem->nodes[iedge]);

      if (cut_edges.size() > 1)
      { //set the starting point for the loop over the other part of the element
        iedge = cut_edges[0]+1;
        if (iedge == num_edges)
          iedge = 0;
      }

      curr_elem->interior_links.push_back(link_nodes);
    }
    while(curr_elem->interior_links.size() < cut_edges.size());

    physicalLinkAndFragmentSanityCheck(curr_elem);
  }
}

void CutElemMesh::physicalLinkAndFragmentSanityCheck(element_t *currElem)
{
  unsigned int num_edges = currElem->num_edges;
  std::vector<unsigned int> cut_edges;
  for (unsigned int iedge=0; iedge<num_edges; ++iedge)
  {
    if(currElem->embedded_nodes_on_edge[iedge])
    {
      cut_edges.push_back(iedge);
    }
  }
  if (cut_edges.size() > 2)
  {
    CutElemMeshError("In element "<<currElem->id<<" more than 2 cut edges");
  }

  std::vector<unsigned int> num_emb;
  std::vector<unsigned int> num_perm;
  for (unsigned int i=0; i<currElem->interior_links.size(); ++i)
  {
    num_emb.push_back(0);
    num_perm.push_back(0);
    for (unsigned int j=0; j<currElem->interior_links[i].size(); ++j)
    {
      if (currElem->interior_links[i][j]->category == N_CATEGORY_PERMANENT)
        ++num_perm[i];
      else if (currElem->interior_links[i][j]->category == N_CATEGORY_EMBEDDED)
        ++num_emb[i];
      else
      {
        CutElemMeshError("Invalid node category");
      }
    }
  }

  if (cut_edges.size() == 0)
  {
    if (currElem->interior_links.size() != 1 ||
        currElem->interior_links[0].size() != num_edges)
    {
      CutElemMeshError("Incorrect link size for element with 0 cuts");
    }
    if (num_emb[0] != 0 || num_perm[0] != num_edges)
    {
      CutElemMeshError("Incorrect node category for element with 0 cuts");
    }
  }
  else if (cut_edges.size() == 1)
  {
    if (currElem->interior_links.size() != 1 ||
        currElem->interior_links[0].size() != num_edges+1)
    {
      CutElemMeshError("Incorrect link size for element with 1 cut");
    }
    if (num_emb[0] != 1 || num_perm[0] != num_edges)
    {
      CutElemMeshError("Incorrect node category for element with 1 cut");
    }
  }
  else if (cut_edges.size() == 2)
  {
    if (currElem->interior_links.size() != 2 ||
        (currElem->interior_links[0].size()+currElem->interior_links[1].size()) != num_edges+4)
    {
      CutElemMeshError("Incorrect link size for element with 2 cuts");
    }
    if (num_emb[0] != 2 || num_emb[1] != 2 || (num_perm[0]+num_perm[1]) != num_edges)
    {
      CutElemMeshError("Incorrect node category for element with 2 cuts");
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
  CrackTipElements.clear();

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
  for (unsigned int i=0; i<ParentElements.size(); ++i)
  {
    if (!deleteFromMap(Elements, ParentElements[i]))
    {
      CutElemMeshError("Attempted to delete parent element: "<<ParentElements[i]->id
                       <<" from Elements, but couldn't find it");
    }
  }
  ParentElements.clear();

  std::map<unsigned int, element_t*>::iterator eit;
  for (eit = Elements.begin(); eit != Elements.end(); ++eit )
  {
    element_t *curr_elem = eit->second;
    curr_elem->parent=NULL;
    curr_elem->children.clear();
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
                                      const std::vector<std::pair<FRAG_NODE_CATEGORY, unsigned int> > &interior_link)
{
  if (elem->interior_links.size() != 0)
  {
    CutElemMeshError("In restore_fragment_info Elements must not have any interior links");
  }

  std::vector< node_t*> tmp_interior_link;

  for (unsigned int i=0; i<interior_link.size(); ++i)
  {
    if (interior_link[i].first == FRAG_NODE_EMBEDDED)
    {
      unsigned int embedded_node_id = interior_link[i].second;
      std::map<unsigned int, node_t*>::const_iterator mit;
      mit = EmbeddedNodes.find(embedded_node_id);
      if (mit == EmbeddedNodes.end())
      {
        CutElemMeshError("In restore_fragment_info could not find EmbeddedNode with id: "<<embedded_node_id);
      }
      node_t *node = mit->second;
      tmp_interior_link.push_back(node);
    }
    else if (interior_link[i].first == FRAG_NODE_LOCAL_INDEX)
    {
      unsigned int local_node_index = interior_link[i].second;
      if (local_node_index > elem->nodes.size())
      {
        CutElemMeshError("In restore_fragment_info node index out of bounds: "<<local_node_index);
      }
      node_t *node = elem->nodes[local_node_index];
      tmp_interior_link.push_back(node);
    }
  }
  elem->interior_links.push_back(tmp_interior_link);
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
    {
      CutElemMeshError("Element cannot have existing children in createChildElements");
    }
    unsigned int num_edges = curr_elem->num_edges;
    unsigned int num_nodes = curr_elem->num_nodes;
    unsigned int num_links = curr_elem->interior_links.size();
    if (num_links > 1 || curr_elem->should_duplicate_for_crack_tip())
    {
      if (num_links > 2)
      {
        CutElemMeshError("More than 2 fragments not yet supported");
      }
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
        for (unsigned int j=0; j<num_edges; j++)
        {
          childElem->local_edge_has_intersection[j] = curr_elem->local_edge_has_intersection[j];
          if (curr_elem->local_edge_has_intersection[j])
          {
            childElem->embedded_nodes_on_edge[j] = curr_elem->embedded_nodes_on_edge[j];
            childElem->intersection_x[j] = curr_elem->intersection_x[j];
          }
        }
        for (unsigned int j=0; j<num_nodes; j++)
        {
          bool node_in_fragment = false;
          for (unsigned int k=0; k<curr_elem->interior_links[ichild].size(); ++k)
          {
            if (curr_elem->nodes[j] == curr_elem->interior_links[ichild][k])
            {
              node_in_fragment = true;
              break;
            }
          }
          if (node_in_fragment)
          {
            childElem->nodes[j] = curr_elem->nodes[j];
          }
          else
          {
            unsigned int new_node_id = getNewID(TempNodes);
            node_t* newNode = new node_t(new_node_id,N_CATEGORY_TEMP,curr_elem->nodes[j]);
            TempNodes.insert(std::make_pair(new_node_id,newNode));
            childElem->nodes[j] = newNode;
          }
        }

        std::vector<node_t*> link_nodes;
        for (unsigned int j=0; j<curr_elem->interior_links[ichild].size(); ++j)
        {
          node_t * cur_link_node = curr_elem->interior_links[ichild][j];
          if (cur_link_node->category == N_CATEGORY_PERMANENT)
          {
            for (unsigned int k=0; k<num_nodes; ++k)
            {
              if ((cur_link_node == childElem->nodes[k]) ||
                  (cur_link_node == childElem->nodes[k]->parent)) //BWS -- why would the link node be in the parent elem?
              {
                link_nodes.push_back(childElem->nodes[k]);
                break;
              }
            }
            if (link_nodes.size() != j+1)
            {
              CutElemMeshError("Could not find link node in child elem nodes or their parents");
            }
          }
          else if (cur_link_node->category == N_CATEGORY_EMBEDDED)
          {
            link_nodes.push_back(cur_link_node);
          }
          else
          {
            CutElemMeshError("Invalid node category");
          }
        }
        childElem->interior_links.push_back(link_nodes);
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
                             <<" is not a neighbor of its neighbor element: " << NeighborElem[j][k]->id);
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
          unsigned int jplus1(j<(num_edges-1) ? j+1 : 0);
          std::vector<node_t*> childEdgeNodes;
          childEdgeNodes.push_back(childElem->nodes[j]);
          childEdgeNodes.push_back(childElem->nodes[jplus1]);

          if (childElem->local_edge_has_intersection[j])
          {
            //set up the sets according to the links
            std::set<node_t*> child_link_nodes;
            for (unsigned int l=0; l < childElem->interior_links[0].size(); l++)
              child_link_nodes.insert( childElem->interior_links[0][l] );

            for (unsigned int l=0; l < NeighborElem[j][k]->children.size(); l++)
            {
              element_t *childOfNeighborElem = NeighborElem[j][k]->children[l];

              //get nodes on this edge of childOfNeighborElem
              unsigned int nce_plus1(neighbor_common_edge[j][k]<(num_edges-1) ? neighbor_common_edge[j][k]+1 : 0);
              std::vector<node_t*> childOfNeighborEdgeNodes;
              childOfNeighborEdgeNodes.push_back(childOfNeighborElem->nodes[neighbor_common_edge[j][k]]);
              childOfNeighborEdgeNodes.push_back(childOfNeighborElem->nodes[nce_plus1]);

              //Check to see if the nodes are already merged.  There's nothing else to do in that case.
              if (childEdgeNodes[0] == childOfNeighborEdgeNodes[1] &&
                  childEdgeNodes[1] == childOfNeighborEdgeNodes[0])
              {
                //if (childElem->is_partial() || childOfNeighborElem->is_partial())
                {
                  addToMergedEdgeMap(childEdgeNodes[0], childEdgeNodes[1], childElem, childOfNeighborElem);
                }
                continue;
              }

              //construct this set
              std::set<node_t*> neigh_link_nodes;
              for (unsigned int n=0; n < childOfNeighborElem->interior_links[0].size(); n++)
                neigh_link_nodes.insert( childOfNeighborElem->interior_links[0][n] );

              //Compare the sets of nodes on the common edge to see if more than one are common
              //That indicates that they share material, and the nodes should be collapsed.
              std::vector<node_t*> common_nodes;
              std::set_intersection(child_link_nodes.begin(), child_link_nodes.end(),
                                    neigh_link_nodes.begin(), neigh_link_nodes.end(),
                                    std::inserter(common_nodes,common_nodes.end()));

              if (common_nodes.size() > 1)
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
                unsigned int nce_plus1(neighbor_common_edge[j][k]<(num_edges-1) ? neighbor_common_edge[j][k]+1 : 0);
                std::vector<node_t*> childOfNeighborEdgeNodes;
                childOfNeighborEdgeNodes.push_back(childOfNeighborElem->nodes[neighbor_common_edge[j][k]]);
                childOfNeighborEdgeNodes.push_back(childOfNeighborElem->nodes[nce_plus1]);

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
                             <<" from TempNodes, but couldn't find it");
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
                             <<" from PermanentNodes, but couldn't find it");
          }
          childOfNeighborNode = childNode;
        }
        else if (childNode->parent == childOfNeighborNode)
        {
          childElem->switchNode(childOfNeighborNode, childNode);
          if (!deleteFromMap(PermanentNodes, childNode))
          {
            CutElemMeshError("Attempted to delete node: "<<childNode->id
                             <<" from PermanentNodes, but couldn't find it");
          }
          childNode = childOfNeighborNode;
        }
        else
        {
          CutElemMeshError("Attempting to merge nodes: "<<childNode->id<<" and "
                           <<childOfNeighborNode->id<<" but both are permanent");
        }
      }
      else
      {
        if (childOfNeighborNode->parent != childNode &&
            childOfNeighborNode->parent != childNode->parent)
        {
          CutElemMeshError("Attempting to merge nodes "<<childOfNeighborNode->id_cat_str()<<" and "
                           <<childNode->id_cat_str()<<" but neither the 2nd node nor its parent is parent of the 1st");
        }
        childOfNeighborElem->switchNode(childNode, childOfNeighborNode);
        if (!deleteFromMap(TempNodes, childOfNeighborNode))
        {
          CutElemMeshError("Attempted to delete node: "<<childOfNeighborNode->id
                           <<" from TempNodes, but couldn't find it");
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
                         <<childOfNeighborNode->id<<" but neither the 2nd node nor its parent is parent of the 1st");
      }
      childElem->switchNode(childOfNeighborNode, childNode);
      if (!deleteFromMap(TempNodes, childNode))
      {
        CutElemMeshError("Attempted to delete node: "<<childNode->id<<" from TempNodes, but couldn't find it");
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
                         <<childOfNeighborNode->id<<" but they don't share a common parent");
      }

      if (!deleteFromMap(TempNodes, childOfNeighborNode))
      {
        CutElemMeshError("Attempted to delete node: "<<childOfNeighborNode->id
                         <<" from TempNodes, but couldn't find it");
      }
      if (!deleteFromMap(TempNodes, childNode))
      {
        CutElemMeshError("Attempted to delete node: "<<childNode->id
                         <<" from TempNodes, but couldn't find it");
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
  unsigned int num_edges = currElem->num_edges;
  std::set<node_t*> edgeNodes;

  unsigned int edgeIDplus1(edgeID<(num_edges-1) ? edgeID+1 : 0);
  edgeNodes.insert(currElem->nodes[edgeID]);
  edgeNodes.insert(currElem->nodes[edgeIDplus1]);

  node_t* embeddedNode = currElem->embedded_nodes_on_edge[edgeID];
  node_t* neighEmbeddedNode = neighborElem->embedded_nodes_on_edge[neighborEdgeID];

  if (embeddedNode != neighEmbeddedNode)
  {
    CutElemMeshError("Embedded nodes on merged edge must match");
  }

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
            currElem->parent->children[i]->embedded_nodes_on_edge[edgeID] == embeddedNode)
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
            neighborElem->parent->children[i]->embedded_nodes_on_edge[neighborEdgeID] == embeddedNode)
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

    std::set<node_t*> currLinkNodes;
    for (unsigned int k=0; k < currElem->interior_links[0].size(); k++)
      currLinkNodes.insert( currElem->interior_links[0][k] );

    std::set<node_t*> neighLinkNodes;
    for (unsigned int k=0; k < neighborElem->interior_links[0].size(); k++)
      neighLinkNodes.insert( neighborElem->interior_links[0][k] );

    std::vector<node_t*> currCommonNodes;
    std::set_intersection(currLinkNodes.begin(), currLinkNodes.end(),
                          edgeNodes.begin(), edgeNodes.end(),
                          std::inserter(currCommonNodes,currCommonNodes.end()));

    std::vector<node_t*> neighCommonNodes;
    std::set_intersection(neighLinkNodes.begin(), neighLinkNodes.end(),
                          edgeNodes.begin(), edgeNodes.end(),
                          std::inserter(neighCommonNodes,neighCommonNodes.end()));

    if (currCommonNodes.size() == 1 &&
        neighCommonNodes.size() == 1 &&
        currLinkNodes.find(embeddedNode) != currLinkNodes.end() &&
        neighLinkNodes.find(embeddedNode) != neighLinkNodes.end())
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
  unsigned int num_edges = currElem->num_edges;
  std::set<node_t*> edgeNodes;

  unsigned int edgeIDplus1(edgeID<(num_edges-1) ? edgeID+1 : 0);
  edgeNodes.insert(currElem->nodes[edgeID]);
  edgeNodes.insert(currElem->nodes[edgeIDplus1]);

  node_t* embeddedNode = currElem->embedded_nodes_on_edge[edgeID];

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
          currElem->parent->children[i]->embedded_nodes_on_edge[edgeID] == embeddedNode)
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

      std::set<node_t*> currLinkNodes;
      for (unsigned int k=0; k < currElem->interior_links[0].size(); k++)
        currLinkNodes.insert( currElem->interior_links[0][k] );

      std::vector<node_t*> currCommonNodes;
      std::set_intersection(currLinkNodes.begin(), currLinkNodes.end(),
                            edgeNodes.begin(), edgeNodes.end(),
                            std::inserter(currCommonNodes,currCommonNodes.end()));

      if (currCommonNodes.size() == 1 &&
          currLinkNodes.find(embeddedNode) != currLinkNodes.end())
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
  std::cout<<"BWS mergededgemap:"<<std::endl;
  for (memit = MergedEdgeMap.begin(); memit != MergedEdgeMap.end(); ++memit)
  {
    std::cout<<"Elems: ";
    std::set<element_t*> conn_elems = memit->second;
    std::set<element_t*>::iterator setit;
    for (setit = conn_elems.begin(); setit != conn_elems.end(); ++setit)
    {
      std::cout<<(*setit)->id<<" ";
    }
    std::cout<<std::endl;
  }

  //Go through MergedEdgeMap to find elements that are newly at the crack tip due to
  //crack growth.
  for (memit = MergedEdgeMap.begin(); memit != MergedEdgeMap.end(); ++memit)
  {
    if (memit->second.size() < 2)
    {
      CutElemMeshError("in findCrackTipElements() cannot have <2 elements on common edge");
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
          CutElemMeshError("in findCrackTipElements() only 2 of elements on tip edge can overlay");
        }
        CrackTipElements.insert(this_tip_elems[2]);
      }
      else if (olay12)
      {
        if (olay01 || olay20)
        {
          CutElemMeshError("in findCrackTipElements() only 2 of elements on tip edge can overlay");
        }
        CrackTipElements.insert(this_tip_elems[0]);
      }
      else if (olay20)
      {
        if (olay01 || olay12)
        {
          CutElemMeshError("in findCrackTipElements() only 2 of elements on tip edge can overlay");
        }
        CrackTipElements.insert(this_tip_elems[1]);
      }
    }
    else
    {
      CutElemMeshError("in findCrackTipElements() cannot have >3 elements on common edge");
    }
  }
  std::cout<<"Crack tip elements: ";
  for (sit=CrackTipElements.begin(); sit!=CrackTipElements.end(); ++sit)
  {
    std::cout<<(*sit)->id<<" ";
  }
  std::cout<<std::endl;
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
      if (currElem->local_edge_has_intersection[j])
        std::cout << currElem->embedded_nodes_on_edge[j]->id << " ";
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
    for (unsigned int j=0; j!= currElem->interior_links.size(); j++)
    {
      std::cout<<std::setw(4);
      std::cout << " " << j << " | ";
      for (unsigned int k=0; k < currElem->interior_links[j].size(); k++)
      {
        std::cout << std::setw(4) << currElem->interior_links[j][k]->id_cat_str();
      }
    }
    std::cout << std::endl;
  }
}

CutElemMesh::element_t* CutElemMesh::getElemByID(unsigned int id)
{
  std::map<unsigned int, CutElemMesh::element_t*>::iterator mit = Elements.find(id);
  if (mit == Elements.end())
  {
    CutElemMeshError("in getElemByID() could not find element: "<<id);
  }
  return mit->second;
}
