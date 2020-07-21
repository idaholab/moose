//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EFAFace.h"

#include "EFAFaceNode.h"
#include "EFANode.h"
#include "EFAEdge.h"
#include "EFAFragment2D.h"
#include "EFAFuncs.h"
#include "EFAError.h"

EFAFace::EFAFace(unsigned int n_nodes, unsigned int num_interior_face_nodes)
  : _num_nodes(n_nodes),
    _nodes(_num_nodes, NULL),
    _num_edges(_num_nodes),
    _edges(_num_edges, NULL),
    _face_interior_nodes(num_interior_face_nodes, NULL)
{
}

EFAFace::EFAFace(const EFAFace & other_face)
  : _num_nodes(other_face._num_nodes),
    _nodes(_num_nodes, NULL),
    _num_edges(_num_nodes),
    _edges(_num_edges, NULL)
{
  for (unsigned int k = 0; k < other_face._num_nodes; ++k)
  {
    _nodes[k] = other_face._nodes[k];
    _edges[k] = new EFAEdge(*other_face._edges[k]);
  }
  for (unsigned int k = 0; k < other_face._interior_nodes.size(); ++k)
    _interior_nodes.push_back(new EFAFaceNode(*other_face._interior_nodes[k]));
}

EFAFace::EFAFace(const EFAFragment2D * frag)
  : _num_nodes(frag->numEdges()),
    _nodes(_num_nodes, NULL),
    _num_edges(_num_nodes),
    _edges(_num_edges, NULL)
{
  for (unsigned int k = 0; k < frag->numEdges(); ++k)
  {
    EFANode * node = frag->getEdge(k)->getNode(0);
    unsigned int kprev(k > 0 ? (k - 1) : (frag->numEdges() - 1));
    if (!frag->getEdge(kprev)->containsNode(node))
      node = getEdge(k)->getNode(1);
    _nodes[k] = node;
    _edges[k] = new EFAEdge(*frag->getEdge(k));
  }
}

EFAFace::~EFAFace()
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

void
EFAFace::setInteriorFaceNode(unsigned int i, EFANode * node)
{
  _face_interior_nodes[i] = node;
}

unsigned int
EFAFace::numNodes() const
{
  return _nodes.size();
}

void
EFAFace::setNode(unsigned int node_id, EFANode * node)
{
  _nodes[node_id] = node;
}

EFANode *
EFAFace::getNode(unsigned int node_id) const
{
  return _nodes[node_id];
}

void
EFAFace::switchNode(EFANode * new_node, EFANode * old_node)
{
  bool is_face_node = true;
  for (unsigned int i = 0; i < _num_nodes; ++i)
  {
    if (_nodes[i] == old_node)
    {
      _nodes[i] = new_node;
      is_face_node = false;
    }
  }
  if (is_face_node)
  {
    for (unsigned int i = 0; i < _face_interior_nodes.size(); ++i)
      if (_face_interior_nodes[i] == old_node)
        _face_interior_nodes[i] = new_node;
  }
  else
  {
    for (unsigned int i = 0; i < _edges.size(); ++i)
      _edges[i]->switchNode(new_node, old_node);
    for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
      _interior_nodes[i]->switchNode(new_node, old_node);
  }
}

bool
EFAFace::getMasterInfo(EFANode * node,
                       std::vector<EFANode *> & master_nodes,
                       std::vector<double> & master_weights) const
{
  // Given a EFAnode, find the element edge or fragment edge that contains it
  // Return its master nodes and weights
  master_nodes.clear();
  master_weights.clear();
  bool masters_found = false;
  for (unsigned int i = 0; i < _num_edges; ++i) // check element exterior edges
  {
    if (_edges[i]->containsNode(node))
    {
      masters_found = _edges[i]->getNodeMasters(node, master_nodes, master_weights);
      if (masters_found)
        break;
      else
        EFAError("In getMasterInfo: cannot find master nodes in element edges");
    }
  }

  if (!masters_found) // check element interior embedded nodes
  {
    for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    {
      if (_interior_nodes[i]->getNode() == node)
      {
        std::vector<double> emb_xi(2, 0.0);
        emb_xi[0] = _interior_nodes[i]->getParametricCoordinates(0);
        emb_xi[1] = _interior_nodes[i]->getParametricCoordinates(1);
        for (unsigned int j = 0; j < _num_nodes; ++j)
        {
          master_nodes.push_back(_nodes[j]);
          double weight = 0.0;
          if (_num_nodes == 4)
            weight = Efa::linearQuadShape2D(j, emb_xi);
          else if (_num_nodes == 3)
            weight = Efa::linearTriShape2D(j, emb_xi);
          else
            EFAError("EFAface::getMasterInfo() only works for quad and tri EFAface");
          master_weights.push_back(weight);
        }
        masters_found = true;
        break;
      }
    }
  }
  return masters_found;
}

