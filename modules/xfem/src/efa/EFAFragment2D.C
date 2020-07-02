//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EFAElement2D.h"

#include "EFANode.h"
#include "EFAEdge.h"
#include "EFAFace.h"
#include "EFAFragment2D.h"

#include "EFAFaceNode.h"
#include "EFAFuncs.h"
#include "EFAError.h"

EFAFragment2D::EFAFragment2D(EFAElement2D * host,
                             bool create_boundary_edges,
                             const EFAElement2D * from_host,
                             unsigned int frag_id)
  : EFAFragment(), _host_elem(host)
{
  if (create_boundary_edges)
  {
    if (!from_host)
      EFAError("EFAfragment2D constructor must have a from_host to copy from");
    if (frag_id == std::numeric_limits<unsigned int>::max()) // copy the from_host itself
    {
      for (unsigned int i = 0; i < from_host->numEdges(); ++i)
        _boundary_edges.push_back(new EFAEdge(*from_host->getEdge(i)));
    }
    else
    {
      if (frag_id > from_host->numFragments() - 1)
        EFAError("In EFAfragment2D constructor fragment_copy_index out of bounds");
      for (unsigned int i = 0; i < from_host->getFragment(frag_id)->numEdges(); ++i)
        _boundary_edges.push_back(new EFAEdge(*from_host->getFragmentEdge(frag_id, i)));
    }
  }
}

EFAFragment2D::EFAFragment2D(EFAElement2D * host, const EFAFace * from_face)
  : EFAFragment(), _host_elem(host)
{
  for (unsigned int i = 0; i < from_face->numEdges(); ++i)
    _boundary_edges.push_back(new EFAEdge(*from_face->getEdge(i)));
}

EFAFragment2D::~EFAFragment2D()
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
EFAFragment2D::switchNode(EFANode * new_node, EFANode * old_node)
{
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
    _boundary_edges[i]->switchNode(new_node, old_node);
}

bool
EFAFragment2D::containsNode(EFANode * node) const
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
EFAFragment2D::getNumCuts() const
{
  unsigned int num_cut_edges = 0;
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    if (_boundary_edges[i]->hasIntersection())
      num_cut_edges += _boundary_edges[i]->numEmbeddedNodes();
  }
  return num_cut_edges;
}

unsigned int
EFAFragment2D::getNumCutNodes() const
{
  unsigned int num_cut_nodes = 0;
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
    if (_boundary_edges[i]->getNode(0)->category() == EFANode::N_CATEGORY_EMBEDDED_PERMANENT)
      num_cut_nodes++;
  return num_cut_nodes;
}

std::set<EFANode *>
EFAFragment2D::getAllNodes() const
{
  std::set<EFANode *> nodes;
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    nodes.insert(_boundary_edges[i]->getNode(0));
    nodes.insert(_boundary_edges[i]->getNode(1));
  }
  return nodes;
}

bool
EFAFragment2D::isConnected(EFAFragment * other_fragment) const
{
  bool is_connected = false;
  EFAFragment2D * other_frag2d = dynamic_cast<EFAFragment2D *>(other_fragment);
  if (!other_frag2d)
    EFAError("in isConnected other_fragment is not of type EFAfragement2D");

  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    for (unsigned int j = 0; j < other_frag2d->numEdges(); ++j)
    {
      if (_boundary_edges[i]->equivalent(*other_frag2d->getEdge(j)))
      {
        is_connected = true;
        break;
      }
    }
    if (is_connected)
      break;
  } // i
  return is_connected;
}

void
EFAFragment2D::removeInvalidEmbeddedNodes(std::map<unsigned int, EFANode *> & EmbeddedNodes)
{
  // if a fragment only has 1 intersection which is in an interior edge
  // remove this embedded node (MUST DO THIS AFTER combine_tip_edges())
  if (getNumCuts() == 1)
  {
    for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
    {
      if (isEdgeInterior(i) && _boundary_edges[i]->hasIntersection())
      {
        if (_host_elem->numInteriorNodes() != 1)
          EFAError("host element must have 1 interior node at this point");
        Efa::deleteFromMap(EmbeddedNodes, _boundary_edges[i]->getEmbeddedNode(0));
        _boundary_edges[i]->removeEmbeddedNodes();
        _host_elem->deleteInteriorNodes();
        break;
      }
    } // i
  }
}

