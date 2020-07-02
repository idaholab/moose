//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EFAElement2D.h"

#include <iomanip>

#include "EFAFaceNode.h"
#include "EFANode.h"
#include "EFAEdge.h"
#include "EFAFace.h"
#include "EFAFragment2D.h"
#include "EFAFuncs.h"
#include "EFAError.h"
#include "XFEMFuncs.h"

EFAElement2D::EFAElement2D(unsigned int eid, unsigned int n_nodes) : EFAElement(eid, n_nodes)
{
  if (n_nodes == 4 || n_nodes == 8 || n_nodes == 9)
    _num_edges = 4;
  else if (n_nodes == 3 || n_nodes == 6)
    _num_edges = 3;
  else
    EFAError("In EFAelement2D the supported element types are QUAD4, QUAD8, QUAD9, TRI3 and TRI6");
  setLocalCoordinates();
  _edges = std::vector<EFAEdge *>(_num_edges, NULL);
  _edge_neighbors =
      std::vector<std::vector<EFAElement2D *>>(_num_edges, std::vector<EFAElement2D *>(1, NULL));
}

EFAElement2D::EFAElement2D(const EFAElement2D * from_elem, bool convert_to_local)
  : EFAElement(from_elem->_id, from_elem->_num_nodes),
    _num_edges(from_elem->_num_edges),
    _edges(_num_edges, NULL),
    _edge_neighbors(_num_edges, std::vector<EFAElement2D *>(1, NULL))
{
  if (convert_to_local)
  {
    // build local nodes from global nodes
    for (unsigned int i = 0; i < _num_nodes; ++i)
    {
      if (from_elem->_nodes[i]->category() == EFANode::N_CATEGORY_PERMANENT ||
          from_elem->_nodes[i]->category() == EFANode::N_CATEGORY_TEMP ||
          from_elem->_nodes[i]->category() == EFANode::N_CATEGORY_EMBEDDED_PERMANENT)
      {
        _nodes[i] = from_elem->createLocalNodeFromGlobalNode(from_elem->_nodes[i]);
        _local_nodes.push_back(_nodes[i]); // convenient to delete local nodes
      }
      else
        EFAError("In EFAelement2D ",
                 from_elem->id(),
                 " the copy constructor must have from_elem w/ global nodes. node: ",
                 i,
                 " category: ",
                 from_elem->_nodes[i]->category());
    }

    // copy edges, fragments and interior nodes from from_elem
    for (unsigned int i = 0; i < _num_edges; ++i)
      _edges[i] = new EFAEdge(*from_elem->_edges[i]);
    for (unsigned int i = 0; i < from_elem->_fragments.size(); ++i)
      _fragments.push_back(new EFAFragment2D(this, true, from_elem, i));
    for (unsigned int i = 0; i < from_elem->_interior_nodes.size(); ++i)
      _interior_nodes.push_back(new EFAFaceNode(*from_elem->_interior_nodes[i]));

    // replace all global nodes with local nodes
    for (unsigned int i = 0; i < _num_nodes; ++i)
    {
      if (_nodes[i]->category() == EFANode::N_CATEGORY_LOCAL_INDEX)
        switchNode(
            _nodes[i],
            from_elem->_nodes[i],
            false); // when save to _cut_elem_map, the EFAelement is not a child of any parent
      else
        EFAError("In EFAelement2D copy constructor this elem's nodes must be local");
    }

    _local_node_coor = from_elem->_local_node_coor;
  }
  else
    EFAError("this EFAelement2D constructor only converts global nodes to local nodes");
}

EFAElement2D::EFAElement2D(const EFAFace * from_face)
  : EFAElement(0, from_face->numNodes()),
    _num_edges(from_face->numEdges()),
    _edges(_num_edges, NULL),
    _edge_neighbors(_num_edges, std::vector<EFAElement2D *>(1, NULL))
{
  for (unsigned int i = 0; i < _num_nodes; ++i)
    _nodes[i] = from_face->getNode(i);
  for (unsigned int i = 0; i < _num_edges; ++i)
    _edges[i] = new EFAEdge(*from_face->getEdge(i));
  for (unsigned int i = 0; i < from_face->numInteriorNodes(); ++i)
    _interior_nodes.push_back(new EFAFaceNode(*from_face->getInteriorNode(i)));
}

EFAElement2D::~EFAElement2D()
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

void
EFAElement2D::setLocalCoordinates()
{
  if (_num_edges == 4)
  {
    /*
                3     6     2
    QUAD9(QUAD8): o-----o-----o
                  |           |
                  |     8     |
                7 o     o     o 5
                  |           |
                  |           |
                  o-----o-----o
                  0     4     1

    */
    _local_node_coor.resize(_num_nodes);
    _local_node_coor[0] = EFAPoint(0.0, 0.0, 0.0);
    _local_node_coor[1] = EFAPoint(1.0, 0.0, 0.0);
    _local_node_coor[2] = EFAPoint(1.0, 1.0, 0.0);
    _local_node_coor[3] = EFAPoint(0.0, 1.0, 0.0);

    if (_num_nodes > 4)
    {
      _local_node_coor[4] = EFAPoint(0.5, 0.0, 0.0);
      _local_node_coor[5] = EFAPoint(1.0, 0.5, 0.0);
      _local_node_coor[6] = EFAPoint(0.5, 1.0, 0.0);
      _local_node_coor[7] = EFAPoint(0.0, 0.5, 0.0);
    }

    if (_num_nodes > 8)
      _local_node_coor[8] = EFAPoint(0.5, 0.5, 0.0);
  }
  else
  {
    /*
      TRI6: 2
            o
            | \
            |   \
          5 o     o 4
            |       \
            |         \
            o-----o-----o
            0     3     1
    */
    _local_node_coor.resize(_num_nodes);
    _local_node_coor[0] = EFAPoint(0.0, 0.0, 0.0);
    _local_node_coor[1] = EFAPoint(1.0, 0.0, 0.0);
    _local_node_coor[2] = EFAPoint(0.0, 1.0, 0.0);

    if (_num_nodes > 3)
    {
      _local_node_coor[3] = EFAPoint(0.5, 0.0, 0.0);
      _local_node_coor[4] = EFAPoint(0.5, 0.5, 0.0);
      _local_node_coor[5] = EFAPoint(0.0, 0.5, 0.0);
    }
  }
}

unsigned int
EFAElement2D::numFragments() const
{
  return _fragments.size();
}

bool
EFAElement2D::isPartial() const
{
  bool partial = false;
  if (_fragments.size() > 0)
  {
    for (unsigned int i = 0; i < _num_edges; ++i)
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
EFAElement2D::getNonPhysicalNodes(std::set<EFANode *> & non_physical_nodes) const
{
  // Any nodes that don't belong to any fragment are non-physical
  // First add all nodes in the element to the set
  for (unsigned int i = 0; i < _nodes.size(); ++i)
    non_physical_nodes.insert(_nodes[i]);

  // Now delete any nodes that are contained in fragments
  std::set<EFANode *>::iterator sit;
  for (sit = non_physical_nodes.begin(); sit != non_physical_nodes.end();)
  {
    bool erased = false;
    for (unsigned int i = 0; i < _fragments.size(); ++i)
    {
      if (_fragments[i]->containsNode(*sit))
      {
        non_physical_nodes.erase(sit++);
        erased = true;
        break;
      }
    }
    if (!erased)
      ++sit;
  }
}

void
EFAElement2D::switchNode(EFANode * new_node, EFANode * old_node, bool descend_to_parent)
{
  // We are not switching any embedded nodes here
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
    _parent->switchNode(new_node, old_node, false);
    for (unsigned int i = 0; i < _parent->numGeneralNeighbors(); ++i)
    {
      EFAElement * neigh_elem = _parent->getGeneralNeighbor(i); // generalized neighbor element
      for (unsigned int k = 0; k < neigh_elem->numChildren(); ++k)
        neigh_elem->getChild(k)->switchNode(new_node, old_node, false);
    }
  }
}

void
EFAElement2D::switchEmbeddedNode(EFANode * new_emb_node, EFANode * old_emb_node)
{
  for (unsigned int i = 0; i < _num_edges; ++i)
    _edges[i]->switchNode(new_emb_node, old_emb_node);
  for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    _interior_nodes[i]->switchNode(new_emb_node, old_emb_node);
  for (unsigned int i = 0; i < _fragments.size(); ++i)
    _fragments[i]->switchNode(new_emb_node, old_emb_node);
}

void
EFAElement2D::getMasterInfo(EFANode * node,
                            std::vector<EFANode *> & master_nodes,
                            std::vector<double> & master_weights) const
{
  // Given a EFANode, find the element edge or fragment edge that contains it
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
        for (unsigned int j = 0; j < _num_edges; ++j)
        {
          master_nodes.push_back(_nodes[j]);
          double weight = 0.0;
          if (_num_edges == 4)
            weight = Efa::linearQuadShape2D(j, emb_xi);
          else if (_num_edges == 3)
            weight = Efa::linearTriShape2D(j, emb_xi);
          else
            EFAError("unknown 2D element");
          master_weights.push_back(weight);
        }
        masters_found = true;
        break;
      }
    }
  }

  if (!masters_found)
    EFAError("In EFAelement2D::getMaterInfo, cannot find the given EFAnode");
}

