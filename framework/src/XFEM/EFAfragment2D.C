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

#include "EFAelement2D.h"
#include "EFAfragment2D.h"
#include "EFAfuncs.h"
#include <typeinfo>

EFAfragment2D::EFAfragment2D(EFAelement2D * host, bool create_boundary_edges,
                             const EFAelement2D * from_host, unsigned int frag_id):
  EFAfragment(),
  _host_elem(host)
{
  if (create_boundary_edges)
  {
    if (!from_host)
      mooseError("EFAfragment2D constructor must have a from_host to copy from");
    if (frag_id == std::numeric_limits<unsigned int>::max())// copy the from_host itself
    {
      for (unsigned int i = 0; i < from_host->num_edges(); ++i)
        _boundary_edges.push_back(new EFAedge(*from_host->get_edge(i)));
    }
    else
    {
      if (frag_id > from_host->num_frags() - 1)
        mooseError("In EFAfragment2D constructor fragment_copy_index out of bounds");
      for (unsigned int i = 0; i < from_host->get_fragment(frag_id)->num_edges(); ++i)
        _boundary_edges.push_back(new EFAedge(*from_host->get_frag_edge(frag_id,i)));
    }
  }
}

EFAfragment2D::EFAfragment2D(EFAelement2D* host, const EFAface* from_face):
  EFAfragment(),
  _host_elem(host)
{
  for (unsigned int i = 0; i < from_face->num_edges(); ++i)
    _boundary_edges.push_back(new EFAedge(*from_face->get_edge(i)));
}

EFAfragment2D::~EFAfragment2D()
{
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    if (_boundary_edges[i])
    {
      delete _boundary_edges[i];
      _boundary_edges[i] = NULL;
    }
  }
}

void
EFAfragment2D::switchNode(EFAnode *new_node, EFAnode *old_node)
{
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
    _boundary_edges[i]->switchNode(new_node, old_node);
}

bool
EFAfragment2D::containsNode(EFAnode *node) const
{
  bool contains = false;
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    if (_boundary_edges[i]->containsNode(node))
    {
      contains = true;
      break;
    }
  }
  return contains;
}

unsigned int
EFAfragment2D::get_num_cuts() const
{
  unsigned int num_cut_edges = 0;
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    if (_boundary_edges[i]->has_intersection())
      num_cut_edges += _boundary_edges[i]->num_embedded_nodes();
  }
  return num_cut_edges;
}

std::set<EFAnode*>
EFAfragment2D::get_all_nodes() const
{
  std::set<EFAnode*> nodes;
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    nodes.insert(_boundary_edges[i]->get_node(0));
    nodes.insert(_boundary_edges[i]->get_node(1));
  }
  return nodes;
}

bool
EFAfragment2D::isConnected(EFAfragment* other_fragment) const
{
  bool is_connected = false;
  EFAfragment2D* other_frag2d = dynamic_cast<EFAfragment2D*>(other_fragment);
  if (!other_frag2d)
    mooseError("in isConnected other_fragment is not of type EFAfragement2D");

  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    for (unsigned int j = 0; j < other_frag2d->num_edges(); ++j)
    {
      if (_boundary_edges[i]->equivalent(*other_frag2d->get_edge(j)))
      {
        is_connected = true;
        break;
      }
    } // j
    if (is_connected) break;
  } // i
  return is_connected;
}

void
EFAfragment2D::remove_invalid_embedded(std::map<unsigned int, EFAnode*> &EmbeddedNodes)
{
  // if a fragment only has 1 intersection which is in an interior edge
  // remove this embedded node (MUST DO THIS AFTER combine_tip_edges())
  if (get_num_cuts() == 1)
  {
    for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
    {
      if (is_edge_interior(i) && _boundary_edges[i]->has_intersection())
      {
        if (_host_elem->num_interior_nodes() != 1)
          mooseError("host element must have 1 interior node at this point");
        deleteFromMap(EmbeddedNodes, _boundary_edges[i]->get_embedded_node(0));
        _boundary_edges[i]->remove_embedded_node();
        _host_elem->delete_interior_nodes();
        break;
      }
    } // i
  }
}

