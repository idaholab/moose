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

EFAelement::EFAelement(unsigned int eid):
  _id(eid),
  _num_nodes(4),
  _num_edges(4),
  _nodes(_num_nodes,NULL),
  _edges(_num_edges,NULL),
  _parent(NULL),
  _edge_neighbors(_num_edges,std::vector<EFAelement*>(1,NULL)),
  _crack_tip_split_element(false)
{}

EFAelement::EFAelement(const EFAelement* from_elem, bool convert_to_local):
  _id(from_elem->_id),
  _num_nodes(from_elem->_num_nodes),
  _num_edges(from_elem->_num_edges),
  _nodes(_num_nodes,NULL),
  _edges(_num_edges,NULL),
  _parent(NULL),
  _edge_neighbors(_num_edges,std::vector<EFAelement*>(1,NULL)),
  _crack_tip_split_element(from_elem->_crack_tip_split_element)
{
  if (convert_to_local)
  {
    // build local nodes from global nodes
    for (unsigned int i = 0; i < _num_nodes; ++i)
    {
      if (from_elem->_nodes[i]->category() == N_CATEGORY_PERMANENT ||
          from_elem->_nodes[i]->category() == N_CATEGORY_TEMP)
      {
        _nodes[i] = from_elem->create_local_node_from_global_node(from_elem->_nodes[i]);
        _local_nodes.push_back(_nodes[i]);
      }
      else
        mooseError("In EFAelement "<<from_elem->id()<<" the copy constructor must have from_elem w/ global nodes. node: "
                    << i << " category: "<<from_elem->_nodes[i]->category());
    }

    // copy edges, fragments and interior nodes from from_elem
    for (unsigned int i = 0; i < _num_edges; ++i)
      _edges[i] = new EFAedge(*from_elem->_edges[i]);
    for (unsigned int i = 0; i < from_elem->_fragments.size(); ++i)
      _fragments.push_back(new EFAfragment(NULL, true, from_elem, i));
    for (unsigned int i = 0; i < from_elem->_interior_nodes.size(); ++i)
      _interior_nodes.push_back(new FaceNode(*from_elem->_interior_nodes[i]));

    // replace all global nodes with local nodes
    for (unsigned int i = 0; i < _num_nodes; ++i)
    {
      if (_nodes[i]->category() == N_CATEGORY_LOCAL_INDEX)
        switchNode(_nodes[i], from_elem->_nodes[i], false);//when save to _cut_elem_map, the EFAelement is not a child of any parent
      else
        mooseError("In EFAelement copy constructor this elem's nodes must be local");
    } 
  }
  else
    mooseError("this EFAelement constructor only converts global nodes to local nodes");
}

EFAelement::~EFAelement()
{
  for (unsigned int i = 0; i < _fragments.size(); ++i)
  {
    if (_fragments[i])
    {
      delete _fragments[i];
      _fragments[i] = NULL;
    }
  }
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
  for (unsigned int i = 0; i < _local_nodes.size(); ++i)
  {
    if (_local_nodes[i])
    {
      delete _local_nodes[i];
      _local_nodes[i] = NULL;
    }
  }
}

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
EFAelement::display_nodes()
{
  std::cout << "***** display nodes for element " << _id << " *****" << std::endl;
  for (unsigned int i = 0; i < _num_nodes; ++i)
    std::cout << "addr " << _nodes[i] << ", ID " << _nodes[i]->id() << ", category " << _nodes[i]->category() << std::endl;
}

void
EFAelement::switchNode(EFAnode *new_node,
                       EFAnode *old_node,
                       bool descend_to_parent)
{
  for (unsigned int i = 0; i < _num_nodes; ++i)
  {
    if (_nodes[i] == old_node)
      _nodes[i] = new_node;
  }
  for (unsigned int i = 0; i < _fragments.size(); ++i)
    _fragments[i]->switchNode(new_node, old_node);

  for (unsigned int i = 0; i < _edges.size(); ++i)
    _edges[i]->switchNode(new_node, old_node);

  if (_parent && descend_to_parent)
  {
    _parent->switchNode(new_node,old_node,false);
    for (unsigned int i = 0; i < _parent->num_edges(); ++i)
    {
      for (unsigned int j = 0; j < _parent->num_edge_neighbors(i); ++j)
      {
        EFAelement* edge_neighbor = _parent->get_edge_neighbor(i,j);
        for (unsigned int k = 0; k < edge_neighbor->num_children(); ++k)
        {
          edge_neighbor->get_child(k)->switchNode(new_node,old_node,false);
        }
      } // j
    } // i
  }
}

void
EFAelement::switchEmbeddedNode(EFAnode *new_emb_node,
                               EFAnode *old_emb_node)
{
  for (unsigned int i = 0; i < _num_edges; ++i)
    _edges[i]->switchNode(new_emb_node, old_emb_node);
  for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    _interior_nodes[i]->switchNode(new_emb_node, old_emb_node);
  for (unsigned int i = 0; i < _fragments.size(); ++i)
    _fragments[i]->switchNode(new_emb_node, old_emb_node);
}