unsigned int
EFAElement2D::numInteriorNodes() const
{
  return _interior_nodes.size();
}

bool
EFAElement2D::overlaysElement(const EFAElement2D * other_elem) const
{
  bool overlays = false;

  if (other_elem->numEdges() != _num_edges)
    return overlays;

  std::vector<EFANode *> common_nodes = getCommonNodes(other_elem);

  // Find indices of common nodes
  if (common_nodes.size() == 2)
  {
    std::vector<EFANode *> common_nodes_vec(common_nodes.begin(), common_nodes.end());

    unsigned int e1n1idx = _num_edges + 1;
    unsigned int e1n2idx = _num_edges + 1;
    for (unsigned int i = 0; i < _num_edges; ++i)
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

    if (e1n1idx > _num_edges || e1n2idx > _num_edges)
      EFAError("in overlays_elem() couldn't find common node");

    bool e1ascend = false;
    unsigned int e1n1idx_plus1(e1n1idx < (_num_edges - 1) ? e1n1idx + 1 : 0);
    unsigned int e1n1idx_minus1(e1n1idx > 0 ? e1n1idx - 1 : _num_edges - 1);
    if (e1n2idx == e1n1idx_plus1)
    {
      e1ascend = true;
    }
    else
    {
      if (e1n2idx != e1n1idx_minus1)
        EFAError("in overlays_elem() common nodes must be adjacent to each other");
    }

    unsigned int e2n1idx = _num_edges + 1;
    unsigned int e2n2idx = _num_edges + 1;
    for (unsigned int i = 0; i < _num_edges; ++i)
    {
      if (other_elem->getNode(i) == common_nodes_vec[0])
      {
        e2n1idx = i;
      }
      else if (other_elem->getNode(i) == common_nodes_vec[1])
      {
        e2n2idx = i;
      }
    }
    if (e2n1idx > other_elem->numNodes() || e2n2idx > other_elem->numNodes())
      EFAError("in overlays_elem() couldn't find common node");

    bool e2ascend = false;
    unsigned int e2n1idx_plus1(e2n1idx < (_num_edges - 1) ? e2n1idx + 1 : 0);
    unsigned int e2n1idx_minus1(e2n1idx > 0 ? e2n1idx - 1 : _num_edges - 1);
    if (e2n2idx == e2n1idx_plus1)
    {
      e2ascend = true;
    }
    else
    {
      if (e2n2idx != e2n1idx_minus1)
        EFAError("in overlays_elem() common nodes must be adjacent to each other");
    }

    // if indices both ascend or descend, they overlay
    if ((e1ascend && e2ascend) || (!e1ascend && !e2ascend))
    {
      overlays = true;
    }
  }
  else if (common_nodes.size() > 2)
  {
    // TODO: We probably need more error checking here.
    overlays = true;
  }
  return overlays;
}

unsigned int
EFAElement2D::getNeighborIndex(const EFAElement * neighbor_elem) const
{
  for (unsigned int i = 0; i < _num_edges; ++i)
    for (unsigned int j = 0; j < _edge_neighbors[i].size(); ++j)
      if (_edge_neighbors[i][j] == neighbor_elem)
        return i;

  EFAError(
      "in get_neighbor_index() element: ", _id, " does not have neighbor: ", neighbor_elem->id());
}

void
EFAElement2D::clearNeighbors()
{
  _general_neighbors.clear();
  for (unsigned int edge_iter = 0; edge_iter < _num_edges; ++edge_iter)
    _edge_neighbors[edge_iter] = std::vector<EFAElement2D *>(1, NULL);
}

void
EFAElement2D::setupNeighbors(std::map<EFANode *, std::set<EFAElement *>> & InverseConnectivityMap)
{
  findGeneralNeighbors(InverseConnectivityMap);
  for (unsigned int eit2 = 0; eit2 < _general_neighbors.size(); ++eit2)
  {
    EFAElement2D * neigh_elem = dynamic_cast<EFAElement2D *>(_general_neighbors[eit2]);
    if (!neigh_elem)
      EFAError("neighbor_elem is not of EFAelement2D type");

    std::vector<EFANode *> common_nodes = getCommonNodes(neigh_elem);
    if (common_nodes.size() >= 2)
    {
      for (unsigned int edge_iter = 0; edge_iter < _num_edges; ++edge_iter)
      {
        std::set<EFANode *> edge_nodes = getEdgeNodes(edge_iter);
        bool is_edge_neighbor = false;

        // Must share nodes on this edge
        if (Efa::numCommonElems(edge_nodes, common_nodes) == 2 && (!overlaysElement(neigh_elem)))
        {
          // Fragments must match up.
          if ((_fragments.size() > 1) || (neigh_elem->numFragments() > 1))
            EFAError("in updateEdgeNeighbors: Cannot have more than 1 fragment");
          else if ((_fragments.size() == 1) && (neigh_elem->numFragments() == 1))
          {
            if (_fragments[0]->isConnected(neigh_elem->getFragment(0)))
              is_edge_neighbor = true;
          }
          else // If there are no fragments to match up, consider them neighbors
            is_edge_neighbor = true;
        }

        if (is_edge_neighbor)
        {
          if (_edge_neighbors[edge_iter][0])
          {
            if (_edge_neighbors[edge_iter].size() > 1)
            {
              EFAError("Element ",
                       _id,
                       " already has 2 edge neighbors: ",
                       _edge_neighbors[edge_iter][0]->id(),
                       " ",
                       _edge_neighbors[edge_iter][1]->id());
            }
            _edge_neighbors[edge_iter].push_back(neigh_elem);
          }
          else
            _edge_neighbors[edge_iter][0] = neigh_elem;
        }
      }
    }
  }
}

void
EFAElement2D::neighborSanityCheck() const
{
  for (unsigned int edge_iter = 0; edge_iter < _num_edges; ++edge_iter)
  {
    for (unsigned int en_iter = 0; en_iter < _edge_neighbors[edge_iter].size(); ++en_iter)
    {
      EFAElement2D * neigh_elem = _edge_neighbors[edge_iter][en_iter];
      if (neigh_elem != NULL)
      {
        bool found_neighbor = false;
        for (unsigned int edge_iter2 = 0; edge_iter2 < neigh_elem->numEdges(); ++edge_iter2)
        {
          for (unsigned int en_iter2 = 0; en_iter2 < neigh_elem->numEdgeNeighbors(edge_iter2);
               ++en_iter2)
          {
            if (neigh_elem->getEdgeNeighbor(edge_iter2, en_iter2) == this)
            {
              if ((en_iter2 > 1) && (en_iter > 1))
                EFAError(
                    "Element and neighbor element cannot both have >1 neighbors on a common edge");
              found_neighbor = true;
              break;
            }
          }
        }
        if (!found_neighbor)
          EFAError("Neighbor element doesn't recognize current element as neighbor");
      }
    }
  }
}

void
EFAElement2D::initCrackTip(std::set<EFAElement *> & CrackTipElements)
{
  if (isCrackTipElement())
  {
    CrackTipElements.insert(this);
    for (unsigned int edge_iter = 0; edge_iter < _num_edges; ++edge_iter)
    {
      if ((_edge_neighbors[edge_iter].size() == 2) && (_edges[edge_iter]->hasIntersection()))
      {
        // Neither neighbor overlays current element.  We are on the uncut element ahead of the tip.
        // Flag neighbors as crack tip split elements and add this element as their crack tip
        // neighbor.
        if (_edge_neighbors[edge_iter][0]->overlaysElement(this) ||
            _edge_neighbors[edge_iter][1]->overlaysElement(this))
          EFAError("Element has a neighbor that overlays itself");

        // Make sure the current elment hasn't been flagged as a tip element
        if (_crack_tip_split_element)
          EFAError("crack_tip_split_element already flagged.  In elem: ",
                   _id,
                   " flags: ",
                   _crack_tip_split_element,
                   " ",
                   _edge_neighbors[edge_iter][0]->isCrackTipSplit(),
                   " ",
                   _edge_neighbors[edge_iter][1]->isCrackTipSplit());

        _edge_neighbors[edge_iter][0]->setCrackTipSplit();
        _edge_neighbors[edge_iter][1]->setCrackTipSplit();

        _edge_neighbors[edge_iter][0]->addCrackTipNeighbor(this);
        _edge_neighbors[edge_iter][1]->addCrackTipNeighbor(this);
      }
    } // edge_iter
  }
}

unsigned int
EFAElement2D::getCrackTipSplitElementID() const
{
  if (isCrackTipElement())
  {
    for (unsigned int edge_iter = 0; edge_iter < _num_edges; ++edge_iter)
    {
      if ((_edge_neighbors[edge_iter].size() == 2) && (_edges[edge_iter]->hasIntersection()))
      {
        if (_edge_neighbors[edge_iter][0] != NULL &&
            _edge_neighbors[edge_iter][0]->isCrackTipSplit())
        {
          return _edge_neighbors[edge_iter][0]->id();
        }
      }
    }
  }
  EFAError("In getCrackTipSplitElementID could not find element id");
  return 0;
}