void
EFAfragment2D::combine_tip_edges()
{
  // combine the tip edges in a crack tip fragment
  // N.B. the host elem can only have one elem_tip_edge, otherwise it should have already been completely split
  if (!_host_elem)
    mooseError("In combine_tip_edges() the frag must have host_elem");

  bool has_tip_edges = false;
  unsigned int elem_tip_edge_id = 99999;
  std::vector<unsigned int> frag_tip_edge_id;
  for (unsigned int i = 0; i < _host_elem->num_edges(); ++i)
  {
    frag_tip_edge_id.clear();
    if (_host_elem->get_edge(i)->has_intersection())
    {
      for (unsigned int j = 0; j < _boundary_edges.size(); ++j)
      {
        if (_host_elem->get_edge(i)->containsEdge(*_boundary_edges[j]))
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
    // frag_tip_edge_id[0] must precede frag_tip_edge_id[1]
    unsigned int edge0_next(frag_tip_edge_id[0] < (num_edges()-1) ? frag_tip_edge_id[0]+1 : 0);
    if (edge0_next != frag_tip_edge_id[1])
      mooseError("frag_tip_edge_id[1] must be the next edge of frag_tip_edge_id[0]");

    // get the two end nodes of the new edge
    EFAnode* node1 = _boundary_edges[frag_tip_edge_id[0]]->get_node(0);
    EFAnode* emb_node = _boundary_edges[frag_tip_edge_id[0]]->get_node(1);
    EFAnode* node2 = _boundary_edges[frag_tip_edge_id[1]]->get_node(1);
    if (emb_node != _boundary_edges[frag_tip_edge_id[1]]->get_node(0))
      mooseError("fragment edges are not correctly set up");

    // get the new edge with one intersection
    EFAedge* elem_edge = _host_elem->get_edge(elem_tip_edge_id);
    double xi_node1 = elem_edge->distance_from_node1(node1);
    double xi_node2 = elem_edge->distance_from_node1(node2);
    double xi_emb = elem_edge->distance_from_node1(emb_node);
    double position = (xi_emb - xi_node1)/(xi_node2 - xi_node1);
    EFAedge* full_edge = new EFAedge(node1, node2);
    full_edge->add_intersection(position, emb_node, node1);

    // combine the two original fragment edges
    delete _boundary_edges[frag_tip_edge_id[0]];
    delete _boundary_edges[frag_tip_edge_id[1]];
    _boundary_edges[frag_tip_edge_id[0]] = full_edge;
    _boundary_edges.erase(_boundary_edges.begin()+frag_tip_edge_id[1]);
  }
}

/*
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
*/

bool
EFAfragment2D::is_edge_interior(unsigned int edge_id) const
{
  if (!_host_elem)
    mooseError("in is_edge_interior() fragment must have host elem");

  bool edge_in_elem_edge = false;
  for (unsigned int i = 0; i < _host_elem->num_edges(); ++i)
  {
    if (_host_elem->get_edge(i)->containsEdge(*_boundary_edges[edge_id]))
    {
      edge_in_elem_edge = true;
      break;
    }
  } // i
  if (!edge_in_elem_edge)
    return true; // yes, is interior
  else
    return false;
}

std::vector<unsigned int>
EFAfragment2D::get_interior_edge_id() const
{
  std::vector<unsigned int> interior_edge_id;
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    if (is_edge_interior(i))
      interior_edge_id.push_back(i);
  }
  return interior_edge_id;
}

bool
EFAfragment2D::isSecondaryInteriorEdge(unsigned int edge_id) const
{
  bool is_second_cut = false;
  if (!_host_elem)
    mooseError("in isSecondaryInteriorEdge fragment must have host elem");

  for (unsigned int i = 0; i < _host_elem->num_interior_nodes(); ++i)
  {
    if (_boundary_edges[edge_id]->containsNode(_host_elem->get_interior_node(i)->get_node()))
    {
      is_second_cut = true;
      break;
    }
  }
  return is_second_cut;
}

unsigned int
EFAfragment2D::num_edges() const
{
  return _boundary_edges.size();
}

EFAedge*
EFAfragment2D::get_edge(unsigned int edge_id) const
{
  if (edge_id > _boundary_edges.size() - 1)
    mooseError("in EFAfragment2D::get_edge, index out of bounds");
  return _boundary_edges[edge_id];
}

void
EFAfragment2D::add_edge(EFAedge* new_edge)
{
  _boundary_edges.push_back(new_edge);
}

std::set<EFAnode*>
EFAfragment2D::get_edge_nodes(unsigned int edge_id) const
{
  std::set<EFAnode*> edge_nodes;
  edge_nodes.insert(_boundary_edges[edge_id]->get_node(0));
  edge_nodes.insert(_boundary_edges[edge_id]->get_node(1));
  return edge_nodes;
}

EFAelement2D*
EFAfragment2D::get_host() const
{
  return _host_elem;
}

std::vector<EFAfragment2D*>
EFAfragment2D::split()
{
  // This method will split one existing fragment into one or two
  // new fragments and return them.
  // N.B. each boundary each can only have 1 cut at most
  std::vector<EFAfragment2D*> new_fragments;
  std::vector<unsigned int> cut_edges;
  for (unsigned int iedge = 0; iedge < _boundary_edges.size(); ++iedge)
  {
    if (_boundary_edges[iedge]->num_embedded_nodes() > 1)
      mooseError("A fragment boundary edge can't have more than 1 cuts");
    if (_boundary_edges[iedge]->has_intersection())
      cut_edges.push_back(iedge);
  }

  if (cut_edges.size() > 2)
    mooseError("In split() fragment cannot have more than 2 cut edges");
  else if (cut_edges.size() == 1 || cut_edges.size() == 2)
  {
    unsigned int iedge=0;
    unsigned int icutedge=0;

    do //loop over new fragments
    {
      EFAfragment2D * new_frag = new EFAfragment2D(_host_elem, false, NULL);

      do //loop over edges
      {
        if (iedge == cut_edges[icutedge])
        {
          EFAnode * first_node_on_edge = _boundary_edges[iedge]->get_node(0);
          unsigned int iprevedge(iedge>0 ? iedge-1 : _boundary_edges.size()-1);
          if (!_boundary_edges[iprevedge]->containsNode(first_node_on_edge))
          {
            first_node_on_edge = _boundary_edges[iedge]->get_node(1);
            if (!_boundary_edges[iprevedge]->containsNode(first_node_on_edge))
              mooseError("Previous edge does not contain either of the nodes in this edge");
          }
          EFAnode * embedded_node1 = _boundary_edges[iedge]->get_embedded_node(0);
          new_frag->add_edge(new EFAedge(first_node_on_edge, embedded_node1));

          ++icutedge; // jump to next cut edge or jump back to this edge when only 1 cut edge
          if (icutedge == cut_edges.size())
            icutedge = 0;
          iedge = cut_edges[icutedge];
          EFAnode * embedded_node2 = _boundary_edges[iedge]->get_embedded_node(0);
          if (embedded_node2 != embedded_node1)
            new_frag->add_edge(new EFAedge(embedded_node1, embedded_node2));

          EFAnode * second_node_on_edge = _boundary_edges[iedge]->get_node(1);
          unsigned int inextedge(iedge<(_boundary_edges.size()-1) ? iedge+1 : 0);
          if (!_boundary_edges[inextedge]->containsNode(second_node_on_edge))
          {
            second_node_on_edge = _boundary_edges[iedge]->get_node(0);
            if (!_boundary_edges[inextedge]->containsNode(second_node_on_edge))
              mooseError("Next edge does not contain either of the nodes in this edge");
          }
          new_frag->add_edge(new EFAedge(embedded_node2, second_node_on_edge));
        }
        else // not a cut edge
          new_frag->add_edge(new EFAedge(*_boundary_edges[iedge]));

        ++iedge;
        if (iedge == _boundary_edges.size())
          iedge = 0;
      }
      while(!_boundary_edges[iedge]->containsEdge(*new_frag->get_edge(0)));

      if (cut_edges.size() > 1)
      { //set the starting point for the loop over the other part of the element
        iedge = cut_edges[0]+1;
        if (iedge == _boundary_edges.size())
          iedge = 0;
      }

      new_fragments.push_back(new_frag);
    }
    while(new_fragments.size() < cut_edges.size());
  }

  return new_fragments;
}

