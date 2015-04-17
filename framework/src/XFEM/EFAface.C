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

EFAface::EFAface(const EFAface & other_face):
  _num_nodes(other_face._num_nodes),
  _nodes(_num_nodes, NULL),
  _num_edges(_num_nodes),
  _edges(_num_edges, NULL)
{
  for (unsigned int k = 0; k < other_face._num_nodes; ++k)
  {
    _nodes[k] = other_face._nodes[k];
    _edges[k] = new EFAedge(*other_face._edges[k]);
  }
  for (unsigned int k = 0; k < other_face._interior_nodes.size(); ++k)
    _interior_nodes.push_back(new FaceNode(*other_face._interior_nodes[k]));
}


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
  return _nodes.size();
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
  for (unsigned int i = 0; i < _num_nodes; ++i)
  {
    if (_nodes[i] == old_node)
      _nodes[i] = new_node;
  }
  for (unsigned int i = 0; i < _edges.size(); ++i)
    _edges[i]->switchNode(new_node, old_node);
  for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    _interior_nodes[i]->switchNode(new_node, old_node);
}

bool
EFAface::getMasterInfo(EFAnode* node, std::vector<EFAnode*> &master_nodes,
                       std::vector<double> &master_weights) const
{
  //Given a EFAnode, find the element edge or fragment edge that contains it
  //Return its master nodes and weights
  master_nodes.clear();
  master_weights.clear();
  bool masters_found = false;
  for (unsigned int i = 0; i < _num_edges; ++i) // check element exterior edges
  {
    if (_edges[i]->containsNode(node))
    {
      masters_found = _edges[i]->getNodeMasters(node,master_nodes,master_weights);
      if (masters_found)
        break;
      else
        mooseError("In getMasterInfo: cannot find master nodes in element edges");
    }
  } // i

  if (!masters_found) // check element interior embedded nodes
  {
    for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    {
      if (_interior_nodes[i]->get_node() == node)
      {
        std::vector<double> emb_xi(2,0.0);
        emb_xi[0] = _interior_nodes[i]->get_para_coords(0);
        emb_xi[1] = _interior_nodes[i]->get_para_coords(1);
        for (unsigned int j = 0; j < _num_nodes; ++j)
        {
          master_nodes.push_back(_nodes[j]);
          double weight = 0.0;
          if (_num_nodes == 4)
            weight = linearQuadShape2D(j, emb_xi);
          else if (_num_nodes == 3)
            weight = linearTrigShape2D(j, emb_xi);
          else
            mooseError("EFAface::getMasterInfo() only works for quad and trig EFAface");
          master_weights.push_back(weight);
        } // j
        masters_found = true;
        break;
      }
    } // i
  }
  return masters_found;
}

bool
EFAface::getEdgeNodeParaCoor(EFAnode* node, std::vector<double> &xi_2d) const
{
  //get the parametric coords of a node in an element edge
  unsigned int edge_id = 99999;
  bool edge_found = false;
  if (!is_trig_quad())
    mooseError("EFAface::getEdgeNodeParaCoor can only work for quad or trig faces");

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
    double rel_dist = _edges[edge_id]->distance_from_node1(node);
    double xi_1d = 2.0*rel_dist - 1.0; // translate to [-1,1] parent coord syst
    mapParaCoorFrom1Dto2D(edge_id, xi_1d, xi_2d);
  }
  return edge_found;
}

bool
EFAface::getFaceNodeParaCoor(EFAnode* node, std::vector<double> &xi_2d) const
{
  bool node_in_face = false;
  if (!is_trig_quad())
    mooseError("EFAface::getFaceNodeParaCoor can only work for quad or trig faces");

  if (getEdgeNodeParaCoor(node, xi_2d))
    node_in_face = true;
  else
  {
    for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    {
      if (_interior_nodes[i]->get_node() == node)
      {
        xi_2d.resize(2,0.0);
        xi_2d[0] = _interior_nodes[i]->get_para_coords(0);
        xi_2d[1] = _interior_nodes[i]->get_para_coords(1);
        node_in_face = true;
        break;
      }
    } // i
  }
  return node_in_face;
}

unsigned int
EFAface::num_interior_nodes() const
{
  return _interior_nodes.size();
}

void
EFAface::createNodes()
{
  for (unsigned int i = 0; i < _edges.size(); ++i)
  {
    if (_edges[i] != NULL)
      _nodes[i] = _edges[i]->get_node(0);
    else
      mooseError("in EFAface::createNodes() _edges[i] does not exist");
  }
}