bool
EFAElement2D::shouldDuplicateForCrackTip(const std::set<EFAElement *> & CrackTipElements)
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
    std::set<EFAElement *>::iterator sit;
    sit = CrackTipElements.find(this);
    if (sit == CrackTipElements.end() && isCrackTipElement())
      should_duplicate = true;
    else if (shouldDuplicateCrackTipSplitElement(CrackTipElements))
      should_duplicate = true;
    else if (shouldDuplicateForPhantomCorner())
      should_duplicate = true;
  }
  return should_duplicate;
}

bool
EFAElement2D::shouldDuplicateCrackTipSplitElement(const std::set<EFAElement *> & CrackTipElements)
{
  // Determine whether element at crack tip should be duplicated.  It should be duplicated
  // if the crack will extend into the next element, or if it has a non-physical node
  // connected to a face where a crack terminates, but will extend.

  bool should_duplicate = false;
  if (_fragments.size() == 1)
  {
    std::vector<unsigned int> split_neighbors;
    if (willCrackTipExtend(split_neighbors))
      should_duplicate = true;
    else
    {
      // The element may not be at the crack tip, but could have a non-physical node
      // connected to a crack tip face (on a neighbor element) that will be split.  We need to
      // duplicate in that case as well.
      std::set<EFANode *> non_physical_nodes;
      getNonPhysicalNodes(non_physical_nodes);

      for (unsigned int eit = 0; eit < _general_neighbors.size(); ++eit)
      {
        EFAElement2D * neigh_elem = dynamic_cast<EFAElement2D *>(_general_neighbors[eit]);
        if (!neigh_elem)
          EFAError("general elem is not of type EFAelement2D");

        // check if a general neighbor is an old crack tip element and will be split
        std::set<EFAElement *>::iterator sit;
        sit = CrackTipElements.find(neigh_elem);
        if (sit != CrackTipElements.end() && neigh_elem->numFragments() > 1)
        {
          for (unsigned int i = 0; i < neigh_elem->numEdges(); ++i)
          {
            std::set<EFANode *> neigh_edge_nodes = neigh_elem->getEdgeNodes(i);
            if (neigh_elem->numEdgeNeighbors(i) == 2 &&
                Efa::numCommonElems(neigh_edge_nodes, non_physical_nodes) > 0)
            {
              should_duplicate = true;
              break;
            }
          } // i
        }
        if (should_duplicate)
          break;
      } // eit
    }
  } // IF one fragment
  return should_duplicate;
}

bool
EFAElement2D::shouldDuplicateForPhantomCorner()
{
  // if a partial element will be split for a second time and it has two neighbor elements
  // sharing one phantom node with the aforementioned partial element, then the two neighbor
  // elements should be duplicated
  bool should_duplicate = false;
  if (_fragments.size() == 1 && (!_crack_tip_split_element))
  {
    for (unsigned int i = 0; i < _num_edges; ++i)
    {
      std::set<EFANode *> phantom_nodes = getPhantomNodeOnEdge(i);
      if (phantom_nodes.size() > 0 && numEdgeNeighbors(i) == 1)
      {
        EFAElement2D * neighbor_elem = _edge_neighbors[i][0];
        if (neighbor_elem->numFragments() > 1) // neighbor will be split
        {
          for (unsigned int j = 0; j < neighbor_elem->numEdges(); ++j)
          {
            if (!neighbor_elem->getEdge(j)->equivalent(*_edges[i]) &&
                neighbor_elem->numEdgeNeighbors(j) > 0)
            {
              std::set<EFANode *> neigh_phantom_nodes = neighbor_elem->getPhantomNodeOnEdge(j);
              if (Efa::numCommonElems(phantom_nodes, neigh_phantom_nodes) > 0)
              {
                should_duplicate = true;
                break;
              }
            }
          } // j
        }
      }
      if (should_duplicate)
        break;
    } // i
  }
  return should_duplicate;
}

bool
EFAElement2D::willCrackTipExtend(std::vector<unsigned int> & split_neighbors) const
{
  // Determine whether the current element is a crack tip element for which the crack will
  // extend into the next element.
  // N.B. this is called at the beginning of createChildElements
  bool will_extend = false;
  if (_fragments.size() == 1 && _crack_tip_split_element)
  {
    for (unsigned int i = 0; i < _crack_tip_neighbors.size(); ++i)
    {
      unsigned int neigh_idx = _crack_tip_neighbors[i];
      if (numEdgeNeighbors(neigh_idx) != 1)
        EFAError("in will_crack_tip_extend() element: ",
                 _id,
                 " has: ",
                 _edge_neighbors[neigh_idx].size(),
                 " on edge: ",
                 neigh_idx);

      EFAElement2D * neighbor_elem = _edge_neighbors[neigh_idx][0];
      if (neighbor_elem->numFragments() > 2)
        EFAError("in will_crack_tip_extend() element: ",
                 neighbor_elem->id(),
                 " has: ",
                 neighbor_elem->numFragments(),
                 " fragments");
      else if (neighbor_elem->numFragments() == 2)
      {
        EFAFragment2D * frag1 = neighbor_elem->getFragment(0);
        EFAFragment2D * frag2 = neighbor_elem->getFragment(1);
        std::vector<EFANode *> neigh_cut_nodes = frag1->getCommonNodes(frag2);
        if (neigh_cut_nodes.size() != 2)
          EFAError("2 frags in a elem does not share 2 common nodes");
        if (_edges[neigh_idx]->isEmbeddedNode(neigh_cut_nodes[0]) ||
            _edges[neigh_idx]->isEmbeddedNode(neigh_cut_nodes[1]))
        {
          split_neighbors.push_back(neigh_idx);
          will_extend = true;
        }
      }
    } // i
  }
  return will_extend;
}

bool
EFAElement2D::isCrackTipElement() const
{
  return fragmentHasTipEdges();
}

unsigned int
EFAElement2D::getNumCuts() const
{
  unsigned int num_cuts = 0;
  for (unsigned int i = 0; i < _num_edges; ++i)
    if (_edges[i]->hasIntersection())
      num_cuts += _edges[i]->numEmbeddedNodes();
  return num_cuts;
}

bool
EFAElement2D::isFinalCut() const
{
  // if an element has been cut twice its fragment must have two interior edges
  bool cut_twice = false;
  if (_fragments.size() > 0)
  {
    unsigned int num_interior_edges = 0;
    for (unsigned int i = 0; i < _fragments[0]->numEdges(); ++i)
    {
      if (_fragments[0]->isEdgeInterior(i))
        num_interior_edges += 1;
    }
    if (num_interior_edges == 2)
      cut_twice = true;
  }
  return cut_twice;
}

void
EFAElement2D::updateFragments(const std::set<EFAElement *> & CrackTipElements,
                              std::map<unsigned int, EFANode *> & EmbeddedNodes)
{
  // combine the crack-tip edges in a fragment to a single intersected edge
  std::set<EFAElement *>::iterator sit;
  sit = CrackTipElements.find(this);
  if (sit != CrackTipElements.end()) // curr_elem is a crack tip element
  {
    if (_fragments.size() == 1)
      _fragments[0]->combineTipEdges();
    else
      EFAError("crack tip elem ", _id, " must have 1 fragment");
  }

  // if a fragment only has 1 intersection which is in an interior edge
  // remove this embedded node (MUST DO THIS AFTER combine_tip_edges())
  if (_fragments.size() == 1)
    _fragments[0]->removeInvalidEmbeddedNodes(EmbeddedNodes);

  // for an element with no fragment, create one fragment identical to the element
  if (_fragments.size() == 0)
    _fragments.push_back(new EFAFragment2D(this, true, this));
  if (_fragments.size() != 1)
    EFAError("Element ", _id, " must have 1 fragment at this point");

  // count fragment's cut edges
  unsigned int num_cut_frag_edges = _fragments[0]->getNumCuts();
  unsigned int num_cut_nodes = _fragments[0]->getNumCutNodes();
  unsigned int num_frag_edges = _fragments[0]->numEdges();
  if (num_cut_frag_edges > 3)
    EFAError("In element ", _id, " there are more than 2 cut fragment edges");

  if (num_cut_frag_edges == 0 && num_cut_nodes == 0)
  {
    if (!isPartial()) // delete the temp frag for an uncut elem
    {
      delete _fragments[0];
      _fragments.clear();
    }
    // Element has already been cut. Don't recreate fragments because we
    // would create multiple fragments to cover the entire element and
    // lose the information about what part of this element is physical.
    return;
  }

  // split one fragment into one, two or three new fragments
  std::vector<EFAFragment2D *> new_frags;
  if (num_cut_frag_edges == 3)
    new_frags = branchingSplit(EmbeddedNodes);
  else
    new_frags = _fragments[0]->split();

  delete _fragments[0]; // delete the old fragment
  _fragments.clear();
  for (unsigned int i = 0; i < new_frags.size(); ++i)
    _fragments.push_back(new_frags[i]);

  fragmentSanityCheck(num_frag_edges, num_cut_frag_edges);
}