void
EFAFragment2D::combineTipEdges()
{
  // combine the tip edges in a crack tip fragment
  // N.B. the host elem can only have one elem_tip_edge, otherwise it should have already been
  // completely split
  if (!_host_elem)
    EFAError("In combine_tip_edges() the frag must have host_elem");

  bool has_tip_edges = false;
  unsigned int elem_tip_edge_id = std::numeric_limits<unsigned int>::max();
  std::vector<unsigned int> frag_tip_edge_id;
  for (unsigned int i = 0; i < _host_elem->numEdges(); ++i)
  {
    frag_tip_edge_id.clear();
    if (_host_elem->getEdge(i)->hasIntersection())
    {
      for (unsigned int j = 0; j < _boundary_edges.size(); ++j)
      {
        if (_host_elem->getEdge(i)->containsEdge(*_boundary_edges[j]))
          frag_tip_edge_id.push_back(j);
      }                                 // j
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
    unsigned int edge0_next(frag_tip_edge_id[0] < (numEdges() - 1) ? frag_tip_edge_id[0] + 1 : 0);
    if (edge0_next != frag_tip_edge_id[1])
      EFAError("frag_tip_edge_id[1] must be the next edge of frag_tip_edge_id[0]");

    // get the two end nodes of the new edge
    EFANode * node1 = _boundary_edges[frag_tip_edge_id[0]]->getNode(0);
    EFANode * emb_node = _boundary_edges[frag_tip_edge_id[0]]->getNode(1);
    EFANode * node2 = _boundary_edges[frag_tip_edge_id[1]]->getNode(1);
    if (emb_node != _boundary_edges[frag_tip_edge_id[1]]->getNode(0))
      EFAError("fragment edges are not correctly set up");

    // get the new edge with one intersection
    EFAEdge * elem_edge = _host_elem->getEdge(elem_tip_edge_id);
    double xi_node1 = elem_edge->distanceFromNode1(node1);
    double xi_node2 = elem_edge->distanceFromNode1(node2);
    double xi_emb = elem_edge->distanceFromNode1(emb_node);
    double position = (xi_emb - xi_node1) / (xi_node2 - xi_node1);
    EFAEdge * full_edge = new EFAEdge(node1, node2);
    full_edge->addIntersection(position, emb_node, node1);

    // combine the two original fragment edges
    delete _boundary_edges[frag_tip_edge_id[0]];
    delete _boundary_edges[frag_tip_edge_id[1]];
    _boundary_edges[frag_tip_edge_id[0]] = full_edge;
    _boundary_edges.erase(_boundary_edges.begin() + frag_tip_edge_id[1]);
  }
}

/*
std::vector<EFAnode*>
EFAfragment::commonNodesWithEdge(EFAEdge & other_edge)
{
  std::vector<EFAnode*> common_nodes;
  for (unsigned int i = 0; i < 2; ++i)
  {
    EFAnode* edge_node = other_edge.node_ptr(i);
    if (containsNode(edge_node))
      common_nodes.push_back(edge_node);
  }
  return common_nodes;
}
*/

bool
EFAFragment2D::isEdgeInterior(unsigned int edge_id) const
{
  if (!_host_elem)
    EFAError("in isEdgeInterior fragment must have host elem");

  bool edge_in_elem_edge = false;

  for (unsigned int i = 0; i < _host_elem->numEdges(); ++i)
  {
    if (_host_elem->getEdge(i)->containsEdge(*_boundary_edges[edge_id]))
    {
      edge_in_elem_edge = true;
      break;
    }
  }
  if (!edge_in_elem_edge)
    return true; // yes, is interior
  else
    return false;
}

std::vector<unsigned int>
EFAFragment2D::getInteriorEdgeID() const
{
  std::vector<unsigned int> interior_edge_id;
  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    if (isEdgeInterior(i))
      interior_edge_id.push_back(i);
  }
  return interior_edge_id;
}

bool
EFAFragment2D::isSecondaryInteriorEdge(unsigned int edge_id) const
{
  bool is_second_cut = false;
  if (!_host_elem)
    EFAError("in isSecondaryInteriorEdge fragment must have host elem");

  for (unsigned int i = 0; i < _host_elem->numInteriorNodes(); ++i)
  {
    if (_boundary_edges[edge_id]->containsNode(_host_elem->getInteriorNode(i)->getNode()))
    {
      is_second_cut = true;
      break;
    }
  }
  return is_second_cut;
}

unsigned int
EFAFragment2D::numEdges() const
{
  return _boundary_edges.size();
}

EFAEdge *
EFAFragment2D::getEdge(unsigned int edge_id) const
{
  if (edge_id > _boundary_edges.size() - 1)
    EFAError("in EFAfragment2D::get_edge, index out of bounds");
  return _boundary_edges[edge_id];
}

void
EFAFragment2D::addEdge(EFAEdge * new_edge)
{
  _boundary_edges.push_back(new_edge);
}