unsigned int
EFAface::num_edges() const
{
  return _edges.size();
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

void
EFAface::combine_two_edges(unsigned int edge_id1, unsigned int edge_id2)
{
  if (_edges[edge_id1]->containsNode(_edges[edge_id2]->get_node(0)) ||
      _edges[edge_id1]->containsNode(_edges[edge_id2]->get_node(1)))
  {
    // edge_id1 must precede edge_id2
    unsigned int edge1_next(edge_id1 < (_num_edges-1) ? edge_id1+1 : 0);
    if (edge1_next != edge_id2) // if not, swap
    {
      unsigned int itmp = edge_id1;
      edge_id1 = edge_id2;
      edge_id2 = itmp;
    }

    // build new edge and delete old ones
    EFAnode* new_node1 = _edges[edge_id1]->get_node(0);
    EFAnode* emb_node = _edges[edge_id1]->get_node(1);
    EFAnode* new_node2 = _edges[edge_id2]->get_node(1);
    if (emb_node != _edges[edge_id2]->get_node(0))
      mooseError("in combine_two_edges face edges are not correctly set up");

    EFAedge* full_edge = new EFAedge(new_node1, new_node2);
    full_edge->add_intersection(-1.0, emb_node, new_node1); // dummy intersection_x

    delete _edges[edge_id1];
    delete _edges[edge_id2];
    _edges[edge_id1] = full_edge;
    _edges.erase(_edges.begin() + edge_id2);

    // update face memeber variables
    _num_edges -= 1;
    _num_nodes -= 1;
    _nodes.resize(_num_nodes, NULL);
    for (unsigned int k = 0; k < _num_edges; ++k)
      _nodes[k] = _edges[k]->get_node(0);
  }
  else
    mooseError("two edges to be combined are not ajacent to each other");
}

void
EFAface::sort_edges()
{
  std::vector<EFAedge*> ordered_edges(_num_edges, NULL);
  ordered_edges[0] = _edges[0];
  for (unsigned int i = 1; i < _num_edges; ++i)
  {
    EFAedge* last_edge = ordered_edges[i-1];
    for (unsigned int j = 0; j < _num_edges; ++j)
    {
      if (!_edges[j]->equivalent(*last_edge) &&
          _edges[j]->containsNode(last_edge->get_node(1)))
      {
        ordered_edges[i] = _edges[j];
        break;
      }
    } // j
  } // i
  _edges = ordered_edges;
}

void
EFAface::reverse_edges()
{
  // reverse the orientation of the face
  for (unsigned int i = 0; i < _edges.size(); ++i)
    _edges[i]->reverse_nodes();
  std::reverse(_edges.begin(), _edges.end());
}

bool
EFAface::is_trig_quad() const
{
  if (_num_edges == 3 || _num_edges == 4)
    return true;
  else
    return false;
}

bool
EFAface::equivalent(const EFAface* other_face) const
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
  for (unsigned int i = 0; i < _num_edges; ++i)
  {
    if (_edges[i]->containsNode(node))
    {
      contain = true;
      break;
    }
  } // i
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

bool
EFAface::doesOwnEdge(const EFAedge* other_edge) const
{
  for (unsigned int i = 0; i < _edges.size(); ++i)
    if (_edges[i]->equivalent(*other_edge))
      return true;
  return false;
}

void
EFAface::remove_embedded_node(EFAnode* emb_node)
{
  for (unsigned int i = 0; i < _num_edges; ++i)
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
  {
    delete _interior_nodes[index];
    _interior_nodes.erase(_interior_nodes.begin() + index);
  }
}

std::vector<EFAface*>
EFAface::split() const
{
  std::vector<EFAface*> new_faces;
  if (get_num_cuts() > 0)
  {
    // contruct a fragment from this face
    EFAfragment2D* frag_tmp = new EFAfragment2D(NULL, this);
    std::vector<EFAfragment2D*> new_frags_tmp = frag_tmp->split();

    // copy new_frags to new_faces
    for (unsigned int i = 0; i < new_frags_tmp.size(); ++i)
      new_faces.push_back(new EFAface(new_frags_tmp[i]));

    // delete frag_tmp and new_frags
    delete frag_tmp;
    for (unsigned int i = 0; i < new_frags_tmp.size(); ++i)
      delete new_frags_tmp[i];
  }
  else
    new_faces.push_back(new EFAface(*this));

  return new_faces;
}

EFAface*
EFAface::combine_with(const EFAface* other_face) const
{
  // combine this face with another adjacent face
  EFAface* new_face = NULL;
  if (isAdjacent(other_face))
  {
    unsigned int this_common_edge_id = adjacentCommonEdge(other_face);
    std::vector<EFAnode*> common_nodes;
    common_nodes.push_back(_edges[this_common_edge_id]->get_node(0));
    common_nodes.push_back(_edges[this_common_edge_id]->get_node(1));

    unsigned int other_common_edge_id = other_face->adjacentCommonEdge(this);
    unsigned int new_n_nodes = _num_edges + other_face->_num_edges - 4;
    EFAfragment2D* new_frag = new EFAfragment2D(NULL, false, NULL); // temp fragment

    unsigned int this_edge_id0(this_common_edge_id>0 ? this_common_edge_id-1 : _num_edges-1); // common_nodes[0]
    unsigned int this_edge_id1(this_common_edge_id<(_num_edges-1) ? this_common_edge_id+1 : 0); // common_nodes[1]
    unsigned int other_edge_id0(other_common_edge_id<(other_face->_num_edges-1) ? other_common_edge_id+1 : 0);
    unsigned int other_edge_id1(other_common_edge_id>0 ? other_common_edge_id-1 : other_face->_num_edges-1);

    EFAedge* new_edge0 = new EFAedge(_edges[this_edge_id0]->get_node(0), other_face->_edges[other_edge_id0]->get_node(1));
    new_edge0->add_intersection(-1.0, common_nodes[0], new_edge0->get_node(0)); // dummy intersection_x
    new_frag->add_edge(new_edge0); // common_nodes[0]'s edge

    unsigned int other_iedge(other_edge_id0<(other_face->_num_edges-1) ? other_edge_id0+1 : 0);
    while (!other_face->_edges[other_iedge]->equivalent(*other_face->_edges[other_edge_id1]))
    {
      new_frag->add_edge(new EFAedge(*other_face->_edges[other_iedge]));
      other_iedge += 1;
      if (other_iedge == other_face->_num_edges)
        other_iedge = 0;
    } // loop over other_face's edges

    EFAedge* new_edge1 = new EFAedge(other_face->_edges[other_edge_id1]->get_node(0), _edges[this_edge_id1]->get_node(1));
    new_edge1->add_intersection(-1.0, common_nodes[1], new_edge1->get_node(0)); // dummy intersection_x
    new_frag->add_edge(new_edge1);

    unsigned int this_iedge(this_edge_id1<(_num_edges-1) ? this_edge_id1+1 : 0);
    while (!_edges[this_iedge]->equivalent(*_edges[this_edge_id0])) // common_nodes[1]'s edge
    {
      new_frag->add_edge(new EFAedge(*_edges[this_iedge]));
      this_iedge += 1;
      if (this_iedge == _num_edges)
        this_iedge = 0;
    } // loop over this_face's edges

    new_face = new EFAface(new_frag);
    delete new_frag;
    if (new_face->num_nodes() != new_n_nodes)
      mooseError("combine_with() sanity check fails");
  }
  return new_face;
}

void
EFAface::reset_edge_intersection(const EFAface* ref_face)
{
  // set up correct edge intersections based on the reference face
  // the reference face must contain the edge of this face that is to be set up
  // the reference face must be an element face
  for (unsigned int j = 0; j < _num_edges; ++j)
  {
    if (_edges[j]->has_intersection())
    {
      if (_edges[j]->num_embedded_nodes() > 1)
        mooseError("frag face edge can only have 1 emb node at this point");

      EFAnode* edge_node1 = _edges[j]->get_node(0);
      EFAnode* edge_node2 = _edges[j]->get_node(1);
      EFAnode* emb_node = _edges[j]->get_embedded_node(0);
      double inters_x = _edges[j]->get_intersection(0, edge_node1);
      if (std::abs(inters_x + 1.0) < 1.0e-4) // invalid intersection found
      {
        std::vector<double> node1_xi2d(2,0.0);
        std::vector<double> node2_xi2d(2,0.0);
        std::vector<double> emb_xi2d(2,0.0);
        if (ref_face->getFaceNodeParaCoor(edge_node1, node1_xi2d) &&
            ref_face->getFaceNodeParaCoor(edge_node2, node2_xi2d) &&
            ref_face->getFaceNodeParaCoor(emb_node, emb_xi2d))
        {
          // TODO: this is not corrent for unstructured elements. Need a fix
          double dist2node1 = std::sqrt((emb_xi2d[0]-node1_xi2d[0])*(emb_xi2d[0]-node1_xi2d[0])
                                      + (emb_xi2d[1]-node1_xi2d[1])*(emb_xi2d[1]-node1_xi2d[1]));
          double full_dist = std::sqrt((node2_xi2d[0]-node1_xi2d[0])*(node2_xi2d[0]-node1_xi2d[0])
                                     + (node2_xi2d[1]-node1_xi2d[1])*(node2_xi2d[1]-node1_xi2d[1]));
          inters_x = dist2node1/full_dist;
        }
        else
          mooseError("reference face does not contain the edge with invalid inters");
        _edges[j]->reset_intersection(inters_x, emb_node, edge_node1);
      }
    }
  } // j
}

unsigned int
EFAface::get_num_cuts() const
{
  unsigned int num_cuts = 0;
  for (unsigned int i = 0; i < _edges.size(); ++i)
  {
    if (_edges[i]->has_intersection())
      num_cuts += _edges[i]->num_embedded_nodes();
  }
  return num_cuts;
}

bool
EFAface::has_intersection() const
{
  if (get_num_cuts() > 1)
    return true;
  else
    return false;
}

void
EFAface::copy_intersection(const EFAface &from_face)
{
  for (unsigned int i = 0; i < _edges.size(); ++i)
    if (from_face._edges[i]->has_intersection())
      _edges[i]->copy_intersection(*from_face._edges[i], 0);

  if (from_face.num_interior_nodes() > 0)
    _interior_nodes = from_face._interior_nodes;
}

bool
EFAface::isAdjacent(const EFAface* other_face) const
{
  // two faces are ajacent if they only share one common edge
  unsigned int counter = 0;
  for (unsigned int i = 0; i < _num_edges; ++i)
    if (other_face->doesOwnEdge(_edges[i]))
      counter += 1;

  if (counter == 1)
    return true;
  else
    return false;
}

unsigned int
EFAface::adjacentCommonEdge(const EFAface* other_face) const
{
  if (isAdjacent(other_face))
  {
    for (unsigned int i = 0; i < _num_edges; ++i)
      if (other_face->doesOwnEdge(_edges[i]))
        return i;
  }
  else
    mooseError("this face is not adjacent with other_face");
  return 99999;
}

bool
EFAface::is_same_orientation(const EFAface* other_face) const
{
  bool same_order = false;
  if (equivalent(other_face))
  {
    for (unsigned int i = 0; i < other_face->num_nodes(); ++i)
    {
      if (other_face->_nodes[i] == _nodes[0])
      {
        unsigned int iplus1(i < (other_face->_num_nodes-1) ? i+1 : 0);
        if (other_face->_nodes[iplus1] == _nodes[1])
        {
          same_order = true;
          break;
        }
        else if (other_face->_nodes[iplus1] != _nodes[_num_nodes-1])
          mooseError("two faces overlap but can't find correct common nodes");
      }
    } // i
  }
  else
    std::cout << "WARNING: in is_same_orientation two faces does not overlap" << std::endl;
  return same_order;
}

FaceNode*
EFAface::get_interior_node(unsigned int index) const
{
  return _interior_nodes[index];
}

void
EFAface::mapParaCoorFrom1Dto2D(unsigned int edge_id, double xi_1d,
                               std::vector<double> &xi_2d) const
{
  // given the 1D parent coord of a point in an 2D element edge, translate it to 2D para coords
  xi_2d.resize(2,0.0);
  if (_num_edges == 4)
  {
    if(edge_id == 0)
    {
      xi_2d[0] = xi_1d;
      xi_2d[1] = -1.0;
    }
    else if(edge_id == 1)
    {
      xi_2d[0] = 1.0;
      xi_2d[1] = xi_1d;
    }
    else if(edge_id == 2)
    {
      xi_2d[0] = -xi_1d;
      xi_2d[1] = 1.0;
    }
    else if(edge_id == 3)
    {
      xi_2d[0] = -1.0;
      xi_2d[1] = -xi_1d;
    }
    else
      mooseError("edge_id out of bounds");
  }
  else if (_num_edges == 3)
  {
    if (edge_id == 0)
    {
      xi_2d[0] = 0.5*(1.0 - xi_1d);
      xi_2d[1] = 0.5*(1.0 + xi_1d);
    }
    else if (edge_id == 1)
    {
      xi_2d[0] = 0.0;
      xi_2d[1] = 0.5*(1.0 - xi_1d);
    }
    else if(edge_id == 2)
    {
      xi_2d[0] = 0.5*(1.0 + xi_1d);
      xi_2d[1] = 0.0;
    }
    else
      mooseError("edge_id out of bounds");
  }
  else
    mooseError("the EFAface::mapParaCoorFrom1Dto2D only works for quad and trig faces");
}