void
EFAElement2D::fragmentSanityCheck(unsigned int n_old_frag_edges, unsigned int n_old_frag_cuts) const
{
  if (n_old_frag_cuts > 3)
    EFAError("Sanity check: in element ", _id, " frag has more than 3 cut edges");

  // count permanent and embedded nodes for new fragments
  std::vector<unsigned int> num_emb;
  std::vector<unsigned int> num_perm;
  std::vector<unsigned int> num_emb_perm;
  for (unsigned int i = 0; i < _fragments.size(); ++i)
  {
    num_emb.push_back(0);
    num_perm.push_back(0);
    num_emb_perm.push_back(0);
    std::set<EFANode *> perm_nodes;
    std::set<EFANode *> emb_nodes;
    std::set<EFANode *> emb_perm_nodes;
    for (unsigned int j = 0; j < _fragments[i]->numEdges(); ++j)
    {
      for (unsigned int k = 0; k < 2; ++k)
      {
        EFANode * temp_node = _fragments[i]->getEdge(j)->getNode(k);
        if (temp_node->category() == EFANode::N_CATEGORY_PERMANENT)
          perm_nodes.insert(temp_node);
        else if (temp_node->category() == EFANode::N_CATEGORY_EMBEDDED)
          emb_nodes.insert(temp_node);
        else if (temp_node->category() == EFANode::N_CATEGORY_EMBEDDED_PERMANENT)
          emb_perm_nodes.insert(temp_node);
        else
          EFAError("Invalid node category");
      }
    }
    num_perm[i] = perm_nodes.size();
    num_emb[i] = emb_nodes.size();
    num_emb_perm[i] = emb_perm_nodes.size();
  }

  // TODO: For cut-node case, how to check fragment sanity
  for (unsigned int i = 0; i < _fragments.size(); ++i)
    if (num_emb_perm[i] != 0)
      return;

  unsigned int n_interior_nodes = numInteriorNodes();
  if (n_interior_nodes > 0 && n_interior_nodes != 1)
    EFAError("After update_fragments this element has ", n_interior_nodes, " interior nodes");

  if (n_old_frag_cuts == 0)
  {
    if (_fragments.size() != 1 || _fragments[0]->numEdges() != n_old_frag_edges)
      EFAError("Incorrect link size for element with 0 cuts");
  }
  else if (n_old_frag_cuts == 1) // crack tip case
  {
    if (_fragments.size() != 1 || _fragments[0]->numEdges() != n_old_frag_edges + 1)
      EFAError("Incorrect link size for element with 1 cut");
  }
  else if (n_old_frag_cuts == 2)
  {
    if (_fragments.size() != 2 ||
        (_fragments[0]->numEdges() + _fragments[1]->numEdges()) != n_old_frag_edges + 4)
      EFAError("Incorrect link size for element with 2 cuts");
  }
  else if (n_old_frag_cuts == 3)
  {
    if (_fragments.size() != 3 || (_fragments[0]->numEdges() + _fragments[1]->numEdges() +
                                   _fragments[2]->numEdges()) != n_old_frag_edges + 9)
      EFAError("Incorrect link size for element with 3 cuts");
  }
  else
    EFAError("Unexpected number of old fragment cuts");
}

void
EFAElement2D::restoreFragment(const EFAElement * const from_elem)
{
  const EFAElement2D * from_elem2d = dynamic_cast<const EFAElement2D *>(from_elem);
  if (!from_elem2d)
    EFAError("from_elem is not of EFAelement2D type");

  // restore fragments
  if (_fragments.size() != 0)
    EFAError("in restoreFragmentInfo elements must not have any pre-existing fragments");
  for (unsigned int i = 0; i < from_elem2d->numFragments(); ++i)
    _fragments.push_back(new EFAFragment2D(this, true, from_elem2d, i));

  // restore interior nodes
  if (_interior_nodes.size() != 0)
    EFAError("in restoreFragmentInfo elements must not have any pre-exsiting interior nodes");
  for (unsigned int i = 0; i < from_elem2d->_interior_nodes.size(); ++i)
    _interior_nodes.push_back(new EFAFaceNode(*from_elem2d->_interior_nodes[i]));

  // restore edge intersections
  if (getNumCuts() != 0)
    EFAError("In restoreEdgeIntersection: edge cuts already exist in element ", _id);
  for (unsigned int i = 0; i < _num_edges; ++i)
  {
    if (from_elem2d->_edges[i]->hasIntersection())
      _edges[i]->copyIntersection(*from_elem2d->_edges[i], 0);
    if (_edges[i]->numEmbeddedNodes() > 2)
      EFAError("elem ", _id, " has an edge with >2 cuts");
  }

  // replace all local nodes with global nodes
  for (unsigned int i = 0; i < from_elem2d->numNodes(); ++i)
  {
    if (from_elem2d->_nodes[i]->category() == EFANode::N_CATEGORY_LOCAL_INDEX)
      switchNode(
          _nodes[i], from_elem2d->_nodes[i], false); // EFAelement is not a child of any parent
    else
      EFAError("In restoreFragmentInfo all of from_elem's nodes must be local");
  }
}

void
EFAElement2D::createChild(const std::set<EFAElement *> & CrackTipElements,
                          std::map<unsigned int, EFAElement *> & Elements,
                          std::map<unsigned int, EFAElement *> & newChildElements,
                          std::vector<EFAElement *> & ChildElements,
                          std::vector<EFAElement *> & ParentElements,
                          std::map<unsigned int, EFANode *> & TempNodes)
{
  if (_children.size() != 0)
    EFAError("Element cannot have existing children in createChildElements");

  bool shouldDuplicateForCutNodeElement = false;
  for (unsigned int j = 0; j < _num_nodes; ++j)
  {
    if (_nodes[j]->category() == EFANode::N_CATEGORY_EMBEDDED_PERMANENT)
      shouldDuplicateForCutNodeElement = true;
  }

  if (_fragments.size() > 1 || shouldDuplicateForCrackTip(CrackTipElements) ||
      shouldDuplicateForCutNodeElement)
  {
    if (_fragments.size() > 3)
      EFAError("More than 3 fragments not yet supported");

    // set up the children
    ParentElements.push_back(this);
    for (unsigned int ichild = 0; ichild < _fragments.size(); ++ichild)
    {
      unsigned int new_elem_id;
      if (newChildElements.size() == 0)
        new_elem_id = Efa::getNewID(Elements);
      else
        new_elem_id = Efa::getNewID(newChildElements);

      EFAElement2D * childElem = new EFAElement2D(new_elem_id, this->numNodes());
      newChildElements.insert(std::make_pair(new_elem_id, childElem));

      ChildElements.push_back(childElem);
      childElem->setParent(this);
      _children.push_back(childElem);

      std::vector<EFAPoint> local_embedded_node_coor;

      for (unsigned int i = 0; i < this->getFragment(ichild)->numEdges(); ++i)
      {
        if (this->getFragment(ichild)->isEdgeInterior(i))
        {
          std::vector<EFANode *> master_nodes;
          std::vector<double> master_weights;

          for (unsigned int j = 0; j < 2; ++j)
          {
            this->getMasterInfo(
                this->getFragmentEdge(ichild, i)->getNode(j), master_nodes, master_weights);
            EFAPoint coor(0.0, 0.0, 0.0);
            for (unsigned int k = 0; k < master_nodes.size(); ++k)
            {
              EFANode * local = this->createLocalNodeFromGlobalNode(master_nodes[k]);
              coor += _local_node_coor[local->id()] * master_weights[k];
              delete local;
            }
            local_embedded_node_coor.push_back(coor);
          }
        }
      }

      EFAPoint normal(0.0, 0.0, 0.0);
      EFAPoint origin(0.0, 0.0, 0.0);
      EFAPoint normal2(0.0, 0.0, 0.0);
      EFAPoint origin2(0.0, 0.0, 0.0);

      if (local_embedded_node_coor.size())
      {
        EFAPoint cut_line = local_embedded_node_coor[1] - local_embedded_node_coor[0];
        normal = EFAPoint(cut_line(1), -cut_line(0), 0.0);
        Xfem::normalizePoint(normal);
        origin = (local_embedded_node_coor[0] + local_embedded_node_coor[1]) * 0.5;
      }

      if (local_embedded_node_coor.size() == 4)
      {
        EFAPoint cut_line = local_embedded_node_coor[3] - local_embedded_node_coor[2];
        normal2 = EFAPoint(cut_line(1), -cut_line(0), 0.0);
        Xfem::normalizePoint(normal2);
        origin2 = (local_embedded_node_coor[2] + local_embedded_node_coor[3]) * 0.5;
      }

      // get child element's nodes
      for (unsigned int j = 0; j < _num_nodes; ++j)
      {
        EFAPoint p(0.0, 0.0, 0.0);
        p = _local_node_coor[j];

        EFAPoint origin_to_point = p - origin;
        EFAPoint origin2_to_point = p - origin2;

        if (_fragments.size() == 1 &&
            _nodes[j]->category() == EFANode::N_CATEGORY_EMBEDDED_PERMANENT) // create temp node for
                                                                             // embedded permanent
                                                                             // node
        {
          unsigned int new_node_id = Efa::getNewID(TempNodes);
          EFANode * newNode = new EFANode(new_node_id, EFANode::N_CATEGORY_TEMP, _nodes[j]);
          TempNodes.insert(std::make_pair(new_node_id, newNode));
          childElem->setNode(j, newNode); // be a temp node
        }
        else if (_fragments.size() == 1 && !shouldDuplicateForCrackTip(CrackTipElements))
        {
          childElem->setNode(j, _nodes[j]); // inherit parent's node
        }
        else if (std::abs(origin_to_point * normal) < Xfem::tol &&
                 _fragments.size() > 1) // cut through node case
        {
          unsigned int new_node_id = Efa::getNewID(TempNodes);
          EFANode * newNode = new EFANode(new_node_id, EFANode::N_CATEGORY_TEMP, _nodes[j]);
          TempNodes.insert(std::make_pair(new_node_id, newNode));
          childElem->setNode(j, newNode); // be a temp node
        }
        else if (origin_to_point * normal < Xfem::tol && origin2_to_point * normal2 < Xfem::tol &&
                 (_fragments.size() > 1 || shouldDuplicateForCrackTip(CrackTipElements)))
        {
          childElem->setNode(j, _nodes[j]); // inherit parent's node
        }
        else if (normal.norm() < Xfem::tol && normal2.norm() < Xfem::tol &&
                 _fragments.size() == 1) // cut along edge case
        {
          childElem->setNode(j, _nodes[j]); // inherit parent's node
        }
        else if ((_fragments.size() > 1 ||
                  shouldDuplicateForCrackTip(
                      CrackTipElements))) // parent element's node is not in fragment
        {
          unsigned int new_node_id = Efa::getNewID(TempNodes);
          EFANode * newNode = new EFANode(new_node_id, EFANode::N_CATEGORY_TEMP, _nodes[j]);
          TempNodes.insert(std::make_pair(new_node_id, newNode));
          childElem->setNode(j, newNode); // be a temp node
        }
      }

      // get child element's fragments
      EFAFragment2D * new_frag = new EFAFragment2D(childElem, true, this, ichild);
      childElem->_fragments.push_back(new_frag);

      // get child element's edges
      for (unsigned int j = 0; j < _num_edges; ++j)
      {
        unsigned int jplus1(j < (_num_edges - 1) ? j + 1 : 0);
        EFAEdge * new_edge = new EFAEdge(childElem->getNode(j), childElem->getNode(jplus1));
        if (_edges[j]->hasIntersection())
          new_edge->copyIntersection(*_edges[j], 0);
        if ((_num_edges == 4 && _num_nodes > 4) || (_num_edges == 3 && _num_nodes > 3))
          new_edge->setInteriorNode(childElem->getNode(_num_edges + j));
        childElem->setEdge(j, new_edge);
      }
      childElem->removePhantomEmbeddedNode(); // IMPORTANT

      // inherit old interior nodes
      for (unsigned int j = 0; j < _interior_nodes.size(); ++j)
        childElem->_interior_nodes.push_back(new EFAFaceNode(*_interior_nodes[j]));
    }
  }
  else // num_links == 1 || num_links == 0
    // child is itself - but don't insert into the list of ChildElements!!!
    _children.push_back(this);
}