bool
EFAFace::getEdgeNodeParametricCoords(EFANode * node, std::vector<double> & xi_2d) const
{
  // get the parametric coords of a node in an edge
  bool edge_found = false;
  unsigned int edge_id;
  if (!isTriOrQuad())
    EFAError("EFAface::getEdgeNodeParaCoor can only work for quad or tri faces");

  for (unsigned int i = 0; i < _num_edges; ++i)
  {
    if (_edges[i]->containsNode(node))
    {
      edge_id = i;
      edge_found = true;
      break;
    }
  }
  if (edge_found)
  {
    double rel_dist = _edges[edge_id]->distanceFromNode1(node);
    double xi_1d = 2.0 * rel_dist - 1.0; // translate to [-1,1] parent coord syst
    mapParametricCoordsFrom1DTo2D(edge_id, xi_1d, xi_2d);
  }
  return edge_found;
}

bool
EFAFace::getFaceNodeParametricCoords(EFANode * node, std::vector<double> & xi_2d) const
{
  bool node_in_face = false;
  if (!isTriOrQuad())
    EFAError("EFAface::getFaceNodeParaCoor can only work for quad or tri faces");

  if (getEdgeNodeParametricCoords(node, xi_2d))
    node_in_face = true;
  else
  {
    for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    {
      if (_interior_nodes[i]->getNode() == node)
      {
        xi_2d.resize(2, 0.0);
        xi_2d[0] = _interior_nodes[i]->getParametricCoordinates(0);
        xi_2d[1] = _interior_nodes[i]->getParametricCoordinates(1);
        node_in_face = true;
        break;
      }
    } // i
  }
  return node_in_face;
}

unsigned int
EFAFace::numInteriorNodes() const
{
  return _interior_nodes.size();
}

void
EFAFace::createNodes()
{
  for (unsigned int i = 0; i < _edges.size(); ++i)
  {
    if (_edges[i] != NULL)
      _nodes[i] = _edges[i]->getNode(0);
    else
      EFAError("in EFAface::createNodes() _edges[i] does not exist");
  }
}

unsigned int
EFAFace::numEdges() const
{
  return _edges.size();
}

EFAEdge *
EFAFace::getEdge(unsigned int edge_id) const
{
  return _edges[edge_id];
}

void
EFAFace::setEdge(unsigned int edge_id, EFAEdge * new_edge)
{
  _edges[edge_id] = new_edge;
}

void
EFAFace::createEdges()
{
  for (unsigned int i = 0; i < _num_nodes; ++i)
  {
    unsigned int i_plus1(i < (_num_nodes - 1) ? i + 1 : 0);
    if (_nodes[i] != NULL && _nodes[i_plus1] != NULL)
    {
      EFAEdge * new_edge = new EFAEdge(_nodes[i], _nodes[i_plus1]);
      _edges[i] = new_edge;
    }
    else
      EFAError("EFAface::createEdges requires exsiting _nodes");
  }
}

void
EFAFace::combineTwoEdges(unsigned int edge_id1, unsigned int edge_id2)
{
  if (_edges[edge_id1]->containsNode(_edges[edge_id2]->getNode(0)) ||
      _edges[edge_id1]->containsNode(_edges[edge_id2]->getNode(1)))
  {
    // edge_id1 must precede edge_id2
    unsigned int edge1_next(edge_id1 < (_num_edges - 1) ? edge_id1 + 1 : 0);
    if (edge1_next != edge_id2) // if not, swap
    {
      unsigned int itmp = edge_id1;
      edge_id1 = edge_id2;
      edge_id2 = itmp;
    }

    // build new edge and delete old ones
    EFANode * new_node1 = _edges[edge_id1]->getNode(0);
    EFANode * emb_node = _edges[edge_id1]->getNode(1);
    EFANode * new_node2 = _edges[edge_id2]->getNode(1);
    if (emb_node != _edges[edge_id2]->getNode(0))
      EFAError("in combine_two_edges face edges are not correctly set up");

    EFAEdge * full_edge = new EFAEdge(new_node1, new_node2);
    full_edge->addIntersection(-1.0, emb_node, new_node1); // dummy intersection_x

    delete _edges[edge_id1];
    delete _edges[edge_id2];
    _edges[edge_id1] = full_edge;
    _edges.erase(_edges.begin() + edge_id2);

    // update face memeber variables
    _num_edges -= 1;
    _num_nodes -= 1;
    _nodes.resize(_num_nodes, NULL);
    for (unsigned int k = 0; k < _num_edges; ++k)
      _nodes[k] = _edges[k]->getNode(0);
  }
  else
    EFAError("two edges to be combined are not ajacent to each other");
}

