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

EFAelement::EFAelement(unsigned int eid):
  _id(eid),
  num_nodes(4),
  num_edges(4),
  nodes(num_nodes,NULL),
  edges(num_edges,NULL),
  parent(NULL),
  edge_neighbors(num_edges,std::vector<EFAelement*>(1,NULL)),
  crack_tip_split_element(false)
{}

EFAelement::EFAelement(const EFAelement* from_elem, bool convert_to_local):
  _id(from_elem->id()),
  num_nodes(from_elem->num_nodes),
  num_edges(from_elem->num_edges),
  nodes(num_nodes,NULL),
  edges(num_edges,NULL),
  parent(NULL),
  edge_neighbors(num_edges,std::vector<EFAelement*>(1,NULL)),
  crack_tip_split_element(from_elem->crack_tip_split_element)
{
  if (convert_to_local)
  {
    // build local nodes from global nodes
    for (unsigned int i = 0; i < num_nodes; ++i)
    {
      if (from_elem->nodes[i]->category() == N_CATEGORY_PERMANENT ||
          from_elem->nodes[i]->category() == N_CATEGORY_TEMP)
      {
        nodes[i] = from_elem->create_local_node_from_global_node(from_elem->nodes[i]);
        local_nodes.push_back(nodes[i]);
      }
      else
        mooseError("In EFAelement "<<from_elem->id()<<" the copy constructor must have from_elem w/ global nodes. node: "
                    << i << " category: "<<from_elem->nodes[i]->category());
    }

    // copy edges, fragments and interior nodes from from_elem
    for (unsigned int i = 0; i < num_edges; ++i)
      edges[i] = new EFAedge(*from_elem->edges[i]);
    for (unsigned int i = 0; i < from_elem->fragments.size(); ++i)
      fragments.push_back(new EFAfragment(NULL, true, from_elem, i));
    for (unsigned int i = 0; i < from_elem->interior_nodes.size(); ++i)
      interior_nodes.push_back(new FaceNode(*from_elem->interior_nodes[i]));

    // replace all global nodes with local nodes
    for (unsigned int i = 0; i < num_nodes; ++i)
    {
      if (nodes[i]->category() == N_CATEGORY_LOCAL_INDEX)
        switchNode(nodes[i], from_elem->nodes[i], false);//when save to _cut_elem_map, the EFAelement is not a child of any parent
      else
        mooseError("In EFAelement copy constructor this elem's nodes must be local");
    } 
  }
  else
    mooseError("this EFAelement constructor only converts global nodes to local nodes");
}

EFAelement::~EFAelement()
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
  for (unsigned int i=0; i<interior_nodes.size(); ++i)
  {
    if (interior_nodes[i])
    {
      delete interior_nodes[i];
      interior_nodes[i] = NULL;
    }
  }
  for (unsigned int i = 0; i < local_nodes.size(); ++i)
  {
    if (local_nodes[i])
    {
      delete local_nodes[i];
      local_nodes[i] = NULL;
    }
  }
}

void
EFAelement::createEdges()
{
  for (unsigned int i = 0; i < num_nodes; ++i)
  {
    unsigned int i_plus1(i < (num_nodes-1) ? i+1 : 0);
    EFAedge * new_edge = new EFAedge(nodes[i], nodes[i_plus1]);
    edges[i] = new_edge;
  }
}