void
EFAElement2D::removePhantomEmbeddedNode()
{
  // remove the embedded nodes on edge that are not inside the real part
  if (_fragments.size() > 0)
  {
    for (unsigned int i = 0; i < _num_edges; ++i)
    {
      std::vector<EFANode *> nodes_to_delete;
      for (unsigned int j = 0; j < _edges[i]->numEmbeddedNodes(); ++j)
      {
        if (!_fragments[0]->containsNode(_edges[i]->getEmbeddedNode(j)))
          nodes_to_delete.push_back(_edges[i]->getEmbeddedNode(j));
      }
      for (unsigned int j = 0; j < nodes_to_delete.size(); ++j)
        _edges[i]->removeEmbeddedNode(nodes_to_delete[j]);
    } // i
  }
}

void
EFAElement2D::connectNeighbors(std::map<unsigned int, EFANode *> & PermanentNodes,
                               std::map<unsigned int, EFANode *> & TempNodes,
                               std::map<EFANode *, std::set<EFAElement *>> & InverseConnectivityMap,
                               bool merge_phantom_edges)
{
  // N.B. "this" must point to a child element that was just created
  if (!_parent)
    EFAError("no parent element for child element ", _id, " in connect_neighbors");
  EFAElement2D * parent2d = dynamic_cast<EFAElement2D *>(_parent);
  if (!parent2d)
    EFAError("cannot dynamic cast to parent2d in connect_neighbors");

  // First loop through edges and merge nodes with neighbors as appropriate
  for (unsigned int j = 0; j < _num_edges; ++j)
  {
    for (unsigned int k = 0; k < parent2d->numEdgeNeighbors(j); ++k)
    {
      EFAElement2D * NeighborElem = parent2d->getEdgeNeighbor(j, k);
      unsigned int neighbor_edge_id = NeighborElem->getNeighborIndex(parent2d);

      if (_edges[j]->hasIntersection())
      {
        for (unsigned int l = 0; l < NeighborElem->numChildren(); ++l)
        {
          EFAElement2D * childOfNeighborElem =
              dynamic_cast<EFAElement2D *>(NeighborElem->getChild(l));
          if (!childOfNeighborElem)
            EFAError("dynamic cast childOfNeighborElem fails");

          // Check to see if the nodes are already merged.  There's nothing else to do in that case.
          EFAEdge * neighborChildEdge = childOfNeighborElem->getEdge(neighbor_edge_id);
          if (_edges[j]->equivalent(*neighborChildEdge))
            continue;

          if (_fragments[0]->isConnected(childOfNeighborElem->getFragment(0)))
          {
            unsigned int num_edge_nodes = 2;
            for (unsigned int i = 0; i < num_edge_nodes; ++i)
            {
              unsigned int childNodeIndex = i;
              unsigned int neighborChildNodeIndex = num_edge_nodes - 1 - childNodeIndex;

              EFANode * childNode = _edges[j]->getNode(childNodeIndex);
              EFANode * childOfNeighborNode = neighborChildEdge->getNode(neighborChildNodeIndex);

              mergeNodes(
                  childNode, childOfNeighborNode, childOfNeighborElem, PermanentNodes, TempNodes);
            }

            EFANode * childNode = _edges[j]->getInteriorNode();
            EFANode * childOfNeighborNode = neighborChildEdge->getInteriorNode();

            mergeNodes(
                childNode, childOfNeighborNode, childOfNeighborElem, PermanentNodes, TempNodes);
          }
        } // l, loop over NeighborElem's children
      }
      else // No edge intersection -- optionally merge non-material nodes if they share a common
           // parent
      {
        if (merge_phantom_edges)
        {
          for (unsigned int l = 0; l < NeighborElem->numChildren(); ++l)
          {
            EFAElement2D * childOfNeighborElem =
                dynamic_cast<EFAElement2D *>(NeighborElem->getChild(l));
            if (!childOfNeighborElem)
              EFAError("dynamic cast childOfNeighborElem fails");

            EFAEdge * neighborChildEdge = childOfNeighborElem->getEdge(neighbor_edge_id);
            if (!neighborChildEdge->hasIntersection()) // neighbor edge must NOT have
                                                       // intersection either
            {
              // Check to see if the nodes are already merged.  There's nothing else to do in that
              // case.
              unsigned int num_edge_nodes = 2;
              for (unsigned int i = 0; i < num_edge_nodes; ++i)
              {
                unsigned int childNodeIndex = i;
                unsigned int neighborChildNodeIndex = num_edge_nodes - 1 - childNodeIndex;

                EFANode * childNode = _edges[j]->getNode(childNodeIndex);
                EFANode * childOfNeighborNode = neighborChildEdge->getNode(neighborChildNodeIndex);

                if (childNode->parent() != NULL &&
                    childNode->parent() ==
                        childOfNeighborNode
                            ->parent()) // non-material node and both come from same parent
                  mergeNodes(childNode,
                             childOfNeighborNode,
                             childOfNeighborElem,
                             PermanentNodes,
                             TempNodes);
              } // i
            }
          } // loop over NeighborElem's children
        }   // if (merge_phantom_edges)
      }     // IF edge-j has_intersection()
    }       // k, loop over neighbors on edge j
  }         // j, loop over all edges

  // Now do a second loop through edges and convert remaining nodes to permanent nodes.
  // Important: temp nodes are not shared by any neighbor, so no need to switch nodes on neighbors
  for (unsigned int j = 0; j < _num_nodes; ++j)
  {
    EFANode * childNode = _nodes[j];
    if (childNode->category() == EFANode::N_CATEGORY_TEMP)
    {
      // if current child element does not have siblings, and if current temp node is a lone one
      // this temp node should be merged back to its parent permanent node. Otherwise we would have
      // permanent nodes that are not connected to any element
      std::set<EFAElement *> patch_elems = InverseConnectivityMap[childNode->parent()];
      if (parent2d->numFragments() == 1 && patch_elems.size() == 1)
        switchNode(childNode->parent(), childNode, false);
      else
      {
        unsigned int new_node_id = Efa::getNewID(PermanentNodes);
        EFANode * newNode =
            new EFANode(new_node_id, EFANode::N_CATEGORY_PERMANENT, childNode->parent());
        PermanentNodes.insert(std::make_pair(new_node_id, newNode));
        switchNode(newNode, childNode, false);
      }
      if (!Efa::deleteFromMap(TempNodes, childNode))
        EFAError(
            "Attempted to delete node: ", childNode->id(), " from TempNodes, but couldn't find it");
    }
  }
}