void
EFAFace::sortEdges()
{
  std::vector<EFAEdge *> ordered_edges(_num_edges, NULL);
  ordered_edges[0] = _edges[0];
  for (unsigned int i = 1; i < _num_edges; ++i)
  {
    EFAEdge * last_edge = ordered_edges[i - 1];
    for (unsigned int j = 0; j < _num_edges; ++j)
    {
      if (!_edges[j]->equivalent(*last_edge) && _edges[j]->containsNode(last_edge->getNode(1)))
      {
        ordered_edges[i] = _edges[j];
        break;
      }
    } // j
  }   // i
  _edges = ordered_edges;
}

void
EFAFace::reverseEdges()
{
  // reverse the orientation of the face
  for (unsigned int i = 0; i < _edges.size(); ++i)
    _edges[i]->reverseNodes();
  std::reverse(_edges.begin(), _edges.end());
}

bool
EFAFace::isTriOrQuad() const
{
  if (_num_edges == 3 || _num_edges == 4)
    return true;
  else
    return false;
}

bool
EFAFace::equivalent(const EFAFace * other_face) const
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
    }   // i
    if (counter == _num_nodes)
      overlap = true;
  }
  return overlap;
}

bool
EFAFace::containsNode(const EFANode * node) const
{
  bool contains = false;
  for (unsigned int i = 0; i < _num_edges; ++i)
  {
    if (_edges[i]->containsNode(node))
    {
      contains = true;
      break;
    }
  }
  if (!contains)
  {
    for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    {
      if (_interior_nodes[i]->getNode() == node)
      {
        contains = true;
        break;
      }
    }
  }
  return contains;
}

bool
EFAFace::containsFace(const EFAFace * other_face) const
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

bool
EFAFace::ownsEdge(const EFAEdge * other_edge) const
{
  for (unsigned int i = 0; i < _edges.size(); ++i)
    if (_edges[i]->equivalent(*other_edge))
      return true;
  return false;
}

void
EFAFace::removeEmbeddedNode(EFANode * emb_node)
{
  for (unsigned int i = 0; i < _num_edges; ++i)
    _edges[i]->removeEmbeddedNode(emb_node);

  unsigned int index = 0;
  bool node_found = false;
  for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
  {
    if (_interior_nodes[i]->getNode() == emb_node)
    {
      node_found = true;
      index = i;
      break;
    }
  }
  if (node_found)
  {
    delete _interior_nodes[index];
    _interior_nodes.erase(_interior_nodes.begin() + index);
  }
}

std::vector<EFAFace *>
EFAFace::split() const
{
  std::vector<EFAFace *> new_faces;
  if (getNumCuts() > 0)
  {
    // construct a fragment from this face
    EFAFragment2D * frag_tmp = new EFAFragment2D(NULL, this);
    std::vector<EFAFragment2D *> new_frags_tmp = frag_tmp->split();

    // copy new_frags to new_faces
    for (unsigned int i = 0; i < new_frags_tmp.size(); ++i)
      new_faces.push_back(new EFAFace(new_frags_tmp[i]));

    // delete frag_tmp and new_frags
    delete frag_tmp;
    for (unsigned int i = 0; i < new_frags_tmp.size(); ++i)
      delete new_frags_tmp[i];
  }
  else
    new_faces.push_back(new EFAFace(*this));

  return new_faces;
}

