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

#include "EFAelement.h"
#include "EFAfragment.h"

EFAfragment::EFAfragment(EFAelement * host,
                         bool create_boundary_edges,
                         const EFAelement * from_host,
                         unsigned int fragment_copy_index)
{
  host_elem = host;
  if (create_boundary_edges)
  {
    if (!from_host)
      mooseError("If fragment_copy_index given, EFAfragment constructor must have a from_host to copy from");
    if (fragment_copy_index == std::numeric_limits<unsigned int>::max())
    {
      for (unsigned int i = 0; i < from_host->edges.size(); ++i)
        boundary_edges.push_back(new EFAedge(*from_host->edges[i]));
    }
    else
    {
      if (from_host->fragments.size() <= fragment_copy_index)
      {
        std::cout<<"num frags: "<<from_host->fragments.size()<<" index: "<<fragment_copy_index<<std::endl;
        mooseError("In EFAfragment constructor fragment_copy_index out of bounds");
      }
      for (unsigned int i = 0; i < from_host->fragments[fragment_copy_index]->boundary_edges.size(); ++i)
        boundary_edges.push_back(new EFAedge(*from_host->fragments[fragment_copy_index]->boundary_edges[i]));
    }
  }
}

EFAfragment::EFAfragment(const EFAfragment & other_frag,
                         EFAelement * host)
{
  host_elem = host;
  for (unsigned int i = 0; i < other_frag.boundary_edges.size(); ++i)
  {
    EFAedge * new_edge = new EFAedge(*other_frag.boundary_edges[i]);
    boundary_edges.push_back(new_edge);
  }
}

EFAfragment::~EFAfragment()
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
EFAfragment::switchNode(EFAnode *new_node, EFAnode *old_node)
{
  for (unsigned int i = 0; i < boundary_edges.size(); ++i)
    boundary_edges[i]->switchNode(new_node, old_node);
}

bool
EFAfragment::containsNode(EFAnode *node)
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
EFAfragment::isConnected(EFAfragment &other_fragment)
{
  for (unsigned int i = 0; i < boundary_edges.size(); ++i)
    for (unsigned int j = 0; j < other_fragment.boundary_edges.size(); ++j)
      if (boundary_edges[i]->equivalent(*other_fragment.boundary_edges[j]))
        return true;
  return false;
}

void
EFAfragment::combine_tip_edges()
{
  // combine the tip edges in a crack tip fragment
  // N.B. the host elem can only have one elem_tip_edge, otherwise it should have already been completely split
  if (!host_elem)
    mooseError("In combine_tip_edges() the frag must have host_elem");

  bool has_tip_edges = false;
  unsigned int elem_tip_edge_id = 99999;
  std::vector<unsigned int> frag_tip_edge_id;
  for (unsigned int i = 0; i < host_elem->num_edges; ++i)
  {
    frag_tip_edge_id.clear();
    if (host_elem->edges[i]->has_intersection())
    {
      for (unsigned int j = 0; j < boundary_edges.size(); ++j)
      {
        if (host_elem->edges[i]->containsEdge(*boundary_edges[j]))
          frag_tip_edge_id.push_back(j);
      } // j
      if (frag_tip_edge_id.size() == 2) // combine the two frag edges on this elem edge
      {
        has_tip_edges = true;
        elem_tip_edge_id = i;
        break;
      }
    }
  } // i
  if (has_tip_edges)
  {
    delete boundary_edges[frag_tip_edge_id[0]];
    delete boundary_edges[frag_tip_edge_id[1]];
    boundary_edges[frag_tip_edge_id[0]] = new EFAedge(*host_elem->edges[elem_tip_edge_id]);
    boundary_edges.erase(boundary_edges.begin()+frag_tip_edge_id[1]);
  }
}