void
EFAElement2D::updateFragmentNode()
{
  for (unsigned int j = 0; j < _num_nodes; ++j)
  {
    if (_nodes[j]->parent() != NULL &&
        _nodes[j]->parent()->category() == EFANode::N_CATEGORY_EMBEDDED_PERMANENT)
      switchNode(_nodes[j], _nodes[j]->parent(), false);
  }
}

void
EFAElement2D::printElement(std::ostream & ostream) const
{
  ostream << std::setw(4);
  ostream << _id << " | ";
  for (unsigned int j = 0; j < _num_nodes; ++j)
  {
    ostream << std::setw(5) << _nodes[j]->idCatString();
  }

  ostream << "  | ";
  for (unsigned int j = 0; j < _num_edges; ++j)
  {
    ostream << std::setw(4);
    if (_edges[j]->hasIntersection())
    {
      if (_edges[j]->numEmbeddedNodes() > 1)
      {
        ostream << "[";
        for (unsigned int k = 0; k < _edges[j]->numEmbeddedNodes(); ++k)
        {
          ostream << _edges[j]->getEmbeddedNode(k)->id();
          if (k == _edges[j]->numEmbeddedNodes() - 1)
            ostream << "]";
          else
            ostream << " ";
        }
      }
      else
        ostream << _edges[j]->getEmbeddedNode(0)->id() << " ";
    }
    else
      ostream << "  -- ";
  }
  ostream << "  | ";
  for (unsigned int j = 0; j < _num_edges; ++j)
  {
    if (numEdgeNeighbors(j) > 1)
    {
      ostream << "[";
      for (unsigned int k = 0; k < numEdgeNeighbors(j); ++k)
      {
        ostream << getEdgeNeighbor(j, k)->id();
        if (k == numEdgeNeighbors(j) - 1)
          ostream << "]";
        else
          ostream << " ";
      }
      ostream << " ";
    }
    else
    {
      ostream << std::setw(4);
      if (numEdgeNeighbors(j) == 1)
        ostream << getEdgeNeighbor(j, 0)->id() << " ";
      else
        ostream << "  -- ";
    }
  }
  ostream << "  | ";
  for (unsigned int j = 0; j < _fragments.size(); ++j)
  {
    ostream << std::setw(4);
    ostream << " " << j << " | ";
    for (unsigned int k = 0; k < _fragments[j]->numEdges(); ++k)
    {
      EFANode * prt_node = getFragmentEdge(j, k)->getNode(0);
      unsigned int kprev(k > 0 ? k - 1 : _fragments[j]->numEdges() - 1);
      if (!getFragmentEdge(j, kprev)->containsNode(prt_node))
        prt_node = getFragmentEdge(j, k)->getNode(1);
      ostream << std::setw(5) << prt_node->idCatString();
    }
  }
  ostream << std::endl;
}

EFAFragment2D *
EFAElement2D::getFragment(unsigned int frag_id) const
{
  if (frag_id < _fragments.size())
    return _fragments[frag_id];
  else
    EFAError("frag_id out of bounds");
}

std::set<EFANode *>
EFAElement2D::getEdgeNodes(unsigned int edge_id) const
{
  std::set<EFANode *> edge_nodes;
  edge_nodes.insert(_edges[edge_id]->getNode(0));
  edge_nodes.insert(_edges[edge_id]->getNode(1));
  return edge_nodes;
}

bool
EFAElement2D::getEdgeNodeParametricCoordinate(EFANode * node, std::vector<double> & para_coor) const
{
  // get the parametric coords of a node in an element edge
  unsigned int edge_id = std::numeric_limits<unsigned int>::max();
  bool edge_found = false;
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
    mapParametricCoordFrom1Dto2D(edge_id, xi_1d, para_coor);
  }
  return edge_found;
}

EFAFaceNode *
EFAElement2D::getInteriorNode(unsigned int interior_node_id) const
{
  if (interior_node_id < _interior_nodes.size())
    return _interior_nodes[interior_node_id];
  else
    EFAError("interior_node_id out of bounds");
}

void
EFAElement2D::deleteInteriorNodes()
{
  for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    delete _interior_nodes[i];
  _interior_nodes.clear();
}

unsigned int
EFAElement2D::numEdges() const
{
  return _num_edges;
}

void
EFAElement2D::setEdge(unsigned int edge_id, EFAEdge * edge)
{
  _edges[edge_id] = edge;
}

void
EFAElement2D::createEdges()
{
  for (unsigned int i = 0; i < _num_edges; ++i)
  {
    unsigned int i_plus1(i < (_num_edges - 1) ? i + 1 : 0);
    EFAEdge * new_edge = new EFAEdge(_nodes[i], _nodes[i_plus1]);

    if ((_num_edges == 4 && _num_nodes > 4) || (_num_edges == 3 && _num_nodes > 3))
      new_edge->setInteriorNode(
          _nodes[i + _num_edges]); // '_num_edges' is the offset of interior edge node numbering

    _edges[i] = new_edge;
  }
}

EFAEdge *
EFAElement2D::getEdge(unsigned int edge_id) const
{
  return _edges[edge_id];
}

EFAEdge *
EFAElement2D::getFragmentEdge(unsigned int frag_id, unsigned int edge_id) const
{
  if (frag_id < _fragments.size())
    return _fragments[frag_id]->getEdge(edge_id);
  else
    EFAError("frag_id out of bounds in get_frag_edge()");
}

std::set<EFANode *>
EFAElement2D::getPhantomNodeOnEdge(unsigned int edge_id) const
{
  std::set<EFANode *> phantom_nodes;
  if (_fragments.size() > 0)
  {
    for (unsigned int j = 0; j < 2; ++j) // loop ove 2 edge nodes
    {
      bool node_in_frag = false;
      for (unsigned int k = 0; k < _fragments.size(); ++k)
      {
        if (_fragments[k]->containsNode(_edges[edge_id]->getNode(j)))
        {
          node_in_frag = true;
          break;
        }
      }
      if (!node_in_frag)
        phantom_nodes.insert(_edges[edge_id]->getNode(j));
    }
  }
  return phantom_nodes;
}

bool
EFAElement2D::getFragmentEdgeID(unsigned int elem_edge_id, unsigned int & frag_edge_id) const
{
  // find the fragment edge that is contained by given element edge
  // N.B. if the elem edge contains two frag edges, this method will only return
  // the first frag edge ID
  bool frag_edge_found = false;
  frag_edge_id = std::numeric_limits<unsigned int>::max();
  if (_fragments.size() == 1)
  {
    for (unsigned int j = 0; j < _fragments[0]->numEdges(); ++j)
    {
      if (_edges[elem_edge_id]->containsEdge(*_fragments[0]->getEdge(j)))
      {
        frag_edge_id = j;
        frag_edge_found = true;
        break;
      }
    }
  }
  return frag_edge_found;
}

bool
EFAElement2D::isEdgePhantom(unsigned int edge_id) const
{
  bool is_phantom = false;
  if (_fragments.size() > 0)
  {
    bool contain_frag_edge = false;
    for (unsigned int i = 0; i < _fragments.size(); ++i)
    {
      for (unsigned int j = 0; j < _fragments[i]->numEdges(); ++j)
      {
        if (_edges[edge_id]->containsEdge(*_fragments[i]->getEdge(j)))
        {
          contain_frag_edge = true;
          break;
        }
      } // j
      if (contain_frag_edge)
        break;
    } // i
    if (!contain_frag_edge)
      is_phantom = true;
  }
  return is_phantom;
}

unsigned int
EFAElement2D::numEdgeNeighbors(unsigned int edge_id) const
{
  unsigned int num_neighbors = 0;
  if (_edge_neighbors[edge_id][0])
    num_neighbors = _edge_neighbors[edge_id].size();
  return num_neighbors;
}

EFAElement2D *
EFAElement2D::getEdgeNeighbor(unsigned int edge_id, unsigned int neighbor_id) const
{
  if (_edge_neighbors[edge_id][0] != NULL && neighbor_id < _edge_neighbors[edge_id].size())
    return _edge_neighbors[edge_id][neighbor_id];
  else
    EFAError("edge neighbor does not exist");
}