EFAFace *
EFAFace::combineWithFace(const EFAFace * other_face) const
{
  // combine this face with another adjacent face
  EFAFace * new_face = NULL;
  if (isAdjacent(other_face))
  {
    unsigned int this_common_edge_id = adjacentCommonEdge(other_face);
    std::vector<EFANode *> common_nodes;
    common_nodes.push_back(_edges[this_common_edge_id]->getNode(0));
    common_nodes.push_back(_edges[this_common_edge_id]->getNode(1));

    unsigned int other_common_edge_id = other_face->adjacentCommonEdge(this);
    unsigned int new_n_nodes = _num_edges + other_face->_num_edges - 4;
    EFAFragment2D * new_frag = new EFAFragment2D(NULL, false, NULL); // temp fragment

    unsigned int this_edge_id0(this_common_edge_id > 0 ? this_common_edge_id - 1
                                                       : _num_edges - 1); // common_nodes[0]
    unsigned int this_edge_id1(this_common_edge_id < (_num_edges - 1) ? this_common_edge_id + 1
                                                                      : 0); // common_nodes[1]
    unsigned int other_edge_id0(
        other_common_edge_id < (other_face->_num_edges - 1) ? other_common_edge_id + 1 : 0);
    unsigned int other_edge_id1(other_common_edge_id > 0 ? other_common_edge_id - 1
                                                         : other_face->_num_edges - 1);

    EFAEdge * new_edge0 = new EFAEdge(_edges[this_edge_id0]->getNode(0),
                                      other_face->_edges[other_edge_id0]->getNode(1));
    new_edge0->addIntersection(
        -1.0, common_nodes[0], new_edge0->getNode(0)); // dummy intersection_x
    new_frag->addEdge(new_edge0);                      // common_nodes[0]'s edge

    unsigned int other_iedge(other_edge_id0 < (other_face->_num_edges - 1) ? other_edge_id0 + 1
                                                                           : 0);
    while (!other_face->_edges[other_iedge]->equivalent(*other_face->_edges[other_edge_id1]))
    {
      new_frag->addEdge(new EFAEdge(*other_face->_edges[other_iedge]));
      other_iedge += 1;
      if (other_iedge == other_face->_num_edges)
        other_iedge = 0;
    } // loop over other_face's edges

    EFAEdge * new_edge1 = new EFAEdge(other_face->_edges[other_edge_id1]->getNode(0),
                                      _edges[this_edge_id1]->getNode(1));
    new_edge1->addIntersection(
        -1.0, common_nodes[1], new_edge1->getNode(0)); // dummy intersection_x
    new_frag->addEdge(new_edge1);

    unsigned int this_iedge(this_edge_id1 < (_num_edges - 1) ? this_edge_id1 + 1 : 0);
    while (!_edges[this_iedge]->equivalent(*_edges[this_edge_id0])) // common_nodes[1]'s edge
    {
      new_frag->addEdge(new EFAEdge(*_edges[this_iedge]));
      this_iedge += 1;
      if (this_iedge == _num_edges)
        this_iedge = 0;
    } // loop over this_face's edges

    new_face = new EFAFace(new_frag);
    delete new_frag;
    if (new_face->numNodes() != new_n_nodes)
      EFAError("combine_with() sanity check fails");
  }
  return new_face;
}

void
EFAFace::resetEdgeIntersection(const EFAFace * ref_face)
{
  // set up correct edge intersections based on the reference face
  // the reference face must contain the edge of this face that is to be set up
  // the reference face must be an element face
  for (unsigned int j = 0; j < _num_edges; ++j)
  {
    if (_edges[j]->hasIntersection())
    {
      if (_edges[j]->numEmbeddedNodes() > 1)
        EFAError("frag face edge can only have 1 emb node at this point");

      EFANode * edge_node1 = _edges[j]->getNode(0);
      EFANode * edge_node2 = _edges[j]->getNode(1);
      EFANode * emb_node = _edges[j]->getEmbeddedNode(0);
      double inters_x = _edges[j]->getIntersection(0, edge_node1);
      if (std::abs(inters_x + 1.0) < 1.0e-4) // invalid intersection found
      {
        std::vector<double> node1_xi2d(2, 0.0);
        std::vector<double> node2_xi2d(2, 0.0);
        std::vector<double> emb_xi2d(2, 0.0);
        if (ref_face->getFaceNodeParametricCoords(edge_node1, node1_xi2d) &&
            ref_face->getFaceNodeParametricCoords(edge_node2, node2_xi2d) &&
            ref_face->getFaceNodeParametricCoords(emb_node, emb_xi2d))
        {
          // TODO: this is not correct for unstructured elements. Need a fix
          double dist2node1 =
              std::sqrt((emb_xi2d[0] - node1_xi2d[0]) * (emb_xi2d[0] - node1_xi2d[0]) +
                        (emb_xi2d[1] - node1_xi2d[1]) * (emb_xi2d[1] - node1_xi2d[1]));
          double full_dist =
              std::sqrt((node2_xi2d[0] - node1_xi2d[0]) * (node2_xi2d[0] - node1_xi2d[0]) +
                        (node2_xi2d[1] - node1_xi2d[1]) * (node2_xi2d[1] - node1_xi2d[1]));
          inters_x = dist2node1 / full_dist;
        }
        else
          EFAError("reference face does not contain the edge with invalid inters");
        _edges[j]->resetIntersection(inters_x, emb_node, edge_node1);
      }
    }
  }
}