void
EFAelement::switchNode(EFAnode *new_node,
                       EFAnode *old_node,
                       bool descend_to_parent)
{
  for (unsigned int i=0; i<num_nodes; ++i)
  {
    if (nodes[i] == old_node)
      nodes[i] = new_node;
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
          EFAelement* edge_neighbor = parent->edge_neighbors[i][j];
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
EFAelement::switchEmbeddedNode(EFAnode *new_emb_node,
                                           EFAnode *old_emb_node)
{
  for (unsigned int i=0; i<num_edges; ++i)
    edges[i]->switchNode(new_emb_node, old_emb_node);
  for (unsigned int i = 0; i < interior_nodes.size(); ++i)
    interior_nodes[i]->switchNode(new_emb_node, old_emb_node);
  for (unsigned int i=0; i<fragments.size(); ++i)
    fragments[i]->switchNode(new_emb_node, old_emb_node);
}

unsigned int
EFAelement::id() const
{
  return _id;
}

bool
EFAelement::is_partial()
{
  bool partial = false;
  if (fragments.size() > 0)
  {
    for (unsigned int i = 0; i < num_nodes; ++i)
    {
      bool node_in_frag = false;
      for (unsigned int j = 0; j < fragments.size(); ++j)
      {
        if (fragments[j]->containsNode(nodes[i]))
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

bool
EFAelement::overlays_elem(EFAelement* other_elem)
{
  bool overlays = false;
  //Find common nodes
  std::set<EFAnode*> e1nodes(nodes.begin(), nodes.end());
  std::set<EFAnode*> e2nodes(other_elem->nodes.begin(), other_elem->nodes.end());
  std::vector<EFAnode*> common_nodes;
  std::set_intersection(e1nodes.begin(), e1nodes.end(),
                        e2nodes.begin(), e2nodes.end(),
                        std::inserter(common_nodes,common_nodes.end()));

  //Find indices of common nodes
  if (common_nodes.size() == 2)
  {
    std::vector<EFAnode*> common_nodes_vec(common_nodes.begin(),common_nodes.end());

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
      mooseError("in overlays_elem() couldn't find common node");

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
        mooseError("in overlays_elem() common nodes must be adjacent to each other");
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
      mooseError("in overlays_elem() couldn't find common node");

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
EFAelement::overlays_elem(EFAnode* other_edge_node1, EFAnode* other_edge_node2)
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
  for (unsigned int i=0; i<num_edges; ++i)
  {
    for (unsigned int j=0; j<edge_neighbors[i].size(); ++j)
    {
      if (edge_neighbors[i][j] == neighbor_elem)
        return i;
    }
  }
  mooseError("in get_neighbor_index() element: "<<_id<<" does not have neighbor: "<<neighbor_elem->id());
}

unsigned int
EFAelement::get_num_edge_neighbors(unsigned int edge_id)
{
  unsigned int num_neighbors = 0;
  if (edge_neighbors[edge_id][0])
    num_neighbors = edge_neighbors[edge_id].size();
  return num_neighbors;
}

void
EFAelement::add_crack_tip_neighbor(EFAelement * neighbor_elem)
{
  unsigned int neighbor_index = get_neighbor_index(neighbor_elem);
  for (unsigned int i=0; i<crack_tip_neighbors.size(); ++i)
  {
    if (crack_tip_neighbors[i] == neighbor_index)
    {
      mooseError("in add_crack_tip_neighbor() element: "<<_id
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
    mooseError("in add_crack_tip_neighbor() element: "
                <<_id<<" cannot have more than 2 crack tip neighbors");
  }
}

bool
EFAelement::will_crack_tip_extend(std::vector<unsigned int> &split_neighbors)
{
  // N.B. this is called at the beginning of createChildElements
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
          mooseError("in will_crack_tip_extend() element: "<<_id<<" has: "
                      <<edge_neighbors[neigh_idx].size()<<" on edge: "<<neigh_idx);
        }
        EFAelement * neighbor_elem = edge_neighbors[neigh_idx][0];
        if (neighbor_elem->fragments.size() > 2)
        {
          mooseError("in will_crack_tip_extend() element: "<<neighbor_elem->id()<<" has: "
                           <<neighbor_elem->fragments.size()<<" fragments");
        }
        else if (neighbor_elem->fragments.size() == 2)
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
EFAelement::get_nodes_on_edge(unsigned int edge_idx,
                                          std::vector<EFAnode*> &edge_nodes)
{
  edge_nodes.push_back(edges[edge_idx]->get_node(0));
  edge_nodes.push_back(edges[edge_idx]->get_node(1));
}

void
EFAelement::get_non_physical_nodes(std::set<EFAnode*> &non_physical_nodes)
{
  //Any nodes that don't belong to any fragment are non-physical
  //First add all nodes in the element to the set
  for (unsigned int i=0; i<nodes.size(); ++i)
    non_physical_nodes.insert(nodes[i]);

  //Now delete any nodes that are contained in fragments
  std::set<EFAnode*>::iterator sit;
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
EFAelement::shouldDuplicateCrackTipSplitElem()
{
  bool should_duplicate = false;
  if (fragments.size() == 1)
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
                std::vector<EFAnode*> edge_nodes;
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
  if (fragments.size() == 1 && (!crack_tip_split_element))
  {
    for (unsigned int i = 0; i < num_edges; ++i)
    {
      std::set<EFAnode*> phantom_nodes = getPhantomNodeOnEdge(i);
      if (phantom_nodes.size() > 0 && get_num_edge_neighbors(i) == 1)
      {
        EFAelement * neighbor_elem = edge_neighbors[i][0];
        if (neighbor_elem->fragments.size() > 1) // neighbor will be split
        {
          for (unsigned int j = 0; j < neighbor_elem->num_edges; ++j)
          {
            if (!neighbor_elem->edges[j]->isOverlapping(*edges[i]) &&
                neighbor_elem->get_num_edge_neighbors(j) > 0)
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

std::set<EFAnode*>
EFAelement::getPhantomNodeOnEdge(unsigned int edge_id)
{
  std::set<EFAnode*> phantom_nodes;
  if (fragments.size() > 0)
  {
    for (unsigned int j = 0; j < 2; ++j) // loop ove 2 edge nodes
    {
      bool node_in_frag = false;
      for (unsigned int k = 0; k < fragments.size(); ++k)
      {
        if (fragments[k]->containsNode(edges[edge_id]->get_node(j)))
        {
          node_in_frag = true;
          break;
        }
      }
      if (!node_in_frag)
        phantom_nodes.insert(edges[edge_id]->get_node(j));
    } // j
  }
  return phantom_nodes;
}

EFAnode *
EFAelement::create_local_node_from_global_node(const EFAnode * global_node) const
{
  if (global_node->category() != N_CATEGORY_PERMANENT &&
      global_node->category() != N_CATEGORY_TEMP)
    mooseError("In create_local_node_from_global_node node is not global");

  EFAnode * new_local_node = NULL;
  unsigned int inode = 0;
  for (; inode < nodes.size(); ++inode)
  {
    if (nodes[inode] == global_node)
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
  if (local_node->category() != N_CATEGORY_LOCAL_INDEX)
    mooseError("In get_global_node_from_local_node node passed in is not local");

  EFAnode * global_node = nodes[local_node->id()];

  if (global_node->category() != N_CATEGORY_PERMANENT &&
      global_node->category() != N_CATEGORY_TEMP)
    mooseError("In get_global_node_from_local_node, the node stored by the element is not global");

  return global_node;
}

void
EFAelement::getMasterInfo(EFAnode* node, std::vector<EFAnode*> &master_nodes,
                                      std::vector<double> &master_weights) const
{
  master_nodes.clear();
  master_weights.clear();
  bool masters_found = false;
  for (unsigned int i = 0; i < num_edges; ++i) // check element exterior edges
  {
    if (edges[i]->containsNode(node))
    {
      masters_found = edges[i]->getNodeMasters(node,master_nodes,master_weights);
      if (masters_found)
        break;
      else
        mooseError("In getMasterInfo: cannot find master nodes in element edges");
    }
  }

  if (!masters_found) // check element interior embedded nodes
  {
    for (unsigned int i = 0; i < interior_nodes.size(); ++i)
    {
      if (interior_nodes[i]->get_node() == node)
      {
        double node_xi[4][2] = {{-1.0,-1.0},{1.0,-1.0},{1.0,1.0},{-1.0,1.0}};
        double emb_xi  = interior_nodes[i]->get_para_coords(0);
        double emb_eta = interior_nodes[i]->get_para_coords(1);
        for (unsigned int j = 0; j < num_nodes; ++j)
        {
          master_nodes.push_back(nodes[j]);
          double weight = 0.0;
          if (num_nodes == 4)
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
  for (unsigned int i = 0; i < num_nodes; ++i)
  {
    if (nodes[i] == node)
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
EFAelement::getFragmentEdgeID(unsigned int elem_edge_id, unsigned int &frag_id, 
                                          unsigned int &frag_edge_id)
{
  // find the fragment edge that coincides with the given element edge
  bool frag_edge_found = false;
  frag_id = 99999;
  frag_edge_id = 99999;
  if (fragments.size() > 0)
  {
    for (unsigned int i = 0; i < fragments.size(); ++i)
    {
      for (unsigned int j = 0; j < fragments[i]->boundary_edges.size(); ++j)
      {
        if (edges[elem_edge_id]->isOverlapping(*fragments[i]->boundary_edges[j]))
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
  if (fragments.size() > 0)
  {
    if (fragments.size() != 1)
      mooseError("in is_edge_phantom() an element has more than 1 fragment");
    EFAnode * edge_node1 = edges[edge_id]->get_node(0);
    EFAnode * edge_node2 = edges[edge_id]->get_node(1);
    if ((!fragments[0]->containsNode(edge_node1)) &&
        (!fragments[0]->containsNode(edge_node2)))
      is_phantom =true;
  }
  return is_phantom;
}

bool
EFAelement::frag_has_tip_edges()
{
  bool has_tip_edges = false;
  if (fragments.size() == 1)
  {
    for (unsigned int i = 0; i < num_edges; ++i)
    {
      unsigned int num_frag_edges = 0; // count how many fragment edges this element edge contains
      if (edges[i]->has_intersection())
      {
        for (unsigned int j = 0; j < fragments[0]->boundary_edges.size(); ++j)
        {
          if (edges[i]->containsEdge(*fragments[0]->boundary_edges[j]))
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
  if (fragments.size() > 0) // crack tip element with a partial fragment saved
  {
    for (unsigned int i = 0; i < num_edges; ++i)
    {
      unsigned int num_frag_edges = 0; // count how many fragment edges this element edge contains
      if (edges[i]->has_intersection())
      {
        for (unsigned int j = 0; j < fragments[0]->boundary_edges.size(); ++j)
        {
          if (edges[i]->containsEdge(*fragments[0]->boundary_edges[j]))
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
      for (unsigned int i = 0; i < num_edges; ++i)
      {
        if (edges[i]->has_intersection())
        {
          tip_edge_id = i;
          break;
        }
      }
    }
  }
  return tip_edge_id;
}

bool
EFAelement::getEmbeddedNodeParaCoor(EFAnode* embedded_node, std::vector<double> &para_coor)
{
  unsigned int edge_id = 99999;
  bool edge_found = false;
  for (unsigned int i = 0; i < num_edges; ++i)
  {
    if (edges[i]->get_embedded_node() == embedded_node)
    {
      edge_id = i;
      edge_found = true;
      break;
    }
  }
  if (edge_found)
  {
    EFAnode* edge_node1 = edges[edge_id]->get_node(0);
    double xi_1d = 2.0*edges[edge_id]->get_intersection(edge_node1) -1.0;
    mapParaCoorFrom1Dto2D(edge_id, xi_1d, para_coor);
  }
  return edge_found;
}

unsigned int
EFAelement::getNumInteriorNodes()
{
  return interior_nodes.size();
}

unsigned int
EFAelement::get_num_cuts()
{
  unsigned int num_cuts = 0;
  for (unsigned int i = 0; i < num_edges; ++i)
    if (edges[i]->has_intersection())
      num_cuts += 1;
  return num_cuts;
}

bool
EFAelement::is_cut_twice()
{
  bool cut_twice = false;
  if (fragments.size() > 0)
  {
    unsigned int num_interior_edges = 0;
    for (unsigned int i = 0; i < fragments[0]->boundary_edges.size(); ++i)
    {
      if (fragments[0]->boundary_edges[i]->is_interior_edge())
        num_interior_edges += 1;
    }
    if (num_interior_edges == 2)
      cut_twice = true;
  }
  return cut_twice;
}

void
EFAelement::display_nodes()
{
  std::cout << "***** display nodes for element " << _id << " *****" << std::endl;
  for (unsigned int i = 0; i < num_nodes; ++i)
    std::cout << "addr " << nodes[i] << ", ID " << nodes[i]->id() << ", category " << nodes[i]->category() << std::endl;
}

void
EFAelement::mapParaCoorFrom1Dto2D(unsigned int edge_id, double xi_1d, std::vector<double> &para_coor)
{
  para_coor.resize(2,0.0);
  if (num_edges == 4)
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

template <class T>
unsigned int
EFAelement::num_common_elems(std::set<T> &v1, std::set<T> &v2)
{
  std::vector<T> common_elems;
  std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(),
                        std::inserter(common_elems, common_elems.end()));
  return common_elems.size();
}