std::vector<EFAfragment*>
EFAfragment::split()
{
  // This method will split one existing fragment into one or two
  // new fragments and return them

  std::vector<EFAfragment *> new_fragments;
  std::vector<unsigned int> cut_edges;
  for (unsigned int iedge = 0; iedge < boundary_edges.size(); ++iedge)
  {
    if(boundary_edges[iedge]->has_intersection())
      cut_edges.push_back(iedge);
  }

  if (cut_edges.size() == 0)
  {} // do nothing
  else if (cut_edges.size() > 2)
    mooseError("In split() Fragment cannot have more than 2 cut edges");
  else // cut_edges.size () == 1 || 2
  {
    unsigned int iedge=0;
    unsigned int icutedge=0;

    do //loop over new fragments
    {
      EFAfragment * new_frag = new EFAfragment(host_elem, false, NULL);

      do //loop over edges
      {
        if (iedge == cut_edges[icutedge])
        {
          EFAnode * first_node_on_edge = boundary_edges[iedge]->get_node(0);
          unsigned int iprevedge(iedge>0 ? iedge-1 : boundary_edges.size()-1);
          if (!boundary_edges[iprevedge]->containsNode(first_node_on_edge))
          {
            first_node_on_edge = boundary_edges[iedge]->get_node(1);
            if (!boundary_edges[iprevedge]->containsNode(first_node_on_edge))
              mooseError("Previous edge does not contain either of the nodes in this edge");
          }
          EFAnode * embedded_node1 = boundary_edges[iedge]->get_embedded_node();
          new_frag->boundary_edges.push_back(new EFAedge(first_node_on_edge, embedded_node1));

          ++icutedge; // jump to next cut edge or jump back to this edge when only 1 cut edge
          if (icutedge == cut_edges.size())
            icutedge = 0;
          iedge = cut_edges[icutedge];
          EFAnode * embedded_node2 = boundary_edges[iedge]->get_embedded_node();
          if (embedded_node2 != embedded_node1)
            new_frag->boundary_edges.push_back(new EFAedge(embedded_node1, embedded_node2));

          EFAnode * second_node_on_edge = boundary_edges[iedge]->get_node(1);
          unsigned int inextedge(iedge<(boundary_edges.size()-1) ? iedge+1 : 0);
          if (!boundary_edges[inextedge]->containsNode(second_node_on_edge))
          {
            second_node_on_edge = boundary_edges[iedge]->get_node(0);
            if (!boundary_edges[inextedge]->containsNode(second_node_on_edge))
              mooseError("Previous edge does not contain either of the nodes in this edge");
          }
          new_frag->boundary_edges.push_back(new EFAedge(embedded_node2, second_node_on_edge));
        }
        else
          new_frag->boundary_edges.push_back(new EFAedge(*boundary_edges[iedge]));

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

std::vector<EFAnode*>
EFAfragment::commonNodesWithEdge(EFAedge & other_edge)
{
  std::vector<EFAnode*> common_nodes;
  for (unsigned int i = 0; i < 2; ++i)
  {
    EFAnode* edge_node = other_edge.get_node(i);
    if (containsNode(edge_node))
      common_nodes.push_back(edge_node);
  }
  return common_nodes;
}

unsigned int
EFAfragment::get_num_cuts()
{
  unsigned int num_cut_edges = 0;
  for (unsigned int i = 0; i < boundary_edges.size(); ++i)
  {
    if (boundary_edges[i]->has_intersection())
      num_cut_edges += 1;
  }
  return num_cut_edges;
}

EFAelement*
EFAfragment::get_host()
{
  return host_elem;
}

std::vector<unsigned int>
EFAfragment::get_interior_edge_id()
{
  std::vector<unsigned int> interior_edge_id;
  for (unsigned int i = 0; i < boundary_edges.size(); ++i)
  {
    if (boundary_edges[i]->is_interior_edge())
      interior_edge_id.push_back(i);
  }
  return interior_edge_id;
}

bool
EFAfragment::is_edge_second_cut(unsigned int edge_id)
{
  bool is_second_cut = false;
  if (!host_elem)
    mooseError("in is_edge_second_cut() fragment must have host elem");
  for (unsigned int i = 0; i < host_elem->interior_nodes.size(); ++i)
    if (boundary_edges[edge_id]->containsNode(host_elem->interior_nodes[i]->get_node()))
    {
      is_second_cut = true;
      break;
    }
  return is_second_cut;
}