bool
EFAElement2D::fragmentHasTipEdges() const
{
  bool has_tip_edges = false;
  if (_fragments.size() == 1)
  {
    for (unsigned int i = 0; i < _num_edges; ++i)
    {
      unsigned int num_frag_edges = 0; // count how many fragment edges this element edge contains
      if (_edges[i]->hasIntersection())
      {
        for (unsigned int j = 0; j < _fragments[0]->numEdges(); ++j)
        {
          if (_edges[i]->containsEdge(*_fragments[0]->getEdge(j)))
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
EFAElement2D::getTipEdgeID() const
{
  // if this element is a crack tip element, returns the crack tip edge's ID
  unsigned int tip_edge_id = std::numeric_limits<unsigned int>::max();
  if (_fragments.size() == 1) // crack tip element with a partial fragment saved
  {
    for (unsigned int i = 0; i < _num_edges; ++i)
    {
      unsigned int num_frag_edges = 0; // count how many fragment edges this element edge contains
      if (_edges[i]->hasIntersection())
      {
        for (unsigned int j = 0; j < _fragments[0]->numEdges(); ++j)
        {
          if (_edges[i]->containsEdge(*_fragments[0]->getEdge(j)))
            num_frag_edges += 1;
        }
        if (num_frag_edges == 2) // element edge contains two fragment edges
        {
          tip_edge_id = i;
          break;
        }
      }
    }
  }
  return tip_edge_id;
}

EFANode *
EFAElement2D::getTipEmbeddedNode() const
{
  // if this element is a crack tip element, returns the crack tip edge's ID
  EFANode * tip_emb = NULL;
  if (_fragments.size() == 1) // crack tip element with a partial fragment saved
  {
    for (unsigned int i = 0; i < _num_edges; ++i)
    {
      std::vector<EFAEdge *> frag_edges; // count how many fragment edges this element edge contains
      if (_edges[i]->hasIntersection())
      {
        for (unsigned int j = 0; j < _fragments[0]->numEdges(); ++j)
        {
          if (_edges[i]->containsEdge(*_fragments[0]->getEdge(j)))
            frag_edges.push_back(_fragments[0]->getEdge(j));
        }                           // j
        if (frag_edges.size() == 2) // element edge contains two fragment edges
        {
          if (frag_edges[1]->containsNode(frag_edges[0]->getNode(1)))
            tip_emb = frag_edges[0]->getNode(1);
          else if (frag_edges[1]->containsNode(frag_edges[0]->getNode(0)))
            tip_emb = frag_edges[0]->getNode(0);
          else
            EFAError("Common node can't be found between 2 tip frag edges");
          break;
        }
      }
    }
  }
  return tip_emb;
}

bool
EFAElement2D::edgeContainsTip(unsigned int edge_id) const
{
  bool contains_tip = false;
  if (_fragments.size() == 1)
  {
    unsigned int num_frag_edges = 0; // count how many fragment edges this element edge contains
    if (_edges[edge_id]->hasIntersection())
    {
      for (unsigned int j = 0; j < _fragments[0]->numEdges(); ++j)
      {
        if (_edges[edge_id]->containsEdge(*_fragments[0]->getEdge(j)))
          num_frag_edges += 1;
      } // j
      if (num_frag_edges == 2)
        contains_tip = true;
    }
  }
  return contains_tip;
}

bool
EFAElement2D::fragmentEdgeAlreadyCut(unsigned int ElemEdgeID) const
{
  // when marking cuts, check if the corresponding frag edge already has been cut
  bool has_cut = false;
  if (edgeContainsTip(ElemEdgeID))
    has_cut = true;
  else
  {
    unsigned int FragEdgeID = std::numeric_limits<unsigned int>::max();
    if (getFragmentEdgeID(ElemEdgeID, FragEdgeID))
    {
      EFAEdge * frag_edge = getFragmentEdge(0, FragEdgeID);
      if (frag_edge->hasIntersection())
        has_cut = true;
    }
  }
  return has_cut;
}

void
EFAElement2D::addEdgeCut(unsigned int edge_id,
                         double position,
                         EFANode * embedded_node,
                         std::map<unsigned int, EFANode *> & EmbeddedNodes,
                         bool add_to_neighbor)
{
  EFANode * local_embedded = NULL;
  EFANode * edge_node1 = _edges[edge_id]->getNode(0);
  if (embedded_node) // use the existing embedded node if it was passed in
    local_embedded = embedded_node;

  if (_edges[edge_id]->hasIntersectionAtPosition(position, edge_node1) && position > Xfem::tol &&
      position < 1.0 - Xfem::tol)
  {
    unsigned int emb_id = _edges[edge_id]->getEmbeddedNodeIndex(position, edge_node1);
    EFANode * old_emb = _edges[edge_id]->getEmbeddedNode(emb_id);
    if (embedded_node && embedded_node != old_emb)
    {
      EFAError("Attempting to add edge intersection when one already exists with different node.",
               " elem: ",
               _id,
               " edge: ",
               edge_id,
               " position: ",
               position);
    }
    local_embedded = old_emb;
  }
  else // if no cut exists at the input position
  {
    bool add2elem = true;

    // check if it is necessary to add cuts to fragment
    // id of partially overlapping fragment edge
    unsigned int frag_edge_id = std::numeric_limits<unsigned int>::max();
    EFAEdge * frag_edge = NULL;
    EFANode * frag_edge_node1 = NULL;
    double frag_pos = -1.0;
    bool add2frag = false;

    if (getFragmentEdgeID(edge_id, frag_edge_id)) // elem edge contains a frag edge
    {
      frag_edge = getFragmentEdge(0, frag_edge_id);
      if (!fragmentEdgeAlreadyCut(edge_id))
      {
        double xi[2] = {-1.0, -1.0}; // relative coords of two frag edge nodes
        xi[0] = _edges[edge_id]->distanceFromNode1(frag_edge->getNode(0));
        xi[1] = _edges[edge_id]->distanceFromNode1(frag_edge->getNode(1));
        if ((position - xi[0]) * (position - xi[1]) <
            0.0) // the cut to be added is within the real part of the edge
        {
          frag_edge_node1 = frag_edge->getNode(0);
          frag_pos = (position - xi[0]) / (xi[1] - xi[0]);
          add2frag = true;
        }
        else                // the emb node to be added is in the phantom part of the elem edge
          add2elem = false; // DO NOT ADD INTERSECT IN THIS CASE
      }
      else
      {
        EFAWarning("attempting to add new cut to a cut fragment edge");
        add2elem = false; // DO NOT ADD INTERSECT IN THIS CASE
      }
    }

    // If elem edge has 2 cuts but they have not been restored yet, it's OK because
    // getFragmentEdgeID = false so we won't add anything to the restored fragment.
    // add to elem edge (IMPORTANT to do it AFTER the above fragment check)
    if (add2elem)
    {
      if (!local_embedded) // need to create new embedded node
      {
        unsigned int new_node_id = Efa::getNewID(EmbeddedNodes);
        local_embedded = new EFANode(new_node_id, EFANode::N_CATEGORY_EMBEDDED);
        EmbeddedNodes.insert(std::make_pair(new_node_id, local_embedded));
      }
      _edges[edge_id]->addIntersection(position, local_embedded, edge_node1);
      if (_edges[edge_id]->numEmbeddedNodes() > 2)
        EFAError("element edge can't have >2 embedded nodes");
    }

    // add to frag edge
    if (add2frag)
    {
      frag_edge->addIntersection(frag_pos, local_embedded, frag_edge_node1);
      if (frag_edge->numEmbeddedNodes() > 1)
        EFAError("fragment edge can't have >1 embedded nodes");
    }
  } // IF the input embedded node already exists on this elem edge

  if (add_to_neighbor)
  {
    for (unsigned int en_iter = 0; en_iter < numEdgeNeighbors(edge_id); ++en_iter)
    {
      EFAElement2D * edge_neighbor = getEdgeNeighbor(edge_id, en_iter);
      unsigned int neighbor_edge_id = edge_neighbor->getNeighborIndex(this);
      if (edge_neighbor->getEdge(neighbor_edge_id)->getNode(0) == edge_node1) // same direction
        EFAError("neighbor edge has the same direction as this edge");
      double neigh_pos = 1.0 - position; // get emb node's postion on neighbor edge
      edge_neighbor->addEdgeCut(neighbor_edge_id, neigh_pos, local_embedded, EmbeddedNodes, false);
    }
  } // If add_to_neighbor required
}

void
EFAElement2D::addNodeCut(unsigned int node_id,
                         EFANode * embedded_permanent_node,
                         std::map<unsigned int, EFANode *> & PermanentNodes,
                         std::map<unsigned int, EFANode *> & EmbeddedPermanentNodes)
{
  EFANode * local_embedded_permanent = NULL;
  EFANode * node = _nodes[node_id];
  if (embedded_permanent_node) // use the existing embedded node if it was passed in
    local_embedded_permanent = embedded_permanent_node;

  if (node->category() == EFANode::N_CATEGORY_PERMANENT)
  {
    node->setCategory(EFANode::N_CATEGORY_EMBEDDED_PERMANENT);
    local_embedded_permanent = node;
    EmbeddedPermanentNodes.insert(std::make_pair(node->id(), local_embedded_permanent));
    if (!Efa::deleteFromMap(PermanentNodes, local_embedded_permanent, false))
      EFAError("Attempted to delete node: ",
               local_embedded_permanent->id(),
               " from PermanentNodes, but couldn't find it");
  }
}

bool
EFAElement2D::addFragmentEdgeCut(unsigned int frag_edge_id,
                                 double position,
                                 std::map<unsigned int, EFANode *> & EmbeddedNodes)
{
  if (_fragments.size() != 1)
    EFAError("Element: ", _id, " should have only 1 fragment in addFragEdgeIntersection");
  EFANode * local_embedded = NULL;

  // check if this intersection coincide with any embedded node on this edge
  bool isValidIntersection = true;
  EFAEdge * frag_edge = getFragmentEdge(0, frag_edge_id); // we're considering this edge
  EFANode * edge_node1 = frag_edge->getNode(0);
  EFANode * edge_node2 = frag_edge->getNode(1);
  if ((std::abs(position) < Xfem::tol && edge_node1->category() == EFANode::N_CATEGORY_EMBEDDED) ||
      (std::abs(1.0 - position) < Xfem::tol &&
       edge_node2->category() == EFANode::N_CATEGORY_EMBEDDED))
    isValidIntersection = false;

  // TODO: do not allow to cut fragment's node
  if (std::abs(position) < Xfem::tol || std::abs(1.0 - position) < Xfem::tol)
    isValidIntersection = false;

  // add valid intersection point to an edge
  if (isValidIntersection)
  {
    if (frag_edge->hasIntersection())
    {
      if (!frag_edge->hasIntersectionAtPosition(position, edge_node1))
        EFAError("Attempting to add fragment edge intersection when one already exists with "
                 "different position.",
                 " elem: ",
                 _id,
                 " edge: ",
                 frag_edge_id,
                 " position: ",
                 position,
                 " old position: ",
                 frag_edge->getIntersection(0, edge_node1));
    }
    else // blank edge - in fact, it can only be a blank element interior edge
    {
      if (!_fragments[0]->isEdgeInterior(frag_edge_id) ||
          _fragments[0]->isSecondaryInteriorEdge(frag_edge_id))
        EFAError("Attemping to add intersection to an invalid fragment edge. Element: ",
                 _id,
                 " fragment_edge: ",
                 frag_edge_id);

      // create the embedded node and add it to the fragment's boundary edge
      unsigned int new_node_id = Efa::getNewID(EmbeddedNodes);
      local_embedded = new EFANode(new_node_id, EFANode::N_CATEGORY_EMBEDDED);
      EmbeddedNodes.insert(std::make_pair(new_node_id, local_embedded));
      frag_edge->addIntersection(position, local_embedded, edge_node1);

      // save this interior embedded node to FaceNodes
      // TODO: for unstructured elements, the following calution gives you inaccurate position of
      // face nodes
      // must solve this issue for 3D!
      std::vector<double> node1_para_coor(2, 0.0);
      std::vector<double> node2_para_coor(2, 0.0);
      if (getEdgeNodeParametricCoordinate(edge_node1, node1_para_coor) &&
          getEdgeNodeParametricCoordinate(edge_node2, node2_para_coor))
      {
        double xi = (1.0 - position) * node1_para_coor[0] + position * node2_para_coor[0];
        double eta = (1.0 - position) * node1_para_coor[1] + position * node2_para_coor[1];
        _interior_nodes.push_back(new EFAFaceNode(local_embedded, xi, eta));
      }
      else
        EFAError("elem: ", _id, " cannot get the parametric coords of two end embedded nodes");
    }
    // no need to add intersection for neighbor fragment - if this fragment has a
    // neighbor fragment, the neighbor has already been treated in addEdgeIntersection;
    // for an interior edge, there is no neighbor fragment
  }

  return isValidIntersection;
}

std::vector<EFAFragment2D *>
EFAElement2D::branchingSplit(std::map<unsigned int, EFANode *> & EmbeddedNodes)
{
  if (isPartial())
    EFAError("branching is only allowed for an uncut element");

  // collect all emb nodes counterclockwise
  std::vector<EFANode *> three_nodes;
  for (unsigned int i = 0; i < _edges.size(); ++i)
  {
    EFANode * node1 = _edges[i]->getNode(0);
    if (_edges[i]->numEmbeddedNodes() == 1)
      three_nodes.push_back(_edges[i]->getEmbeddedNode(0));
    else if (_edges[i]->numEmbeddedNodes() == 2)
    {
      unsigned int id0(
          _edges[i]->getIntersection(0, node1) < _edges[i]->getIntersection(1, node1) ? 0 : 1);
      unsigned int id1 = 1 - id0;
      three_nodes.push_back(_edges[i]->getEmbeddedNode(id0));
      three_nodes.push_back(_edges[i]->getEmbeddedNode(id1));
    }
  }
  if (three_nodes.size() != 3)
    EFAError("three_nodes.size() != 3");

  // get the parent coords of the braycenter of the three nodes
  // TODO: may need a better way to compute this "branching point"
  std::vector<double> center_xi(2, 0.0);
  for (unsigned int i = 0; i < 3; ++i)
  {
    std::vector<double> xi_2d(2, 0.0);
    getEdgeNodeParametricCoordinate(three_nodes[i], xi_2d);
    center_xi[0] += xi_2d[0];
    center_xi[1] += xi_2d[1];
  }
  center_xi[0] /= 3.0;
  center_xi[1] /= 3.0;

  // create a new interior node for current element
  unsigned int new_node_id = Efa::getNewID(EmbeddedNodes);
  EFANode * new_emb = new EFANode(new_node_id, EFANode::N_CATEGORY_EMBEDDED);
  EmbeddedNodes.insert(std::make_pair(new_node_id, new_emb));
  _interior_nodes.push_back(new EFAFaceNode(new_emb, center_xi[0], center_xi[1]));

  // generate the three fragments
  std::vector<EFAFragment2D *> new_fragments;
  for (unsigned int i = 0; i < 3; ++i) // loop over 3 sectors
  {
    EFAFragment2D * new_frag = new EFAFragment2D(this, false, NULL);
    unsigned int iplus1(i < 2 ? i + 1 : 0);
    new_frag->addEdge(new EFAEdge(three_nodes[iplus1], new_emb));
    new_frag->addEdge(new EFAEdge(new_emb, three_nodes[i]));

    unsigned int iedge = 0;
    bool add_more_edges = true;
    for (unsigned int j = 0; j < _edges.size(); ++j)
    {
      if (_edges[j]->containsNode(three_nodes[i]))
      {
        if (_edges[j]->containsNode(three_nodes[iplus1]))
        {
          new_frag->addEdge(new EFAEdge(three_nodes[i], three_nodes[iplus1]));
          add_more_edges = false;
        }
        else
        {
          new_frag->addEdge(new EFAEdge(three_nodes[i], _edges[j]->getNode(1)));
        }
        iedge = j;
        break;
      }
    } // j
    while (add_more_edges)
    {
      iedge += 1;
      if (iedge == _edges.size())
        iedge = 0;
      if (_edges[iedge]->containsNode(three_nodes[iplus1]))
      {
        new_frag->addEdge(new EFAEdge(_edges[iedge]->getNode(0), three_nodes[iplus1]));
        add_more_edges = false;
      }
      else
        new_frag->addEdge(new EFAEdge(_edges[iedge]->getNode(0), _edges[iedge]->getNode(1)));
    }
    new_fragments.push_back(new_frag);
  } // i
  return new_fragments;
}

void
EFAElement2D::mapParametricCoordFrom1Dto2D(unsigned int edge_id,
                                           double xi_1d,
                                           std::vector<double> & para_coor) const
{
  para_coor.resize(2, 0.0);
  if (_num_edges == 4)
  {
    if (edge_id == 0)
    {
      para_coor[0] = xi_1d;
      para_coor[1] = -1.0;
    }
    else if (edge_id == 1)
    {
      para_coor[0] = 1.0;
      para_coor[1] = xi_1d;
    }
    else if (edge_id == 2)
    {
      para_coor[0] = -xi_1d;
      para_coor[1] = 1.0;
    }
    else if (edge_id == 3)
    {
      para_coor[0] = -1.0;
      para_coor[1] = -xi_1d;
    }
    else
      EFAError("edge_id out of bounds");
  }
  else if (_num_edges == 3)
  {
    if (edge_id == 0)
    {
      para_coor[0] = 0.5 * (1.0 - xi_1d);
      para_coor[1] = 0.5 * (1.0 + xi_1d);
    }
    else if (edge_id == 1)
    {
      para_coor[0] = 0.0;
      para_coor[1] = 0.5 * (1.0 - xi_1d);
    }
    else if (edge_id == 2)
    {
      para_coor[0] = 0.5 * (1.0 + xi_1d);
      para_coor[1] = 0.0;
    }
    else
      EFAError("edge_id out of bounds");
  }
  else
    EFAError("unknown element for 2D");
}

std::vector<EFANode *>
EFAElement2D::getCommonNodes(const EFAElement2D * other_elem) const
{
  std::set<EFANode *> e1nodes(_nodes.begin(),
                              _nodes.begin() + _num_edges); // only account for corner nodes
  std::set<EFANode *> e2nodes(other_elem->_nodes.begin(), other_elem->_nodes.begin() + _num_edges);
  std::vector<EFANode *> common_nodes = Efa::getCommonElems(e1nodes, e2nodes);
  return common_nodes;
}