std::set<EFANode *>
EFAFragment2D::getEdgeNodes(unsigned int edge_id) const
{
  std::set<EFANode *> edge_nodes;
  edge_nodes.insert(_boundary_edges[edge_id]->getNode(0));
  edge_nodes.insert(_boundary_edges[edge_id]->getNode(1));
  return edge_nodes;
}

EFAElement2D *
EFAFragment2D::getHostElement() const
{
  return _host_elem;
}

std::vector<EFAFragment2D *>
EFAFragment2D::split()
{
  // This method will split one existing fragment into one or two
  // new fragments and return them.
  // N.B. each boundary each can only have 1 cut at most
  std::vector<EFAFragment2D *> new_fragments;
  std::vector<std::vector<EFANode *>> fragment_nodes(
      2);                       // vectors of EFA nodes in the two fragments
  unsigned int frag_number = 0; // Index of the current fragment that we are assmbling nodes into
  unsigned int edge_cut_count = 0;
  unsigned int node_cut_count = 0;
  for (unsigned int iedge = 0; iedge < _boundary_edges.size(); ++iedge)
  {
    fragment_nodes[frag_number].push_back(_boundary_edges[iedge]->getNode(0));

    if (_boundary_edges[iedge]->getNode(0)->category() ==
        EFANode::N_CATEGORY_EMBEDDED_PERMANENT) // if current node has been cut change fragment
    {
      ++node_cut_count;
      frag_number = 1 - frag_number; // Toggle between 0 and 1
      fragment_nodes[frag_number].push_back(_boundary_edges[iedge]->getNode(0));
    }

    if (_boundary_edges[iedge]->numEmbeddedNodes() > 1)
      EFAError("A fragment boundary edge can't have more than 1 cuts");
    if (_boundary_edges[iedge]->hasIntersection()) // if current edge is cut add cut intersection //
                                                   // node to both fragments and and change fragment
    {
      fragment_nodes[frag_number].push_back(_boundary_edges[iedge]->getEmbeddedNode(0));
      ++edge_cut_count;
      frag_number = 1 - frag_number; // Toggle between 0 and 1
      fragment_nodes[frag_number].push_back(_boundary_edges[iedge]->getEmbeddedNode(0));
    }
  }

  if ((edge_cut_count + node_cut_count) > 1) // any two cuts case
  {
    for (unsigned int frag_idx = 0; frag_idx < 2; ++frag_idx) // Create 2 fragments
    {
      auto & this_frag_nodes = fragment_nodes[frag_idx];
      // check to make sure an edge wasn't cut
      if (this_frag_nodes.size() >= 3)
      {
        EFAFragment2D * new_frag = new EFAFragment2D(_host_elem, false, NULL);
        for (unsigned int inode = 0; inode < this_frag_nodes.size() - 1; inode++)
          new_frag->addEdge(new EFAEdge(this_frag_nodes[inode], this_frag_nodes[inode + 1]));

        new_frag->addEdge(
            new EFAEdge(this_frag_nodes[this_frag_nodes.size() - 1], this_frag_nodes[0]));

        new_fragments.push_back(new_frag);
      }
    }
  }
  else if (edge_cut_count == 1) // single edge cut case
  {
    EFAFragment2D * new_frag = new EFAFragment2D(_host_elem, false, NULL);
    for (unsigned int inode = 0; inode < fragment_nodes[0].size() - 1;
         inode++) // assemble fragment part 1
      new_frag->addEdge(new EFAEdge(fragment_nodes[0][inode], fragment_nodes[0][inode + 1]));

    for (unsigned int inode = 0; inode < fragment_nodes[1].size() - 1;
         inode++) // assemble fragment part 2
      new_frag->addEdge(new EFAEdge(fragment_nodes[1][inode], fragment_nodes[1][inode + 1]));

    new_frag->addEdge(
        new EFAEdge(fragment_nodes[1][fragment_nodes[1].size() - 1], fragment_nodes[0][0]));

    new_fragments.push_back(new_frag);
  }
  else if (node_cut_count == 1) // single node cut case
  {
    EFAFragment2D * new_frag = new EFAFragment2D(_host_elem, false, NULL);
    for (unsigned int iedge = 0; iedge < _boundary_edges.size(); ++iedge)
    {
      EFANode * first_node_on_edge = _boundary_edges[iedge]->getNode(0);
      EFANode * second_node_on_edge = _boundary_edges[iedge]->getNode(1);
      new_frag->addEdge(new EFAEdge(first_node_on_edge, second_node_on_edge));
    }

    new_fragments.push_back(new_frag);
  }

  return new_fragments;
}
