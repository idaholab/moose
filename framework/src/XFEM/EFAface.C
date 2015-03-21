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
#include "EFAface.h"

EFAface::EFAface(unsigned int n_nodes):
  _num_nodes(n_nodes),
  _nodes(_num_nodes, NULL),
  _num_edges(_num_nodes),
  _edges(_num_edges, NULL)
{}

EFAface::EFAface(const EFAfragment2D* frag):
  _num_nodes(frag->num_edges()),
  _nodes(_num_nodes, NULL),
  _num_edges(_num_nodes),
  _edges(_num_edges, NULL)
{
  for (unsigned int k = 0; k < frag->num_edges(); ++k)
  {
    EFAnode* node = frag->get_edge(k)->get_node(0);
    unsigned int kprev(k > 0 ? (k-1) : (frag->num_edges()-1));
    if (!frag->get_edge(kprev)->containsNode(node))
      node = get_edge(k)->get_node(1);
    _nodes[k] = node;
    _edges[k] = new EFAedge(*frag->get_edge(k));
  }
}

EFAface::~EFAface()
{
  for (unsigned int i = 0; i < _edges.size(); ++i)
  {
    if (_edges[i])
    {
      delete _edges[i];
      _edges[i] = NULL;
    }
  }
  for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
  {
    if (_interior_nodes[i])
    {
      delete _interior_nodes[i];
      _interior_nodes[i] = NULL;
    }
  }
}

unsigned int
EFAface::num_nodes() const
{
  return _num_nodes;
}

void
EFAface::set_node(unsigned int node_id, EFAnode* node)
{
  _nodes[node_id] = node;
}

EFAnode*
EFAface::get_node(unsigned int node_id) const
{
  return _nodes[node_id];
}

void
EFAface::switchNode(EFAnode *new_node, EFAnode *old_node)
{
  // We are not switching any embedded nodes here
  for (unsigned int i = 0; i < _num_nodes; ++i)
  {
    if (_nodes[i] == old_node)
      _nodes[i] = new_node;
  }
  for (unsigned int i = 0; i < _edges.size(); ++i)
    _edges[i]->switchNode(new_node, old_node);
}

void
EFAface::switchEmbeddedNode(EFAnode *new_emb_node,
                            EFAnode *old_emb_node)
{
  for (unsigned int i = 0; i < _num_edges; ++i)
    _edges[i]->switchNode(new_emb_node, old_emb_node);
  for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    _interior_nodes[i]->switchNode(new_emb_node, old_emb_node);
}

unsigned int
EFAface::num_edges() const
{
  return _num_edges;
}

EFAedge*
EFAface::get_edge(unsigned int edge_id) const
{
  return _edges[edge_id];
}

void
EFAface::set_edge(unsigned int edge_id, EFAedge* new_edge)
{
  _edges[edge_id] = new_edge;
}

void
EFAface::createEdges()
{
  for (unsigned int i = 0; i < _num_nodes; ++i)
  {
    unsigned int i_plus1(i < (_num_nodes-1) ? i+1 : 0);
    if (_nodes[i] != NULL && _nodes[i_plus1] != NULL)
    {
      EFAedge * new_edge = new EFAedge(_nodes[i], _nodes[i_plus1]);
      _edges[i] = new_edge;
    }
    else
      mooseError("EFAface::createEdges requires exsiting _nodes");
  }
}

bool
EFAface::overlap_with(const EFAface* other_face) const
{
  unsigned int counter = 0; // counter number of equal nodes
  bool overlap = false;
  if (_num_nodes == other_face->_num_nodes)
  {
    for (unsigned int i = 0; i < _num_nodes; ++i)
    {
      for (unsigned int j = 0; j < other_face->_num_nodes; ++j)
      {
        if (_nodes[i] == other_face->_nodes[j])
        {
          counter += 1;
          break;
        }
      } // j
    } // i
    if (counter == _num_nodes)
      overlap = true;
  }
  return overlap;
}

bool
EFAface::containsNode(const EFAnode* node) const
{
  bool contain = false;
  for (unsigned int i = 0; i < _num_nodes; ++i)
  {
    if (_nodes[i] == node)
    {
      contain = true;
      break;
    }
  } //i
  if (!contain)
  {
    for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    {
      if (_interior_nodes[i]->get_node() == node)
      {
        contain = true;
        break;
      }
    } // i
  }
  return contain;
}

bool
EFAface::containsFace(const EFAface* other_face) const
{
  unsigned int counter = 0;
  for (unsigned int i = 0; i < other_face->_num_nodes; ++i)
  {
    if (containsNode(other_face->_nodes[i]))
      counter += 1;
  }
  if (counter == other_face->_num_nodes)
    return true;
  else
    return false;
}

void
EFAface::remove_embedded_node(EFAnode* emb_node)
{
  for (unsigned int i = 0; i < _num_edges; ++i)
    if (_edges[i]->containsNode(emb_node))
      _edges[i]->remove_embedded_node(emb_node);

  unsigned int index = 0;
  bool node_found = false;
  for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
  {
    if (_interior_nodes[i]->get_node() == emb_node)
    {
      node_found = true;
      index = i;
      break;
    }  
  }
  if (node_found)
    _interior_nodes.erase(_interior_nodes.begin() + index);
}

std::vector<EFAface*>
EFAface::split() const
{
  // contruct a fragment from this face
  EFAfragment2D* frag_tmp = new EFAfragment2D(this);
  std::vector<EFAfragment2D*> new_frags = frag_tmp->split();

  // copy new_frags to new_faces
  std::vector<EFAface*> new_faces;
  for (unsigned int i = 0; i < new_frags.size(); ++i)
    new_faces.push_back(new EFAface(new_frags[i]));

  // delete frag_tmp and new_frags
  delete frag_tmp;
  for (unsigned int i = 0; i < new_frags.size(); ++i)
    delete new_frags[i];
  new_frags.clear();

  return new_faces;
}