unsigned int
EFAFace::getNumCuts() const
{
  unsigned int num_cuts = 0;
  for (unsigned int i = 0; i < _edges.size(); ++i)
  {
    if (_edges[i]->hasIntersection())
      num_cuts += _edges[i]->numEmbeddedNodes();
  }
  return num_cuts;
}

bool
EFAFace::hasIntersection() const
{
  if (getNumCuts() > 1)
    return true;
  else
    return false;
}

void
EFAFace::copyIntersection(const EFAFace & from_face)
{
  for (unsigned int i = 0; i < _edges.size(); ++i)
    if (from_face._edges[i]->hasIntersection())
      _edges[i]->copyIntersection(*from_face._edges[i], 0);

  if (from_face.numInteriorNodes() > 0)
    _interior_nodes = from_face._interior_nodes;
}

bool
EFAFace::isAdjacent(const EFAFace * other_face) const
{
  // two faces are adjacent if they only share one common edge
  unsigned int counter = 0;
  for (unsigned int i = 0; i < _num_edges; ++i)
    if (other_face->ownsEdge(_edges[i]))
      counter += 1;

  if (counter == 1)
    return true;
  else
    return false;
}

unsigned int
EFAFace::adjacentCommonEdge(const EFAFace * other_face) const
{
  if (isAdjacent(other_face))
  {
    for (unsigned int i = 0; i < _num_edges; ++i)
      if (other_face->ownsEdge(_edges[i]))
        return i;
  }
  EFAError("this face is not adjacent with other_face");
}

bool
EFAFace::hasSameOrientation(const EFAFace * other_face) const
{
  bool same_order = false;
  if (equivalent(other_face))
  {
    for (unsigned int i = 0; i < other_face->numNodes(); ++i)
    {
      if (other_face->_nodes[i] == _nodes[0])
      {
        unsigned int iplus1(i < (other_face->_num_nodes - 1) ? i + 1 : 0);
        if (other_face->_nodes[iplus1] == _nodes[1])
        {
          same_order = true;
          break;
        }
        else if (other_face->_nodes[iplus1] != _nodes[_num_nodes - 1])
          EFAError("two faces overlap but can't find correct common nodes");
      }
    }
  }
  else
    EFAWarning("in hasSameOrientation two faces does not overlap");
  return same_order;
}

EFAFaceNode *
EFAFace::getInteriorNode(unsigned int index) const
{
  return _interior_nodes[index];
}

void
EFAFace::mapParametricCoordsFrom1DTo2D(unsigned int edge_id,
                                       double xi_1d,
                                       std::vector<double> & xi_2d) const
{
  // given the 1D parent coord of a point in an 2D element edge, translate it to 2D para coords
  xi_2d.resize(2, 0.0);
  if (_num_edges == 4)
  {
    if (edge_id == 0)
    {
      xi_2d[0] = xi_1d;
      xi_2d[1] = -1.0;
    }
    else if (edge_id == 1)
    {
      xi_2d[0] = 1.0;
      xi_2d[1] = xi_1d;
    }
    else if (edge_id == 2)
    {
      xi_2d[0] = -xi_1d;
      xi_2d[1] = 1.0;
    }
    else if (edge_id == 3)
    {
      xi_2d[0] = -1.0;
      xi_2d[1] = -xi_1d;
    }
    else
      EFAError("edge_id out of bounds");
  }
  else if (_num_edges == 3)
  {
    if (edge_id == 0)
    {
      xi_2d[0] = 0.5 * (1.0 - xi_1d);
      xi_2d[1] = 0.5 * (1.0 + xi_1d);
    }
    else if (edge_id == 1)
    {
      xi_2d[0] = 0.0;
      xi_2d[1] = 0.5 * (1.0 - xi_1d);
    }
    else if (edge_id == 2)
    {
      xi_2d[0] = 0.5 * (1.0 + xi_1d);
      xi_2d[1] = 0.0;
    }
    else
      EFAError("edge_id out of bounds");
  }
  else
    EFAError("the EFAface::mapParametricCoordsFrom1DTo2D only works for quad and tri faces");
}