void
EFAelement::get_nodes_on_edge(unsigned int edge_idx,
                              std::vector<EFAnode*> &edge_nodes)
{
  edge_nodes.push_back(_edges[edge_idx]->get_node(0));
  edge_nodes.push_back(_edges[edge_idx]->get_node(1));
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

void
EFAelement::getMasterInfo(EFAnode* node, std::vector<EFAnode*> &master_nodes,
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
  }

  if (!masters_found) // check element interior embedded nodes
  {
    for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    {
      if (_interior_nodes[i]->get_node() == node)
      {
        double node_xi[4][2] = {{-1.0,-1.0},{1.0,-1.0},{1.0,1.0},{-1.0,1.0}};
        double emb_xi  = _interior_nodes[i]->get_para_coords(0);
        double emb_eta = _interior_nodes[i]->get_para_coords(1);
        for (unsigned int j = 0; j < _num_nodes; ++j)
        {
          master_nodes.push_back(_nodes[j]);
          double weight = 0.0;
          if (_num_nodes == 4)
            weight = 0.25*(1+node_xi[j][0]*emb_xi)*(1+node_xi[j][1]*emb_eta);
          else
            mooseError("getMasterInfo only works for quad element now");
          master_weights.push_back(weight);
        }
        masters_found = true;
        break;
      }
    }
  }

  if (!masters_found)
    mooseError("In EFAelement::getMaterInfo, cannot find the given EFAnode");
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

bool
EFAelement::getEmbeddedNodeParaCoor(EFAnode* embedded_node, std::vector<double> &para_coor)
{
  //get the parametric coords of an embedded node
  unsigned int edge_id = 99999;
  bool edge_found = false;
  for (unsigned int i = 0; i < _num_edges; ++i)
  {
    if (_edges[i]->get_embedded_node() == embedded_node)
    {
      edge_id = i;
      edge_found = true;
      break;
    }
  }
  if (edge_found)
  {
    EFAnode* edge_node1 = _edges[edge_id]->get_node(0);
    double xi_1d = 2.0*_edges[edge_id]->get_intersection(edge_node1) -1.0;
    mapParaCoorFrom1Dto2D(edge_id, xi_1d, para_coor);
  }
  return edge_found;
}

void
EFAelement::add_interior_node(FaceNode* face_node)
{
  _interior_nodes.push_back(face_node);
}

unsigned int
EFAelement::getNumInteriorNodes() const
{
  return _interior_nodes.size();
}

FaceNode*
EFAelement::get_interior_node(unsigned int interior_node_id) const
{
  if (interior_node_id < _interior_nodes.size())
    return _interior_nodes[interior_node_id];
  else
    mooseError("interior_node_id out of bounds");
}

unsigned int
EFAelement::num_edges() const
{
  return _num_edges;
}

void
EFAelement::set_edge(unsigned int edge_id, EFAedge* edge)
{
  _edges[edge_id] = edge;
}

void
EFAelement::createEdges()
{
  for (unsigned int i = 0; i < _num_nodes; ++i)
  {
    unsigned int i_plus1(i < (_num_nodes-1) ? i+1 : 0);
    EFAedge * new_edge = new EFAedge(_nodes[i], _nodes[i_plus1]);
    _edges[i] = new_edge;
  }
}

EFAedge*
EFAelement::get_edge(unsigned int edge_id) const
{
  return _edges[edge_id];
}

void
EFAelement::set_parent(EFAelement* parent)
{
  _parent = parent;
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

unsigned int
EFAelement::num_frags() const
{
  return _fragments.size();
}

EFAfragment*
EFAelement::get_fragment(unsigned int frag_id) const
{
  if (frag_id < _fragments.size())
    return _fragments[frag_id];
  else
    mooseError("frag_id out of bounds");
}

void
EFAelement::add_fragment(EFAfragment* frag)
{
  _fragments.push_back(frag);
}

EFAedge*
EFAelement::get_frag_edge(unsigned int frag_id, unsigned int edge_id) const
{
  if (frag_id < _fragments.size())
    return _fragments[frag_id]->get_edge(edge_id);
  else
    mooseError("frag_id out of bounds in get_frag_edge()");
}

bool
EFAelement::is_partial() const
{
  bool partial = false;
  if (_fragments.size() > 0)
  {
    for (unsigned int i = 0; i < _num_nodes; ++i)
    {
      bool node_in_frag = false;
      for (unsigned int j = 0; j < _fragments.size(); ++j)
      {
        if (_fragments[j]->containsNode(_nodes[i]))
        {
          node_in_frag = true;
          break;
        }
      } // j
      if (!node_in_frag)
      {
        partial = true;
        break;
      }
    } // i
  }
  return partial;
}

void
EFAelement::get_non_physical_nodes(std::set<EFAnode*> &non_physical_nodes)
{
  //Any nodes that don't belong to any fragment are non-physical
  //First add all nodes in the element to the set
  for (unsigned int i = 0; i < _nodes.size(); ++i)
    non_physical_nodes.insert(_nodes[i]);

  //Now delete any nodes that are contained in fragments
  std::set<EFAnode*>::iterator sit;
  for (sit = non_physical_nodes.begin(); sit != non_physical_nodes.end(); ++sit)
  {
    for (unsigned int i=0; i<_fragments.size(); ++i)
    {
      if (_fragments[i]->containsNode(*sit))
      {
        non_physical_nodes.erase(sit);
        break;
      }
    }
  }
}

std::set<EFAnode*>
EFAelement::getPhantomNodeOnEdge(unsigned int edge_id)
{
  std::set<EFAnode*> phantom_nodes;
  if (_fragments.size() > 0)
  {
    for (unsigned int j = 0; j < 2; ++j) // loop ove 2 edge nodes
    {
      bool node_in_frag = false;
      for (unsigned int k = 0; k < _fragments.size(); ++k)
      {
        if (_fragments[k]->containsNode(_edges[edge_id]->get_node(j)))
        {
          node_in_frag = true;
          break;
        }
      }
      if (!node_in_frag)
        phantom_nodes.insert(_edges[edge_id]->get_node(j));
    } // j
  }
  return phantom_nodes;
}

bool
EFAelement::getFragmentEdgeID(unsigned int elem_edge_id, unsigned int &frag_id, 
                              unsigned int &frag_edge_id)
{
  // find the fragment edge that coincides with the given element edge
  bool frag_edge_found = false;
  frag_id = 99999;
  frag_edge_id = 99999;
  if (_fragments.size() > 0)
  {
    for (unsigned int i = 0; i < _fragments.size(); ++i)
    {
      for (unsigned int j = 0; j < _fragments[i]->num_edges(); ++j)
      {
        if (_edges[elem_edge_id]->isOverlapping(*_fragments[i]->get_edge(j)))
        {
          frag_id = i;
          frag_edge_id = j;
          frag_edge_found = true;
          break;
        }
      }
      if (frag_edge_found) break;
    }
  }
  return frag_edge_found;
}

bool
EFAelement::is_edge_phantom(unsigned int edge_id)
{
  bool is_phantom = false;
  if (_fragments.size() > 0)
  {
    if (_fragments.size() != 1)
      mooseError("in is_edge_phantom() an element has more than 1 fragment");
    EFAnode * edge_node1 = _edges[edge_id]->get_node(0);
    EFAnode * edge_node2 = _edges[edge_id]->get_node(1);
    if ((!_fragments[0]->containsNode(edge_node1)) &&
        (!_fragments[0]->containsNode(edge_node2)))
      is_phantom =true;
  }
  return is_phantom;
}

bool
EFAelement::overlays_elem(const EFAelement* other_elem) const
{
  bool overlays = false;
  std::vector<EFAnode*> common_nodes = get_common_nodes(other_elem);

  //Find indices of common nodes
  if (common_nodes.size() == 2)
  {
    std::vector<EFAnode*> common_nodes_vec(common_nodes.begin(),common_nodes.end());

    unsigned int e1n1idx = _num_nodes+1;
    unsigned int e1n2idx = _num_nodes+1;
    for (unsigned int i=0; i<_num_nodes; ++i)
    {
      if (_nodes[i] == common_nodes_vec[0])
      {
        e1n1idx = i;
      }
      else if (_nodes[i] == common_nodes_vec[1])
      {
        e1n2idx = i;
      }
    }
    if (e1n1idx > _num_nodes || e1n2idx > _num_nodes)
      mooseError("in overlays_elem() couldn't find common node");

    bool e1ascend = false;
    unsigned int e1n1idx_plus1(e1n1idx<(_num_nodes-1) ? e1n1idx+1 : 0);
    unsigned int e1n1idx_minus1(e1n1idx>0 ? e1n1idx-1 : _num_nodes-1);
    if (e1n2idx == e1n1idx_plus1)
    {
      e1ascend = true;
    }
    else
    {
      if (e1n2idx != e1n1idx_minus1)
        mooseError("in overlays_elem() common nodes must be adjacent to each other");
    }

    unsigned int e2n1idx = other_elem->_num_nodes+1;
    unsigned int e2n2idx = other_elem->_num_nodes+1;
    for (unsigned int i=0; i<other_elem->_num_nodes; ++i)
    {
      if (other_elem->_nodes[i] == common_nodes_vec[0])
      {
        e2n1idx = i;
      }
      else if (other_elem->_nodes[i] == common_nodes_vec[1])
      {
        e2n2idx = i;
      }
    }
    if (e2n1idx > other_elem->_num_nodes || e2n2idx > other_elem->_num_nodes)
      mooseError("in overlays_elem() couldn't find common node");

    bool e2ascend = false;
    unsigned int e2n1idx_plus1(e2n1idx<(_num_nodes-1) ? e2n1idx+1 : 0);
    unsigned int e2n1idx_minus1(e2n1idx>0 ? e2n1idx-1 : _num_nodes-1);
    if (e2n2idx == e2n1idx_plus1)
    {
      e2ascend = true;
    }
    else
    {
      if (e2n2idx != e2n1idx_minus1)
        mooseError("in overlays_elem() common nodes must be adjacent to each other");
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
EFAelement::overlays_elem(EFAnode* other_edge_node1, EFAnode* other_edge_node2) const
{
  bool overlays = false;
  bool found_common_edge = false;
  for (unsigned int edge_iter = 0; edge_iter < _num_edges; ++edge_iter)
  {
    if (_nodes[edge_iter] == other_edge_node1)
    {
      unsigned int edge_iter_plus1(edge_iter<(_num_edges-1) ? edge_iter+1 : 0);
      if (_nodes[edge_iter_plus1] == other_edge_node2)
      {
        overlays = true;
      }
      else
      {
        unsigned int edge_iter_minus1(edge_iter>0 ? edge_iter-1 : _num_edges-1);
        if (_nodes[edge_iter_minus1] != other_edge_node2)
          mooseError("in overlays_elem() element does not have an edge that contains the provided nodes");
      }
      found_common_edge = true;
      break;
    }
  }
  if (!found_common_edge)
    mooseError("in overlays_elem() element does not have an edge that contains the provided nodes");

  return overlays;
}

unsigned int
EFAelement::get_neighbor_index(EFAelement * neighbor_elem)
{
  for (unsigned int i=0; i<_num_edges; ++i)
  {
    for (unsigned int j=0; j<_edge_neighbors[i].size(); ++j)
    {
      if (_edge_neighbors[i][j] == neighbor_elem)
        return i;
    }
  }
  mooseError("in get_neighbor_index() element: "<<_id<<" does not have neighbor: "<<neighbor_elem->id());
}

unsigned int
EFAelement::edge_index_in_neighbor(EFAelement* neighbor_elem)
{
  // identify the common cut edge
  bool found(false);
  unsigned int edge_id = 99999;
  for (unsigned int m = 0; m < neighbor_elem->num_edges(); ++m)
  {
    for (unsigned int n = 0; n < neighbor_elem->num_edge_neighbors(m); ++n)
    {
      if (neighbor_elem->get_edge_neighbor(m,n) == this)
      {
        edge_id = m;
        found = true;
        break;
      }
    } // n
    if (found) break;
  } // m
  if (!found)
    mooseError("Element: "<<_id<<" is not a neighbor of its neighbor element: " << neighbor_elem->id());

  return edge_id;
}

unsigned int
EFAelement::num_edge_neighbors(unsigned int edge_id) const
{
  unsigned int num_neighbors = 0;
  if (_edge_neighbors[edge_id][0])
    num_neighbors = _edge_neighbors[edge_id].size();
  return num_neighbors;
}

EFAelement*
EFAelement::get_edge_neighbor(unsigned int edge_id, unsigned int neighbor_id) const
{
  if (_edge_neighbors[edge_id][0] != NULL && neighbor_id < _edge_neighbors[edge_id].size())
    return _edge_neighbors[edge_id][neighbor_id];
  else
    mooseError("edge neighbor does not exist");
}

void
EFAelement::clear_neighbors()
{
  for (unsigned int edge_iter = 0; edge_iter < _num_edges; ++edge_iter)
    _edge_neighbors[edge_iter] = std::vector<EFAelement*>(1,NULL);
}

void
EFAelement::setup_neighbors(std::map< EFAnode*, std::set<EFAelement*> > &InverseConnectivityMap)
{
  std::set<EFAelement*> neighbor_elements;
  for (unsigned int inode = 0; inode < _num_nodes; ++inode)
  {
    std::set<EFAelement*> this_node_connected_elems = InverseConnectivityMap[_nodes[inode]];
    neighbor_elements.insert(this_node_connected_elems.begin(), this_node_connected_elems.end());
  }

  std::set<EFAelement*>::iterator eit2;
  for (eit2 = neighbor_elements.begin(); eit2 != neighbor_elements.end(); ++eit2)
  {
    if (*eit2 != this)
    {
      EFAelement *neigh_elem = *eit2;
      std::vector<EFAnode*> common_nodes = get_common_nodes(neigh_elem);

      if (common_nodes.size() >= 2)
      {
        bool found_edge = false;
        for (unsigned int edge_iter = 0; edge_iter < _num_edges; ++edge_iter)
        {
          std::set<EFAnode*> edge_nodes;
          EFAnode* edge_node1 = _edges[edge_iter]->get_node(0);
          EFAnode* edge_node2 = _edges[edge_iter]->get_node(1);
          edge_nodes.insert(edge_node1);
          edge_nodes.insert(edge_node2);
          bool is_edge_neighbor = false;

          //Must share nodes on this edge
          if (num_common_elems(edge_nodes, common_nodes) == 2)
          {
            //Must not overlay
            if (!neigh_elem->overlays_elem(edge_node1,edge_node2))
            {
              //Fragments must match up.
              if ((_fragments.size() > 1) ||
                  (neigh_elem->num_frags() > 1))
              {
                mooseError("in updateEdgeNeighbors: Cannot have more than 1 fragment");
              }
              else if ((_fragments.size() == 1) &&
                       (neigh_elem->num_frags() == 1))
              {
                if (_fragments[0]->isConnected(*neigh_elem->get_fragment(0)))
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
            if (_edge_neighbors[edge_iter][0])
            {
              if (_edge_neighbors[edge_iter].size() > 1)
              {
                std::cout<<"Neighbor: "<<neigh_elem->id()<<std::endl;
                mooseError("Element "<<_id<<" already has 2 edge neighbors: "
                                 <<_edge_neighbors[edge_iter][0]->id()<<" "
                                 <<_edge_neighbors[edge_iter][1]->id());
              }
              _edge_neighbors[edge_iter].push_back(neigh_elem);
            }
            else
            {
              _edge_neighbors[edge_iter][0] = neigh_elem;
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
  } // eit2
}

void
EFAelement::neighbor_sanity_check()
{
  for (unsigned int edge_iter = 0; edge_iter < _num_edges; ++edge_iter)
  {
    for (unsigned int en_iter = 0; en_iter < _edge_neighbors[edge_iter].size(); ++en_iter)
    {
      EFAelement* neigh_elem = _edge_neighbors[edge_iter][en_iter];
      if (neigh_elem != NULL)
      {
        bool found_neighbor = false;
        for (unsigned int edge_iter2 = 0; edge_iter2 < neigh_elem->num_edges(); ++edge_iter2)
        {
          for (unsigned int en_iter2 = 0; en_iter2 < neigh_elem->num_edge_neighbors(edge_iter2); ++en_iter2)
          {
            if (neigh_elem->get_edge_neighbor(edge_iter2, en_iter2) == this)
            {
              if ((en_iter2 > 1) && (en_iter > 1))
              {
                mooseError("Element and neighbor element cannot both have >1 neighbors on a common edge");
              }
              found_neighbor = true;
              break;
            }
          }
        }
        if (!found_neighbor)
          mooseError("Neighbor element doesn't recognize current element as neighbor");
      }
    }
  }
}

std::vector<EFAnode*>
EFAelement::get_common_nodes(const EFAelement* other_elem) const
{
  std::set<EFAnode*> e1nodes(_nodes.begin(), _nodes.end());
  std::set<EFAnode*> e2nodes(other_elem->_nodes.begin(), other_elem->_nodes.end());
  std::vector<EFAnode*> common_nodes;
  std::set_intersection(e1nodes.begin(), e1nodes.end(),
                        e2nodes.begin(), e2nodes.end(),
                        std::inserter(common_nodes,common_nodes.end()));
  return common_nodes;
}

void
EFAelement::init_crack_tip(std::set< EFAelement*> &CrackTipElements)
{
  if (frag_has_tip_edges())
  {
    CrackTipElements.insert(this);
    for (unsigned int edge_iter = 0; edge_iter < _num_edges; ++edge_iter)
    {
      if ((_edge_neighbors[edge_iter].size() == 2) &&
          (_edges[edge_iter]->has_intersection()))
      {
        //Neither neighbor overlays current element.  We are on the uncut element ahead of the tip.
        //Flag neighbors as crack tip split elements and add this element as their crack tip neighbor.

        EFAnode* edge_node1 = _edges[edge_iter]->get_node(0);
        EFAnode* edge_node2 = _edges[edge_iter]->get_node(1);

        if ((_edge_neighbors[edge_iter][0]->overlays_elem(edge_node1,edge_node2)) ||
            (_edge_neighbors[edge_iter][1]->overlays_elem(edge_node1,edge_node2)))
	  mooseError("Element has a neighbor that overlays itself");

        //Make sure the current elment hasn't been flagged as a tip element
        if (_crack_tip_split_element)
          mooseError("crack_tip_split_element already flagged.  In elem: "<<_id
		   << " flags: "<<_crack_tip_split_element
		   <<" "<<_edge_neighbors[edge_iter][0]->is_crack_tip_split()
		   <<" "<<_edge_neighbors[edge_iter][1]->is_crack_tip_split());

        _edge_neighbors[edge_iter][0]->set_crack_tip_split();
        _edge_neighbors[edge_iter][1]->set_crack_tip_split();

        _edge_neighbors[edge_iter][0]->add_crack_tip_neighbor(this);
        _edge_neighbors[edge_iter][1]->add_crack_tip_neighbor(this);
      }
    } // edge_iter
  }
}

void
EFAelement::set_crack_tip_split()
{
  _crack_tip_split_element = true;
}

bool
EFAelement::is_crack_tip_split()
{
  return _crack_tip_split_element;
}

void
EFAelement::is_edge_split_or_tip(unsigned int edge_id, bool &edge_split, bool &edge_tip) const
{
  // check on a child element was just created
  edge_split = false;  //Is the current element being split on this edge?
  edge_tip = false;    //Is the current element a crack tip element with a split on this edge?
  EFAnode* embeddedNode = _edges[edge_id]->get_embedded_node();

  if (_parent && _parent->num_children() >= 1)
  {
    if (_parent->num_children() > 1)
    {
      bool hasSiblingWithSameEmbeddedNode = false;
      for (unsigned int i = 0; i < _parent->num_children(); ++i)
      {
        if (_parent->get_child(i) != this &&
            _parent->get_child(i)->get_edge(edge_id)->get_embedded_node() == embeddedNode)
        {
          hasSiblingWithSameEmbeddedNode = true;
          break;
        }
      }
      if (hasSiblingWithSameEmbeddedNode)
        edge_split = true;
    }
    else if (_parent->is_crack_tip_split())
    {
      for (unsigned int i = 0; i < _parent->num_crack_tip_neighbors(); ++i)
      {
        if (_parent->get_crack_tip_neighbor(i) == edge_id)
        {
          edge_tip = true;
          break;
        }
      }
    }
  }
}

bool
EFAelement::should_duplicate_for_crack_tip(const std::set<EFAelement*> &CrackTipElements)
{
  // This method is called in createChildElements()
  // Only duplicate when 
  // 1) currElem will be a NEW crack tip element
  // 2) currElem is a crack tip split element at last time step and the tip will extend
  // 3) currElem is the neighbor of a to-be-second-split element which has another neighbor
  //    sharing a phantom node with currElem
  bool should_duplicate = false;
  if (_fragments.size() == 1)
  {
    std::set<EFAelement*>::iterator sit;
    sit = CrackTipElements.find(this);
    if (sit == CrackTipElements.end() && frag_has_tip_edges())
      should_duplicate = true;
    else if (shouldDuplicateCrackTipSplitElem())
      should_duplicate = true;
    else if (shouldDuplicateForPhantomCorner())
      should_duplicate = true;
  }
  return should_duplicate;
}

bool
EFAelement::shouldDuplicateCrackTipSplitElem()
{
  //Determine whether element at crack tip should be duplicated.  It should be duplicated
  //if the crack will extend into the next element, or if it has a non-physical node
  //connected to a face where a crack terminates, but will extend.

  bool should_duplicate = false;
  if (_fragments.size() == 1)
  {
    std::vector<unsigned int> split_neighbors;
    if (will_crack_tip_extend(split_neighbors))
      should_duplicate = true;
    else
    {
      //The element may not be at the crack tip, but could have a non-physical node
      //connected to a crack tip face (on a neighbor element) that will be split.  We need to 
      //duplicate in that case as well.

      //Get the set of nodes in neighboring elements that are on a crack tip face that will be split
      std::set<EFAnode*> crack_tip_face_nodes;
      for (unsigned int i = 0; i < _num_edges; ++i)
      {
        for (unsigned int j = 0; j < num_edge_neighbors(i); ++j)
        {
          EFAelement* edge_neighbor = get_edge_neighbor(i,j);
          std::vector<unsigned int> neighbor_split_neighbors;
          if (edge_neighbor->will_crack_tip_extend(neighbor_split_neighbors))
          {
            for (unsigned int k = 0; k < neighbor_split_neighbors.size(); ++k)
            {
              //Get the nodes on the crack tip edge
              std::vector<EFAnode*> edge_nodes;
              edge_neighbor->get_nodes_on_edge(neighbor_split_neighbors[k],edge_nodes);
//              for (unsigned int l=0; l<edge_nodes.size(); ++l)
//              {
                crack_tip_face_nodes.insert(edge_nodes.begin(), edge_nodes.end());
//              }
            } // k
          }
        } // j
      } // i

      //See if any of those nodes are in the non-physical part of this element.
      //Create a set of all non-physical elements
      std::set<EFAnode*> non_physical_nodes;
      get_non_physical_nodes(non_physical_nodes);
      if (num_common_elems(crack_tip_face_nodes, non_physical_nodes) > 0)
        should_duplicate = true;
    }
  }
  return should_duplicate;
}

bool
EFAelement::shouldDuplicateForPhantomCorner()
{
  // if a partial element will be split for a second time and it has two neighbor elements
  // sharing one phantom node with the aforementioned partial element, then the two neighbor 
  // elements should be duplicated
  bool should_duplicate = false;
  if (_fragments.size() == 1 && (!_crack_tip_split_element))
  {
    for (unsigned int i = 0; i < _num_edges; ++i)
    {
      std::set<EFAnode*> phantom_nodes = getPhantomNodeOnEdge(i);
      if (phantom_nodes.size() > 0 && num_edge_neighbors(i) == 1)
      {
        EFAelement * neighbor_elem = _edge_neighbors[i][0];
        if (neighbor_elem->num_frags() > 1) // neighbor will be split
        {
          for (unsigned int j = 0; j < neighbor_elem->num_edges(); ++j)
          {
            if (!neighbor_elem->get_edge(j)->isOverlapping(*_edges[i]) &&
                neighbor_elem->num_edge_neighbors(j) > 0)
            {
              std::set<EFAnode*> neigh_phantom_nodes = neighbor_elem->getPhantomNodeOnEdge(j);
              if (num_common_elems(phantom_nodes, neigh_phantom_nodes) > 0)
              {
                should_duplicate = true;
                break;
              }
            }
          } // j
        }
      }
      if (should_duplicate) break;
    } // i
  }
  return should_duplicate;
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
  for (unsigned int i=0; i<_crack_tip_neighbors.size(); ++i)
  {
    if (_crack_tip_neighbors[i] == neighbor_index)
    {
      mooseError("in add_crack_tip_neighbor() element: "<<_id
                  <<" already has a crack tip neighbor set on side: "<<neighbor_index);
    }
  }
  _crack_tip_neighbors.push_back(neighbor_index);
  if (_crack_tip_neighbors.size() > 2)
  {
    for (unsigned int j=0; j<_crack_tip_neighbors.size(); ++j)
    {
      std::cout<<"Neighbor: "<<_crack_tip_neighbors[j]<<std::endl;
    }
    mooseError("in add_crack_tip_neighbor() element: "
                <<_id<<" cannot have more than 2 crack tip neighbors");
  }
}

bool
EFAelement::will_crack_tip_extend(std::vector<unsigned int> &split_neighbors)
{
  //Determine whether the current element is a crack tip element for which the crack will
  //extend into the next element.
  // N.B. this is called at the beginning of createChildElements
  bool will_extend = false;
  if (_fragments.size() == 1)
  {
    if (_crack_tip_split_element)
    {
      for (unsigned int i=0; i<_crack_tip_neighbors.size(); ++i)
      {
        unsigned int neigh_idx = _crack_tip_neighbors[i];
        if (_edge_neighbors[neigh_idx].size() != 1)
        {
          mooseError("in will_crack_tip_extend() element: "<<_id<<" has: "
                      <<_edge_neighbors[neigh_idx].size()<<" on edge: "<<neigh_idx);
        }
        EFAelement * neighbor_elem = _edge_neighbors[neigh_idx][0];
        if (neighbor_elem->num_frags() > 2)
        {
          mooseError("in will_crack_tip_extend() element: "<<neighbor_elem->id()<<" has: "
                           <<neighbor_elem->num_frags()<<" fragments");
        }
        else if (neighbor_elem->num_frags() == 2)
        {
          will_extend = true;
          split_neighbors.push_back(neigh_idx);
        }
      }
    }
  }
  return will_extend;
}

bool
EFAelement::frag_has_tip_edges()
{
  bool has_tip_edges = false;
  if (_fragments.size() == 1)
  {
    for (unsigned int i = 0; i < _num_edges; ++i)
    {
      unsigned int num_frag_edges = 0; // count how many fragment edges this element edge contains
      if (_edges[i]->has_intersection())
      {
        for (unsigned int j = 0; j < _fragments[0]->num_edges(); ++j)
        {
          if (_edges[i]->containsEdge(*_fragments[0]->get_edge(j)))
            num_frag_edges += 1;
        } // j
        if (num_frag_edges == 2)
        {
          has_tip_edges = true;
          break;
        }
      }
    } // i
  }
  return has_tip_edges;
}

unsigned int
EFAelement::get_tip_edge_id()
{
  // if this element is a crack tip element, returns the crack tip edge's ID
  unsigned int tip_edge_id = 99999;
  if (_fragments.size() > 0) // crack tip element with a partial fragment saved
  {
    for (unsigned int i = 0; i < _num_edges; ++i)
    {
      unsigned int num_frag_edges = 0; // count how many fragment edges this element edge contains
      if (_edges[i]->has_intersection())
      {
        for (unsigned int j = 0; j < _fragments[0]->num_edges(); ++j)
        {
          if (_edges[i]->containsEdge(*_fragments[0]->get_edge(j)))
            num_frag_edges += 1;
        } // j
        if (num_frag_edges == 2) // element edge contains two fragment edges
        {
          tip_edge_id = i;
          break;
        }
      }
    } // i
  }
  else // crack tip element with no fragment saved
  {
    if (get_num_cuts() == 1)
    {
      for (unsigned int i = 0; i < _num_edges; ++i)
      {
        if (_edges[i]->has_intersection())
        {
          tip_edge_id = i;
          break;
        }
      }
    }
  }
  return tip_edge_id;
}

void
EFAelement::add_edge_cut(unsigned int edge_id, double position, EFAnode* embedded_node,
                         std::map< unsigned int, EFAnode*> &EmbeddedNodes)
{
  EFAnode* local_embedded_node = NULL;
  EFAnode* edge_node1 = _nodes[edge_id];
  unsigned int frag_id = 99999; // which fragment has the overlapping edge
  unsigned int frag_edge_id = 99999; // the id of the overlapping fragment edge
  if (embedded_node) //use the existing embedded node if it was passed in
    local_embedded_node = embedded_node;

  if (_edges[edge_id]->has_intersection())
  {
    if (!_edges[edge_id]->has_intersection_at_position(position,edge_node1) ||
        (embedded_node && _edges[edge_id]->get_embedded_node() != embedded_node))
    {
      mooseError("Attempting to add edge intersection when one already exists with different position or node."
                  << " elem: "<<_id<<" edge: "<<edge_id<<" position: "<<position<<" old position: "
                  <<_edges[edge_id]->get_intersection(edge_node1));
    }
    local_embedded_node = _edges[edge_id]->get_embedded_node();
  }
  else // blank edge
  {
    if (!local_embedded_node)
    {
      //create the embedded node
      unsigned int new_node_id = getNewID(EmbeddedNodes);
      local_embedded_node = new EFAnode(new_node_id,N_CATEGORY_EMBEDDED);
      EmbeddedNodes.insert(std::make_pair(new_node_id,local_embedded_node));
    }
    _edges[edge_id]->add_intersection(position, local_embedded_node, edge_node1);
    if (getFragmentEdgeID(edge_id, frag_id, frag_edge_id)) // get frag_id, frag_edge_id
    {
      if (!_fragments[frag_id]->get_edge(frag_edge_id)->has_intersection())
        _fragments[frag_id]->get_edge(frag_edge_id)->add_intersection(position,local_embedded_node,edge_node1);
    }
  }

  for (unsigned int en_iter = 0; en_iter < _edge_neighbors[edge_id].size(); ++en_iter)
  {
    EFAelement *edge_neighbor = _edge_neighbors[edge_id][en_iter];
    if (edge_neighbor)
    {
      unsigned int neighbor_edge_id = edge_index_in_neighbor(edge_neighbor);
      if (edge_neighbor->get_edge(neighbor_edge_id)->has_intersection())
      {
        if (!edge_neighbor->get_edge(neighbor_edge_id)->equivalent(*_edges[edge_id]))
          mooseError("Attempting to add edge intersection when neighbor already has one that is incompatible."
                     << " elem: "<<_id<<" edge: "<<edge_id
                     <<" neighbor: "<<edge_neighbor->id()<<" neighbor edge: "<<neighbor_edge_id);
      }
      else
      {
        edge_neighbor->get_edge(neighbor_edge_id)->add_intersection(position,local_embedded_node,edge_node1);
        if (edge_neighbor->getFragmentEdgeID(neighbor_edge_id, frag_id, frag_edge_id)) // get frag_id, frag_edge_id
        {
          if (!edge_neighbor->get_frag_edge(frag_id,frag_edge_id)->has_intersection())
            edge_neighbor->get_frag_edge(frag_id,frag_edge_id)->add_intersection(position,local_embedded_node,edge_node1);
        }
      }
    } // edge_neighbor exsits
  } // en_iter
}

void
EFAelement::add_frag_edge_cut(unsigned int frag_edge_id, double position,
                              std::map< unsigned int, EFAnode*> &EmbeddedNodes)
{
  if (_fragments.size() != 1)
    mooseError("Element: "<<_id<<" should have only 1 fragment in addFragEdgeIntersection");
  EFAnode* local_embedded_node = NULL;

  // check if this intersection coincide with any embedded node on this edge
  double tol = 1.0e-4;
  bool isValidIntersection = true;
  EFAnode* edge_node1 = _fragments[0]->get_edge(frag_edge_id)->get_node(0);
  EFAnode* edge_node2 = _fragments[0]->get_edge(frag_edge_id)->get_node(1);
  if ((std::abs(position) < tol && edge_node1->category() == N_CATEGORY_EMBEDDED) ||
      (std::abs(1.0-position) < tol && edge_node2->category() == N_CATEGORY_EMBEDDED))
    isValidIntersection = false;

  // add valid intersection point to an edge 
  if (isValidIntersection)
  {
    if (_fragments[0]->get_edge(frag_edge_id)->has_intersection())
    {
      if (!_fragments[0]->get_edge(frag_edge_id)->has_intersection_at_position(position,edge_node1))
        mooseError("Attempting to add fragment edge intersection when one already exists with different position or node."
                         << " elem: "<<_id<<" edge: "<<frag_edge_id<<" position: "<<position<<" old position: "
                         <<_fragments[0]->get_edge(frag_edge_id)->get_intersection(edge_node1));
    }
    else // blank edge - in fact, it can only be a blank element interior edge
    {
      if (!_fragments[0]->get_edge(frag_edge_id)->is_interior_edge())
        mooseError("Attemping to add intersection to a non-interior edge. Element: "
                    <<_id<<" fragment_edge: " << frag_edge_id);

      //create the embedded node and add it to the fragment's boundary edge
      unsigned int new_node_id = getNewID(EmbeddedNodes);
      local_embedded_node = new EFAnode(new_node_id,N_CATEGORY_EMBEDDED);
      EmbeddedNodes.insert(std::make_pair(new_node_id,local_embedded_node));
      _fragments[0]->get_edge(frag_edge_id)->add_intersection(position, local_embedded_node, edge_node1);

      //save this interior embedded node to FaceNodes
      std::vector<double> node1_para_coor(2,0.0);
      std::vector<double> node2_para_coor(2,0.0);
      if (getEmbeddedNodeParaCoor(edge_node1, node1_para_coor) &&
          getEmbeddedNodeParaCoor(edge_node2, node2_para_coor))
      {
        double xi  = (1.0-position)*node1_para_coor[0] + position*node2_para_coor[0];
        double eta = (1.0-position)*node1_para_coor[1] + position*node2_para_coor[1];
        _interior_nodes.push_back(new FaceNode(local_embedded_node, xi, eta));
      }
      else
        mooseError("elem: "<<_id<<" cannot get the para coords of two end embedded nodes");
    }
    // no need to add intersection for neighbor fragment - if this fragment has a
    // neighbor fragment, the neighbor has already been treated in addEdgeIntersection;
    // for an interior edge, there is no neighbor fragment
  }
}

unsigned int
EFAelement::get_num_cuts()
{
  unsigned int num_cuts = 0;
  for (unsigned int i = 0; i < _num_edges; ++i)
    if (_edges[i]->has_intersection())
      num_cuts += 1;
  return num_cuts;
}

bool
EFAelement::is_cut_twice()
{
  bool cut_twice = false;
  if (_fragments.size() > 0)
  {
    unsigned int num_interior_edges = 0;
    for (unsigned int i = 0; i < _fragments[0]->num_edges(); ++i)
    {
      if (_fragments[0]->get_edge(i)->is_interior_edge())
        num_interior_edges += 1;
    }
    if (num_interior_edges == 2)
      cut_twice = true;
  }
  return cut_twice;
}

void
EFAelement::update_fragments(const std::set<EFAelement*> &CrackTipElements,
                             std::map<unsigned int, EFAnode*> &EmbeddedNodes)
{
  // combine the crack-tip edges in a fragment to a single intersected edge
  std::set<EFAelement*>::iterator sit;
  sit = CrackTipElements.find(this);
  if (sit != CrackTipElements.end()) // curr_elem is a crack tip element
  {
    if (_fragments.size() == 1)
      _fragments[0]->combine_tip_edges();
    else
      mooseError("crack tip elem " << _id << " can only have 1 fragment");
  }

  // if a fragment only has 1 intersection which is in an interior edge
  // remove this embedded node (MUST DO THIS AFTER combine_tip_edges())
  if (_fragments.size() == 1 && _fragments[0]->get_num_cuts() == 1)
  {
    for (unsigned int i = 0; i < _fragments[0]->num_edges(); ++i)
    {
      if (_fragments[0]->get_edge(i)->is_interior_edge() &&
          _fragments[0]->get_edge(i)->has_intersection())
      {
        if (_interior_nodes.size() != 1)
          mooseError("The element must have 1 interior node");
        deleteFromMap(EmbeddedNodes,_fragments[0]->get_edge(i)->get_embedded_node());
        _fragments[0]->get_edge(i)->remove_embedded_node(); // set pointer to NULL
        delete _interior_nodes[0];
        _interior_nodes.clear();
        break;
      }
    }
  }

  // for an element with no fragment, create one fragment identical to the element
  if (_fragments.size() == 0)
      _fragments.push_back(new EFAfragment(this, true, this));
  if (_fragments.size() != 1)
    mooseError("In update_fragments(): element "<<_id<<" must have 1 fragment at this point");

  // count fragment's cut edges
  unsigned int num_cut_frag_edges = _fragments[0]->get_num_cuts();
  if (num_cut_frag_edges > 2)
    mooseError("In element "<<_id<<" there are more than 2 cut fragment edges");

  if (num_cut_frag_edges == 0)
  {
    if (!is_partial()) // delete the temp frag for an uncut elem
    {
      delete _fragments[0];
      _fragments.clear();
    }
    //Element has already been cut. Don't recreate fragments because we
    //would create multiple fragments to cover the entire element and
    //lose the information about what part of this element is physical.
    return;
  }

  // split one fragment into one or two new fragments
  std::vector<EFAfragment *> new_frags = _fragments[0]->split();
  if (new_frags.size() == 1 || new_frags.size() == 2)
  {
    delete _fragments[0]; // delete the old fragment
    _fragments.clear();
    for (unsigned int i = 0; i < new_frags.size(); ++i)
      _fragments.push_back(new_frags[i]);
  }
  else
    mooseError("Number of fragments must be 1 or 2 at this point");

  fragment_sanity_check();
}

void
EFAelement::fragment_sanity_check()
{
  std::vector<unsigned int> cut_edges;
  for (unsigned int iedge = 0; iedge < _num_edges; ++iedge)
  {
    if(_edges[iedge]->has_intersection())
    {
      cut_edges.push_back(iedge);
    }
  }
  if (cut_edges.size() > 3)
    mooseError("In element "<<_id<<" more than 2 cut edges");

  std::vector<unsigned int> num_emb;
  std::vector<unsigned int> num_perm;
  for (unsigned int i = 0; i < _fragments.size(); ++i)
  {
    num_emb.push_back(0);
    num_perm.push_back(0);
    std::set<EFAnode*> perm_nodes;
    std::set<EFAnode*> emb_nodes;
    for (unsigned int j = 0; j < _fragments[i]->num_edges(); ++j)
    {
      for (unsigned int k = 0; k < 2; ++k)
      {
        EFAnode * temp_node = _fragments[i]->get_edge(j)->get_node(k);
        if (temp_node->category() == N_CATEGORY_PERMANENT)
          perm_nodes.insert(temp_node);
        else if (temp_node->category() == N_CATEGORY_EMBEDDED)
          emb_nodes.insert(temp_node);
        else
          mooseError("Invalid node category");
      }
    }
    num_perm[i] = perm_nodes.size();
    num_emb[i] = emb_nodes.size();
  }

  unsigned int num_interior_nodes = getNumInteriorNodes();
  if (num_interior_nodes > 0 && num_interior_nodes != 1)
    mooseError("After updatePhysicalLinksAndFragments this element has "<<num_interior_nodes<<" interior nodes");

  if (cut_edges.size() == 0)
  {
    if (_fragments.size() != 1 ||
        _fragments[0]->num_edges() != _num_edges)
      mooseError("Incorrect link size for element with 0 cuts");

    if (num_emb[0] != 0 || num_perm[0] != _num_edges)
      mooseError("Incorrect node category for element with 0 cuts");
  }
  else if (cut_edges.size() == 1)
  {
    if (_fragments.size() != 1 ||
        _fragments[0]->num_edges() != _num_edges+1)
      mooseError("Incorrect link size for element with 1 cut");

    if (num_emb[0] != 1 || num_perm[0] != _num_edges)
      mooseError("Incorrect node category for element with 1 cut");
  }
  else if (cut_edges.size() == 2)
  {
    if (_fragments.size() != 2 ||
       (_fragments[0]->num_edges()+_fragments[1]->num_edges()) != _num_edges+4)
      mooseError("Incorrect link size for element with 2 cuts");

    if (num_emb[0] != 2 || num_emb[1] != 2 || (num_perm[0]+num_perm[1]) != _num_edges)
      mooseError("Incorrect node category for element with 2 cuts");
  }
  else if (cut_edges.size() == 3) // TODO: not a good sanity check
  {
    if (_fragments.size() == 1)
    {
      if (num_emb[0] != 3)
        mooseError("Incorrect number of embedded nodes for element with 3 cuts and 1 fragment");
    }
    else if (_fragments.size() == 2)
    {
      if (num_emb[0] != 3 || num_emb[1] != 3)
        mooseError("Incorrect number of embedded nodes for element with 3 cuts and 2 fragment");
    }
  }
}

void
EFAelement::restore_fragment(const EFAelement* const from_elem)
{
  // restore fragments
  if (_fragments.size() != 0)
    mooseError("in restoreFragmentInfo elements must not have any pre-existing fragments");
  for (unsigned int i = 0; i < from_elem->num_frags(); ++i)
    _fragments.push_back(new EFAfragment(*from_elem->_fragments[i], this));

  // restore interior nodes
  if (_interior_nodes.size() != 0)
    mooseError("in restoreFragmentInfo elements must not have any pre-exsiting interior nodes");
  for (unsigned int i = 0; i < from_elem->_interior_nodes.size(); ++i)
    _interior_nodes.push_back(new FaceNode(*from_elem->_interior_nodes[i]));

  // replace all local nodes with global nodes
  for (unsigned int i = 0; i < from_elem->num_nodes(); ++i)
  {
    if (from_elem->_nodes[i]->category() == N_CATEGORY_LOCAL_INDEX)
      switchNode(_nodes[i], from_elem->_nodes[i], false); //EFAelement is not a child of any parent
    else
      mooseError("In restoreFragmentInfo all of from_elem's nodes must be local");
  }
}

void
EFAelement::create_child(const std::set<EFAelement*> &CrackTipElements,
                         std::map<unsigned int, EFAelement*> &Elements,
                         std::map<unsigned int, EFAelement*> &newChildElements, 
                         std::vector<EFAelement*> &ChildElements,
                         std::vector<EFAelement*> &ParentElements,
                         std::map<unsigned int, EFAnode*> &TempNodes)
{
  if (_children.size() != 0)
    mooseError("Element cannot have existing children in createChildElements");

  if (_fragments.size() > 1 || should_duplicate_for_crack_tip(CrackTipElements))
  {
    if (_fragments.size() > 2)
      mooseError("More than 2 fragments not yet supported");

    //set up the children
    ParentElements.push_back(this);
    for (unsigned int ichild = 0; ichild < _fragments.size(); ++ichild)
    {
      unsigned int new_elem_id;
      if (newChildElements.size() == 0)
        new_elem_id = getNewID(Elements);
      else
        new_elem_id = getNewID(newChildElements);

      EFAelement* childElem = new EFAelement(new_elem_id);
      newChildElements.insert(std::make_pair(new_elem_id,childElem));

      ChildElements.push_back(childElem);
      childElem->set_parent(this);
      _children.push_back(childElem);

      // get child element's nodes
      for (unsigned int j = 0; j < _num_nodes; ++j)
      {
        if (_fragments[ichild]->containsNode(_nodes[j]))
          childElem->set_node(j, _nodes[j]); // inherit parent's node
        else // parent element's node is not in fragment
        {
          unsigned int new_node_id = getNewID(TempNodes);
          EFAnode* newNode = new EFAnode(new_node_id,N_CATEGORY_TEMP,_nodes[j]);
          TempNodes.insert(std::make_pair(new_node_id,newNode));
          childElem->set_node(j, newNode); // be a temp node
        }
      }

      // get child element's fragments
      EFAfragment * new_frag = new EFAfragment(childElem, true, this, ichild);
      childElem->add_fragment(new_frag);

      // get child element's edges
      for (unsigned int j = 0; j < _num_edges; ++j)
      {
        unsigned int jplus1(j < (_num_edges-1) ? j+1 : 0);
        EFAedge * new_edge = new EFAedge(childElem->get_node(j), childElem->get_node(jplus1));
        if (_edges[j]->has_intersection())
        {
          double child_position = _edges[j]->get_intersection(_nodes[j]);
          EFAnode * child_embedded_node = _edges[j]->get_embedded_node();
          new_edge->add_intersection(child_position, child_embedded_node, childElem->get_node(j));
        }
        childElem->set_edge(j, new_edge);
      }
      for (unsigned int j = 0; j < _num_edges; ++j) // remove embedded node on phantom edge
      {
        if (childElem->is_edge_phantom(j) && childElem->get_edge(j)->has_intersection())
          childElem->get_edge(j)->remove_embedded_node();
      }

      // inherit old interior nodes
      for (unsigned int j = 0; j < _interior_nodes.size(); ++j)
        childElem->add_interior_node(new FaceNode(*_interior_nodes[j]));
    }
  }
  else //num_links == 1 || num_links == 0
  {
    //child is itself - but don't insert into the list of ChildElements!!!
    _children.push_back(this);
  }
}

void
EFAelement::connect_neighbors(std::map<unsigned int, EFAnode*> &PermanentNodes,
                              std::map<unsigned int, EFAnode*> &EmbeddedNodes,
                              std::map<unsigned int, EFAnode*> &TempNodes,
                              std::map<EFAnode*, std::set<EFAelement*> > &InverseConnectivityMap,
                              bool merge_phantom_edges)
{
  // N.B. "this" must point to a child element that was just created
  if (!_parent)
    mooseError("no parent element for child element " << _id);

  //First loop through edges and merge nodes with neighbors as appropriate
  for (unsigned int j = 0; j < _num_edges; ++j)
  {
    for (unsigned int k = 0; k < _parent->num_edge_neighbors(j); ++k)
    {   
      EFAelement* NeighborElem = _parent->get_edge_neighbor(j,k);
      unsigned int neighbor_edge_id = _parent->edge_index_in_neighbor(NeighborElem);
      //get nodes on this edge of childElem
      std::vector<EFAnode*> childEdgeNodes;
      childEdgeNodes.push_back(_edges[j]->get_node(0));
      childEdgeNodes.push_back(_edges[j]->get_node(1));

      if (_edges[j]->has_intersection())
      {
        for (unsigned int l = 0; l < NeighborElem->num_children(); ++l)
        {
          EFAelement *childOfNeighborElem = NeighborElem->get_child(l);

          //get nodes on this edge of childOfNeighborElem
          std::vector<EFAnode*> childOfNeighborEdgeNodes;
          EFAedge *neighborChildEdge = childOfNeighborElem->get_edge(neighbor_edge_id);
          childOfNeighborEdgeNodes.push_back(neighborChildEdge->get_node(0));
          childOfNeighborEdgeNodes.push_back(neighborChildEdge->get_node(1));

          //Check to see if the nodes are already merged.  There's nothing else to do in that case.
          if (childEdgeNodes[0] == childOfNeighborEdgeNodes[1] &&
              childEdgeNodes[1] == childOfNeighborEdgeNodes[0])
            continue;

          if (_fragments[0]->isConnected(*childOfNeighborElem->get_fragment(0)))
          {
            std::vector<EFAnode*> merged_nodes;
            unsigned int num_edge_nodes = 2;
            for (unsigned int i=0; i < num_edge_nodes; ++i)
            {
              unsigned int childNodeIndex = i;
              unsigned int neighborChildNodeIndex = num_edge_nodes - 1 - childNodeIndex;

              EFAnode* childNode = childEdgeNodes[childNodeIndex];
              EFAnode* childOfNeighborNode = childOfNeighborEdgeNodes[neighborChildNodeIndex];

              mergeNodes(childNode, childOfNeighborNode, this, childOfNeighborElem,
                         PermanentNodes, TempNodes);
              merged_nodes.push_back(childNode);
            }

            duplicateEmbeddedNode(j, childOfNeighborElem, neighbor_edge_id, EmbeddedNodes);
          }
        } // l, loop over NeighborElem's children
      }
      else //No edge intersection -- optionally merge non-material nodes if they share a common parent
      {
        if (merge_phantom_edges)
        {
          for (unsigned int l = 0; l < NeighborElem->num_children(); ++l)
          {
            EFAelement *childOfNeighborElem = NeighborElem->get_child(l);
            EFAedge *neighborChildEdge = childOfNeighborElem->get_edge(neighbor_edge_id);

            if (!neighborChildEdge->has_intersection()) //neighbor edge must NOT have intersection either
            {
              //get nodes on this edge of childOfNeighborElem
              std::vector<EFAnode*> childOfNeighborEdgeNodes;
              childOfNeighborEdgeNodes.push_back(neighborChildEdge->get_node(0));
              childOfNeighborEdgeNodes.push_back(neighborChildEdge->get_node(1));

              //Check to see if the nodes are already merged.  There's nothing else to do in that case.
              if (childEdgeNodes[0] == childOfNeighborEdgeNodes[1] &&
                  childEdgeNodes[1] == childOfNeighborEdgeNodes[0])
                continue;

              unsigned int num_edge_nodes = 2;
              for (unsigned int i = 0; i < num_edge_nodes; ++i)
              {
                unsigned int childNodeIndex = i;
                unsigned int neighborChildNodeIndex = num_edge_nodes - 1 - childNodeIndex;

                EFAnode* childNode = childEdgeNodes[childNodeIndex];
                EFAnode* childOfNeighborNode = childOfNeighborEdgeNodes[neighborChildNodeIndex];

                if (childNode->parent() != NULL &&
                    childNode->parent() == childOfNeighborNode->parent()) //non-material node and both come from same parent
                {
                  mergeNodes(childNode, childOfNeighborNode, this, childOfNeighborElem,
                             PermanentNodes, TempNodes);
                }
              }
            }
          } // loop over NeighborElem's children
        } // if (merge_phantom_edges)
      } // IF edge-j has_intersection()
    } // k, loop over neighbors on edge j
  } // j, loop over all edges

  //Now do a second loop through edges and convert remaining nodes to permanent nodes.
  //If there is no neighbor on that edge, also duplicate the embedded node if it exists
  for (unsigned int j = 0; j < _num_edges; ++j)
  {
    EFAnode* childNode = _nodes[j];

    if (childNode->category() == N_CATEGORY_TEMP)
    {
      // if current child element does not have siblings, and if current temp node is a lonely one
      // this temp node should be merged back to its parent permanent node. Otherwise we would have
      // permanent nodes that are not connected to any element
      std::set<EFAelement*> patch_elems = InverseConnectivityMap[childNode->parent()];
      if (_parent->num_frags() == 1 && patch_elems.size() == 1)
        switchNode(childNode->parent(), childNode);
      else
      {
        unsigned int new_node_id = getNewID(PermanentNodes);
        EFAnode* newNode = new EFAnode(new_node_id,N_CATEGORY_PERMANENT,childNode->parent());
        PermanentNodes.insert(std::make_pair(new_node_id,newNode));
        switchNode(newNode, childNode);
      }
      if (!deleteFromMap(TempNodes, childNode))
        mooseError("Attempted to delete node: "<<childNode->id()
                    <<" from TempNodes, but couldn't find it");
    }
    //No neighbor of parent on this edge -- free edge -- need to convert to permanent nodes
    if (_parent->num_edge_neighbors(j) == 0 && _edges[j]->has_intersection())
      duplicateEmbeddedNode(j, EmbeddedNodes);
  } // j
  
  //duplicate the interior embedded node
  if (_interior_nodes.size() > 0)
    duplicateInteriorEmbeddedNode(EmbeddedNodes);
}

void
EFAelement::print_elem()
{
  std::cout << std::setw(4);
  std::cout << _id << " | ";
  for (unsigned int j = 0; j < _num_nodes; ++j)
  {
    std::cout << std::setw(5) << _nodes[j]->id_cat_str();
  }

  std::cout << "  | ";
  for (unsigned int j = 0; j < _num_edges; ++j)
  {
    std::cout<<std::setw(4);
    if (_edges[j]->has_intersection())
      std::cout << _edges[j]->get_embedded_node()->id() << " ";
    else
      std::cout << "  -- ";
  }
  std::cout << "  | ";
  for (unsigned int j = 0; j < _num_edges; ++j)
  {
    if (num_edge_neighbors(j) > 1)
    {
      std::cout << "[";
      for (unsigned int k = 0; k < num_edge_neighbors(j); ++k)
      {
        std::cout << get_edge_neighbor(j,k)->id();
        if (k == num_edge_neighbors(j)-1)
          std::cout<<"]";
        else
          std::cout<<" ";
      }
      std::cout<< " ";
    }
    else
    {
      std::cout<<std::setw(4);
      if (num_edge_neighbors(j) == 1)
        std::cout << get_edge_neighbor(j,0)->id() << " ";
      else
        std::cout << "  -- ";
    }
  }
  std::cout << "  | ";
  for (unsigned int j = 0; j < _fragments.size(); ++j)
  {
    std::cout<<std::setw(4);
    std::cout << " " << j << " | ";
    for (unsigned int k = 0; k < _fragments[j]->num_edges(); ++k)
    {
      EFAnode* prt_node = get_frag_edge(j,k)->get_node(0);
      unsigned int kprev(k>0 ? k-1 : _fragments[j]->num_edges()-1);
      if (!get_frag_edge(j,kprev)->containsNode(prt_node))
        prt_node = get_frag_edge(j,k)->get_node(1);
      std::cout << std::setw(5) << prt_node->id_cat_str();
    }
  }
  std::cout << std::endl;
}

void
EFAelement::mapParaCoorFrom1Dto2D(unsigned int edge_id, double xi_1d, std::vector<double> &para_coor)
{
  para_coor.resize(2,0.0);
  if (_num_edges == 4)
  {
    if(edge_id == 0)
    {
      para_coor[0] = xi_1d;
      para_coor[1] = -1.0;
    }
    else if(edge_id == 1)
    {
      para_coor[0] = 1.0;
      para_coor[1] = xi_1d;
    }
    else if(edge_id == 2)
    {
      para_coor[0] = -xi_1d;
      para_coor[1] = 1.0;
    }
    else if(edge_id == 3)
    {
      para_coor[0] = -1.0;
      para_coor[1] = -xi_1d;
    }
    else
      mooseError("edge_id out of bounds");
  }
  else
    mooseError("mapParaCoorFram1Dto2D only for quad element only");
}

void
EFAelement::mergeNodes(EFAnode* &childNode, EFAnode* &childOfNeighborNode,
                       EFAelement* childElem, EFAelement* childOfNeighborElem,       
                       std::map<unsigned int, EFAnode*> &PermanentNodes,
                       std::map<unsigned int, EFAnode*> &TempNodes)
{
  if (childNode != childOfNeighborNode)
  {
    if(childNode->category() == N_CATEGORY_PERMANENT)
    {
      if(childOfNeighborNode->category() == N_CATEGORY_PERMANENT)
      {
        if (childOfNeighborNode->parent() == childNode) // merge into childNode
        {
          childOfNeighborElem->switchNode(childNode, childOfNeighborNode);
          if (!deleteFromMap(PermanentNodes, childOfNeighborNode))
          {
            mooseError("Attempted to delete node: "<<childOfNeighborNode->id()
                        <<" from PermanentNodes, but couldn't find it");
          }
          childOfNeighborNode = childNode;
        }
        else if (childNode->parent() == childOfNeighborNode) // merge into childOfNeighborNode
        {
          childElem->switchNode(childOfNeighborNode, childNode);
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
          childOfNeighborElem->switchNode(childNode, childOfNeighborNode);
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
        childOfNeighborElem->switchNode(childNode, childOfNeighborNode);
        if (!deleteFromMap(TempNodes, childOfNeighborNode))
        {
          mooseError("Attempted to delete node: "<<childOfNeighborNode->id()
                      <<" from TempNodes, but couldn't find it");
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
      childElem->switchNode(childOfNeighborNode, childNode);
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

      childOfNeighborElem->switchNode(newNode, childOfNeighborNode);
      childElem->switchNode(newNode, childNode);

      if (childNode->parent() != childOfNeighborNode->parent())
      {
        mooseError("Attempting to merge nodes "<<childNode->id()<<" and "
                    <<childOfNeighborNode->id()<<" but they don't share a common parent");
      }

      if (!deleteFromMap(TempNodes, childOfNeighborNode))
      {
        mooseError("Attempted to delete node: "<<childOfNeighborNode->id()
                    <<" from TempNodes, but couldn't find it");
      }
      if (!deleteFromMap(TempNodes, childNode))
      {
        mooseError("Attempted to delete node: "<<childNode->id()
                    <<" from TempNodes, but couldn't find it");
      }
      childOfNeighborNode = newNode;
      childNode = newNode;
    }
  }
}

//This version is for cases where there is a neighbor element
void
EFAelement::duplicateEmbeddedNode(unsigned int edgeID, EFAelement* neighborElem,
                                  unsigned int neighborEdgeID,
                                  std::map<unsigned int, EFAnode*> &EmbeddedNodes)
{
  EFAnode* embeddedNode = _edges[edgeID]->get_embedded_node();
  EFAnode* neighEmbeddedNode = neighborElem->get_edge(neighborEdgeID)->get_embedded_node();

  if (embeddedNode != neighEmbeddedNode)
    mooseError("Embedded nodes on merged edge must match");

  //Check to see whether the embedded node can be duplicated for both the current element
  //and the neighbor element.

  bool current_split = false;   //Is the current element being split on this edge?
  bool current_tip = false;     //Is the current element a crack tip element with a split on this edge?
  is_edge_split_or_tip(edgeID, current_split, current_tip);

  bool neighbor_split = false;  //Is the neighbor element being split on this edge?
  bool neighbor_tip = false;    //Is the neighbor element a crack tip element with a split on this edge?
  neighborElem->is_edge_split_or_tip(neighborEdgeID, neighbor_split, neighbor_tip);

  bool can_dup = false;
  // Don't duplicate if current and neighbor are both crack tip elems
  // because that embedded node has already been duplicated
  if ((current_split && neighbor_split) ||
      (current_split && neighbor_tip) ||
      (neighbor_split && current_tip))
    can_dup = true;

  if (can_dup)
  {
    // Determine whether the split occurs on the current embedded node.
    // This would happen if the link set contains the embedded node
    // and only one of the nodes on the edge.
    std::vector<EFAnode*> currCommonNodes = _fragments[0]->commonNodesWithEdge(*_edges[edgeID]);
    std::vector<EFAnode*> neighCommonNodes = neighborElem->get_fragment(0)->commonNodesWithEdge(*_edges[edgeID]);   

    if (_fragments[0]->containsNode(embeddedNode) &&
        neighborElem->get_fragment(0)->containsNode(embeddedNode) &&
        currCommonNodes.size() == 1 && neighCommonNodes.size() == 1)
    {
      // Duplicate this embedded node
      unsigned int new_node_id = getNewID(EmbeddedNodes);
      EFAnode* newNode = new EFAnode(new_node_id,N_CATEGORY_EMBEDDED);
      EmbeddedNodes.insert(std::make_pair(new_node_id,newNode));

      switchEmbeddedNode(newNode, embeddedNode);
      neighborElem->switchEmbeddedNode(newNode, embeddedNode);
    }
  }
}

//This version is for cases when there is no neighbor
void
EFAelement::duplicateEmbeddedNode(unsigned int edgeID,
                                  std::map<unsigned int, EFAnode*> &EmbeddedNodes)
{
  EFAnode* embeddedNode = _edges[edgeID]->get_embedded_node();

  // Do elements on both side of the common edge have siblings?
  if (_parent &&
      _parent->num_children() > 1)
  {
    // Determine whether any of the sibling child elements have the same
    // embedded node.  Only duplicate if that is the case.

    bool currElemHasSiblingWithSameEmbeddedNode = false;
    for (unsigned int i = 0; i < _parent->num_children(); ++i)
    {
      if (_parent->get_child(i) != this &&
          _parent->get_child(i)->get_edge(edgeID)->get_embedded_node() == embeddedNode)
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

      std::vector<EFAnode*> currCommonNodes = _fragments[0]->commonNodesWithEdge(*_edges[edgeID]);
      
      if (currCommonNodes.size() == 1 &&
          _fragments[0]->containsNode(embeddedNode))
      {
        // Duplicate this embedded node
        unsigned int new_node_id = getNewID(EmbeddedNodes);
        EFAnode* newNode = new EFAnode(new_node_id,N_CATEGORY_EMBEDDED);
        EmbeddedNodes.insert(std::make_pair(new_node_id,newNode));
        switchEmbeddedNode(newNode, embeddedNode);
      }
    }
  }
}

void
EFAelement::duplicateInteriorEmbeddedNode(std::map<unsigned int, EFAnode*> &EmbeddedNodes)
{
  if (_interior_nodes.size() != 1)
    mooseError("current elem must have 1 interior node");
  EFAnode* embeddedNode = _interior_nodes[0]->get_node();

  if (_parent &&
      _parent->num_children() > 1)
  {
    // Determine whether any of the sibling child elements have the same
    // embedded node.  Only duplicate if that is the case.

    bool currElemHasSiblingWithSameEmbeddedNode = false;
    for (unsigned int i = 0; i < _parent->num_children(); ++i)
    {
      if (_parent->get_child(i) != this)
      {
        if (_parent->get_child(i)->getNumInteriorNodes() != 1)
          mooseError("sibling elem must have 1 interior node");
        if (_parent->get_child(i)->get_interior_node(0)->get_node() == embeddedNode)
        {
          currElemHasSiblingWithSameEmbeddedNode = true;
          break;
        }
      }
    }

    if (currElemHasSiblingWithSameEmbeddedNode)
    {
      // Duplicate this embedded node
      unsigned int new_node_id = getNewID(EmbeddedNodes);
      EFAnode* newNode = new EFAnode(new_node_id,N_CATEGORY_EMBEDDED);
      EmbeddedNodes.insert(std::make_pair(new_node_id,newNode));
      switchEmbeddedNode(newNode, embeddedNode);
    }
  }
}
