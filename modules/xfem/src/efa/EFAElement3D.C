//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EFAElement3D.h"

#include <iomanip>

#include "EFAFaceNode.h"
#include "EFAVolumeNode.h"
#include "EFANode.h"
#include "EFAEdge.h"
#include "EFAFace.h"
#include "EFAFragment3D.h"
#include "EFAFuncs.h"
#include "EFAError.h"
#include "XFEMFuncs.h"

EFAElement3D::EFAElement3D(unsigned int eid, unsigned int n_nodes, unsigned int n_faces)
  : EFAElement(eid, n_nodes),
    _num_faces(n_faces),
    _faces(_num_faces, NULL),
    _face_neighbors(_num_faces),
    _face_edge_neighbors(_num_faces)
{
  if (_num_faces == 4)
  {
    _num_vertices = 4;
    if (_num_nodes == 10)
      _num_interior_face_nodes = 3;
    else if (_num_nodes == 4)
      _num_interior_face_nodes = 0;
    else
      EFAError("In EFAelement3D the supported TET element types are TET4 and TET10");
  }
  else if (_num_faces == 6)
  {
    _num_vertices = 8;
    if (_num_nodes == 27)
      _num_interior_face_nodes = 5;
    else if (_num_nodes == 20)
      _num_interior_face_nodes = 4;
    else if (_num_nodes == 8)
      _num_interior_face_nodes = 0;
    else
      EFAError("In EFAelement3D the supported HEX element types are HEX8, HEX20 and HEX27");
  }
  else
    EFAError("In EFAelement3D the supported element types are TET4, TET10, HEX8, HEX20 and HEX27");
  setLocalCoordinates();
}

EFAElement3D::EFAElement3D(const EFAElement3D * from_elem, bool convert_to_local)
  : EFAElement(from_elem->_id, from_elem->_num_nodes),
    _num_faces(from_elem->_num_faces),
    _faces(_num_faces, NULL),
    _face_neighbors(_num_faces),
    _face_edge_neighbors(_num_faces)
{
  if (convert_to_local)
  {
    // build local nodes from global nodes
    for (unsigned int i = 0; i < _num_nodes; ++i)
    {
      if (from_elem->_nodes[i]->category() == EFANode::N_CATEGORY_PERMANENT ||
          from_elem->_nodes[i]->category() == EFANode::N_CATEGORY_TEMP)
      {
        _nodes[i] = from_elem->createLocalNodeFromGlobalNode(from_elem->_nodes[i]);
        _local_nodes.push_back(_nodes[i]); // convenient to delete local nodes
      }
      else
        EFAError("In EFAelement3D ",
                 from_elem->id(),
                 " the copy constructor must have from_elem w/ global nodes. node: ",
                 i,
                 " category: ",
                 from_elem->_nodes[i]->category());
    }

    // copy faces, fragments and interior nodes from from_elem
    for (unsigned int i = 0; i < _num_faces; ++i)
      _faces[i] = new EFAFace(*from_elem->_faces[i]);
    for (unsigned int i = 0; i < from_elem->_fragments.size(); ++i)
      _fragments.push_back(new EFAFragment3D(this, true, from_elem, i));
    for (unsigned int i = 0; i < from_elem->_interior_nodes.size(); ++i)
      _interior_nodes.push_back(new EFAVolumeNode(*from_elem->_interior_nodes[i]));

    // replace all global nodes with local nodes
    for (unsigned int i = 0; i < _num_nodes; ++i)
    {
      if (_nodes[i]->category() == EFANode::N_CATEGORY_LOCAL_INDEX)
        switchNode(
            _nodes[i],
            from_elem->_nodes[i],
            false); // when save to _cut_elem_map, the EFAelement is not a child of any parent
      else
        EFAError("In EFAelement3D copy constructor this elem's nodes must be local");
    }

    // create element face connectivity array (IMPORTANT)
    findFacesAdjacentToFaces();

    _local_node_coor = from_elem->_local_node_coor;
    _num_interior_face_nodes = from_elem->_num_interior_face_nodes;
    _num_vertices = from_elem->_num_vertices;
  }
  else
    EFAError("this EFAelement3D constructor only converts global nodes to local nodes");
}

EFAElement3D::~EFAElement3D()
{
  for (unsigned int i = 0; i < _fragments.size(); ++i)
  {
    if (_fragments[i])
    {
      delete _fragments[i];
      _fragments[i] = NULL;
    }
  }
  for (unsigned int i = 0; i < _faces.size(); ++i)
  {
    if (_faces[i])
    {
      delete _faces[i];
      _faces[i] = NULL;
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
EFAElement3D::setLocalCoordinates()
{
  if (_num_faces == 6)
  {
    /*
    HEX27(HEX20):  7              18             6
                   o--------------o--------------o
                  /:             /              /|
                 / :            /              / |
                /  :           /              /  |
             19/   :        25/            17/   |
              o--------------o--------------o    |
             /     :        /              /|    |
            /    15o       /    23o       / |  14o
           /       :      /              /  |   /|
         4/        :   16/             5/   |  / |
         o--------------o--------------o    | /  |
         |         :    |   26         |    |/   |
         |  24o    :    |    o         |  22o    |
         |         :    |       10     |   /|    |
         |        3o....|.........o....|../.|....o
         |        .     |              | /  |   / 2
         |       .    21|            13|/   |  /
      12 o--------------o--------------o    | /
         |     .        |              |    |/
         |  11o         | 20o          |    o
         |   .          |              |   / 9
         |  .           |              |  /
         | .            |              | /
         |.             |              |/
         o--------------o--------------o
         0              8              1

     */
    _local_node_coor.resize(_num_nodes);
    _local_node_coor[0] = EFAPoint(0.0, 0.0, 0.0);
    _local_node_coor[1] = EFAPoint(1.0, 0.0, 0.0);
    _local_node_coor[2] = EFAPoint(1.0, 1.0, 0.0);
    _local_node_coor[3] = EFAPoint(0.0, 1.0, 0.0);
    _local_node_coor[4] = EFAPoint(0.0, 0.0, 1.0);
    _local_node_coor[5] = EFAPoint(1.0, 0.0, 1.0);
    _local_node_coor[6] = EFAPoint(1.0, 1.0, 1.0);
    _local_node_coor[7] = EFAPoint(0.0, 1.0, 1.0);

    if (_num_nodes > 8)
    {
      _local_node_coor[8] = EFAPoint(0.5, 0.0, 0.0);
      _local_node_coor[9] = EFAPoint(1.0, 0.5, 0.0);
      _local_node_coor[10] = EFAPoint(0.5, 1.0, 0.0);
      _local_node_coor[11] = EFAPoint(0.0, 0.5, 0.0);
      _local_node_coor[12] = EFAPoint(0.0, 0.0, 0.5);
      _local_node_coor[13] = EFAPoint(1.0, 0.0, 0.5);
      _local_node_coor[14] = EFAPoint(1.0, 1.0, 0.5);
      _local_node_coor[15] = EFAPoint(0.0, 1.0, 0.5);
      _local_node_coor[16] = EFAPoint(0.5, 0.0, 1.0);
      _local_node_coor[17] = EFAPoint(1.0, 0.5, 1.0);
      _local_node_coor[18] = EFAPoint(0.5, 1.0, 1.0);
      _local_node_coor[19] = EFAPoint(0.0, 0.5, 1.0);
    }

    if (_num_nodes > 20)
    {
      _local_node_coor[20] = EFAPoint(0.5, 0.5, 0.0);
      _local_node_coor[21] = EFAPoint(0.5, 0.0, 0.5);
      _local_node_coor[22] = EFAPoint(1.0, 0.5, 0.5);
      _local_node_coor[23] = EFAPoint(0.5, 1.0, 0.5);
      _local_node_coor[24] = EFAPoint(0.0, 0.5, 0.5);
      _local_node_coor[25] = EFAPoint(0.5, 0.5, 1.0);
      _local_node_coor[26] = EFAPoint(0.5, 0.5, 0.5);
    }
  }
  else if (_num_faces == 4)
  {
    /*
                  3
      TET10:      o
                 /|\
                / | \
            7  /  |  \9
              o   |   o
             /    |8   \
            /     o     \
           /    6 |      \
        0 o.....o.|.......o 2
           \      |      /
            \     |     /
             \    |    /
            4 o   |   o 5
               \  |  /
                \ | /
                 \|/
                  o
                  1

    */
    _local_node_coor.resize(_num_nodes);
    _local_node_coor[0] = EFAPoint(0.0, 0.0, 0.0);
    _local_node_coor[1] = EFAPoint(1.0, 0.0, 0.0);
    _local_node_coor[2] = EFAPoint(0.0, 1.0, 0.0);
    _local_node_coor[3] = EFAPoint(0.0, 0.0, 1.0);

    if (_num_nodes > 4)
    {
      _local_node_coor[4] = EFAPoint(0.5, 0.0, 0.0);
      _local_node_coor[5] = EFAPoint(0.5, 0.5, 0.0);
      _local_node_coor[6] = EFAPoint(0.0, 0.5, 0.0);
      _local_node_coor[7] = EFAPoint(0.0, 0.0, 0.5);
      _local_node_coor[8] = EFAPoint(0.5, 0.0, 0.5);
      _local_node_coor[9] = EFAPoint(0.0, 0.5, 0.5);
    }
  }
  else
    EFAError("EFAElement3D: number of faces should be either 4(TET) or 6(HEX).");
}

unsigned int
EFAElement3D::numFragments() const
{
  return _fragments.size();
}

bool
EFAElement3D::isPartial() const
{
  bool partial = false;
  if (_fragments.size() > 0)
  {
    for (unsigned int i = 0; i < _num_vertices; ++i)
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
EFAElement3D::getNonPhysicalNodes(std::set<EFANode *> & non_physical_nodes) const
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
EFAElement3D::switchNode(EFANode * new_node, EFANode * old_node, bool descend_to_parent)
{
  // We are not switching any embedded nodes here; This is an enhanced version
  for (unsigned int i = 0; i < _num_nodes; ++i)
  {
    if (_nodes[i] == old_node)
      _nodes[i] = new_node;
  }
  for (unsigned int i = 0; i < _fragments.size(); ++i)
    _fragments[i]->switchNode(new_node, old_node);

  for (unsigned int i = 0; i < _faces.size(); ++i)
    _faces[i]->switchNode(new_node, old_node);

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
EFAElement3D::switchEmbeddedNode(EFANode * new_emb_node, EFANode * old_emb_node)
{
  for (unsigned int i = 0; i < _num_faces; ++i)
    _faces[i]->switchNode(new_emb_node, old_emb_node);
  for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    _interior_nodes[i]->switchNode(new_emb_node, old_emb_node);
  for (unsigned int i = 0; i < _fragments.size(); ++i)
    _fragments[i]->switchNode(new_emb_node, old_emb_node);
}

void
EFAElement3D::updateFragmentNode()
{
  // In EFAElement3D, updateFragmentNode needs to be implemented
}

void
EFAElement3D::getMasterInfo(EFANode * node,
                            std::vector<EFANode *> & master_nodes,
                            std::vector<double> & master_weights) const
{
  // Given a EFAnode, return its master nodes and weights
  master_nodes.clear();
  master_weights.clear();
  bool masters_found = false;
  for (unsigned int i = 0; i < _num_faces; ++i) // check element exterior faces
  {
    if (_faces[i]->containsNode(node))
    {
      masters_found = _faces[i]->getMasterInfo(node, master_nodes, master_weights);
      if (masters_found)
        break;
      else
        EFAError("In getMasterInfo: cannot find master nodes in element faces");
    }
  }

  if (!masters_found) // check element interior embedded nodes
  {
    for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    {
      if (_interior_nodes[i]->getNode() == node)
      {
        std::vector<double> xi_3d(3, -100.0);
        for (unsigned int j = 0; j < 3; ++j)
          xi_3d[j] = _interior_nodes[i]->getParametricCoordinates(j);
        for (unsigned int j = 0; j < _num_nodes; ++j)
        {
          master_nodes.push_back(_nodes[j]);
          double weight = 0.0;
          if (_num_nodes == 8)
            weight = Efa::linearHexShape3D(j, xi_3d);
          else if (_num_nodes == 4)
            weight = Efa::linearTetShape3D(j, xi_3d);
          else
            EFAError("unknown 3D element");
          master_weights.push_back(weight);
        }
        masters_found = true;
        break;
      }
    }
  }

  if (!masters_found)
    EFAError("In EFAelement3D::getMaterInfo, cannot find the given EFAnode");
}

unsigned int
EFAElement3D::numInteriorNodes() const
{
  return _interior_nodes.size();
}

bool
EFAElement3D::overlaysElement(const EFAElement3D * other_elem) const
{
  bool overlays = false;
  const EFAElement3D * other3d = dynamic_cast<const EFAElement3D *>(other_elem);
  if (!other3d)
    EFAError("failed to dynamic cast to other3d");

  // Find indices of common nodes
  std::vector<unsigned int> common_face_curr = getCommonFaceID(other3d);
  if (common_face_curr.size() == 1)
  {
    unsigned int curr_face_id = common_face_curr[0];
    EFAFace * curr_face = _faces[curr_face_id];
    unsigned int other_face_id = other3d->getFaceID(curr_face);
    EFAFace * other_face = other3d->_faces[other_face_id];
    if (curr_face->hasSameOrientation(other_face))
      overlays = true;
  }
  else if (common_face_curr.size() > 1)
  {
    // TODO: We probably need more error checking here.
    overlays = true;
  }
  return overlays;
}

unsigned int
EFAElement3D::getNeighborIndex(const EFAElement * neighbor_elem) const
{
  for (unsigned int i = 0; i < _num_faces; ++i)
    for (unsigned int j = 0; j < _face_neighbors[i].size(); ++j)
      if (_face_neighbors[i][j] == neighbor_elem)
        return i;
  EFAError("in getNeighborIndex() element ", _id, " does not have neighbor ", neighbor_elem->id());
}

void
EFAElement3D::getNeighborEdgeIndex(const EFAElement3D * neighbor_elem,
                                   unsigned int face_id,
                                   unsigned int edge_id,
                                   unsigned int & neigh_face_id,
                                   unsigned int & neigh_edge_id) const
{
  EFAEdge * edge = this->getFace(face_id)->getEdge(edge_id);
  for (unsigned int i = 0; i < neighbor_elem->numFaces(); ++i)
  {
    for (unsigned int j = 0; j < neighbor_elem->getFace(i)->numEdges(); ++j)
    {
      EFAEdge * neigh_edge = neighbor_elem->getFace(i)->getEdge(j);
      if (neigh_edge->equivalent(*edge))
      {
        neigh_face_id = i;
        neigh_edge_id = j;
        return;
      }
    }
  }
  EFAError("in getNeighborEdgeIndex() element ",
           _id,
           " does not share a common edge with element",
           neighbor_elem->id());
}

void
EFAElement3D::clearNeighbors()
{
  _general_neighbors.clear();
  for (unsigned int face_iter = 0; face_iter < _num_faces; ++face_iter)
  {
    _face_neighbors[face_iter].clear();
    for (unsigned int edge_iter = 0; edge_iter < _faces[face_iter]->numEdges(); ++edge_iter)
      _face_edge_neighbors[face_iter][edge_iter].clear();
  }
}

void
EFAElement3D::setupNeighbors(std::map<EFANode *, std::set<EFAElement *>> & InverseConnectivityMap)
{
  findGeneralNeighbors(InverseConnectivityMap);
  for (unsigned int eit2 = 0; eit2 < _general_neighbors.size(); ++eit2)
  {
    EFAElement3D * neigh_elem = dynamic_cast<EFAElement3D *>(_general_neighbors[eit2]);
    if (!neigh_elem)
      EFAError("neighbor_elem is not of EFAelement3D type");

    std::vector<unsigned int> common_face_id = getCommonFaceID(neigh_elem);
    std::vector<unsigned int> face_ids, edge_ids;
    if (common_face_id.size() == 0 && getCommonEdgeID(neigh_elem, face_ids, edge_ids) &&
        !overlaysElement(neigh_elem))
    {
      bool is_edge_neighbor = false;

      // Fragments must match up.
      if ((_fragments.size() > 1) || (neigh_elem->numFragments() > 1))
      {
        EFAError("in updateFaceNeighbors: Cannot have more than 1 fragment");
      }
      else if ((_fragments.size() == 1) && (neigh_elem->numFragments() == 1))
      {
        if (_fragments[0]->isEdgeConnected(neigh_elem->getFragment(0)))
          is_edge_neighbor = true;
      }
      else // If there are no fragments to match up, consider them edge neighbors
        is_edge_neighbor = true;

      if (is_edge_neighbor)
      {
        for (unsigned int i = 0; i < face_ids.size(); ++i)
        {
          unsigned int face_id = face_ids[i];
          unsigned int edge_id = edge_ids[i];
          _face_edge_neighbors[face_id][edge_id].push_back(neigh_elem);
        }
      }
    }

    if (common_face_id.size() == 1 && !overlaysElement(neigh_elem))
    {
      unsigned int face_id = common_face_id[0];
      bool is_face_neighbor = false;

      // Fragments must match up.
      if ((_fragments.size() > 1) || (neigh_elem->numFragments() > 1))
      {
        EFAError("in updateFaceNeighbors: Cannot have more than 1 fragment");
      }
      else if ((_fragments.size() == 1) && (neigh_elem->numFragments() == 1))
      {
        if (_fragments[0]->isConnected(neigh_elem->getFragment(0)))
          is_face_neighbor = true;
      }
      else // If there are no fragments to match up, consider them neighbors
        is_face_neighbor = true;

      if (is_face_neighbor)
      {
        if (_face_neighbors[face_id].size() > 1)
        {
          EFAError("Element ",
                   _id,
                   " already has 2 face neighbors: ",
                   _face_neighbors[face_id][0]->id(),
                   " ",
                   _face_neighbors[face_id][1]->id());
        }
        _face_neighbors[face_id].push_back(neigh_elem);
      }
    }
  }
}

void
EFAElement3D::neighborSanityCheck() const
{
  for (unsigned int face_iter = 0; face_iter < _num_faces; ++face_iter)
  {
    for (unsigned int en_iter = 0; en_iter < _face_neighbors[face_iter].size(); ++en_iter)
    {
      EFAElement3D * neigh_elem = _face_neighbors[face_iter][en_iter];
      if (neigh_elem != NULL)
      {
        bool found_neighbor = false;
        for (unsigned int face_iter2 = 0; face_iter2 < neigh_elem->numFaces(); ++face_iter2)
        {
          for (unsigned int en_iter2 = 0; en_iter2 < neigh_elem->numFaceNeighbors(face_iter2);
               ++en_iter2)
          {
            if (neigh_elem->getFaceNeighbor(face_iter2, en_iter2) == this)
            {
              if ((en_iter2 > 1) && (en_iter > 1))
              {
                EFAError(
                    "Element and neighbor element cannot both have >1 neighbors on a common face");
              }
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
EFAElement3D::initCrackTip(std::set<EFAElement *> & CrackTipElements)
{
  if (isCrackTipElement())
  {
    CrackTipElements.insert(this);
    for (unsigned int face_iter = 0; face_iter < _num_faces; ++face_iter)
    {
      if ((_face_neighbors[face_iter].size() == 2) && (_faces[face_iter]->hasIntersection()))
      {
        // Neither neighbor overlays current element.  We are on the uncut element ahead of the tip.
        // Flag neighbors as crack tip split elements and add this element as their crack tip
        // neighbor.
        if (_face_neighbors[face_iter][0]->overlaysElement(this) ||
            _face_neighbors[face_iter][1]->overlaysElement(this))
          EFAError("Element has a neighbor that overlays itself");

        // Make sure the current elment hasn't been flagged as a tip element
        if (_crack_tip_split_element)
          EFAError("crack_tip_split_element already flagged.  In elem: ",
                   _id,
                   " flags: ",
                   _crack_tip_split_element,
                   " ",
                   _face_neighbors[face_iter][0]->isCrackTipSplit(),
                   " ",
                   _face_neighbors[face_iter][1]->isCrackTipSplit());

        _face_neighbors[face_iter][0]->setCrackTipSplit();
        _face_neighbors[face_iter][1]->setCrackTipSplit();

        _face_neighbors[face_iter][0]->addCrackTipNeighbor(this);
        _face_neighbors[face_iter][1]->addCrackTipNeighbor(this);
      }
    } // face_iter
  }
}

bool
EFAElement3D::shouldDuplicateForCrackTip(const std::set<EFAElement *> & CrackTipElements)
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
EFAElement3D::shouldDuplicateCrackTipSplitElement(const std::set<EFAElement *> & CrackTipElements)
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
        EFAElement3D * neigh_elem = dynamic_cast<EFAElement3D *>(_general_neighbors[eit]);
        if (!neigh_elem)
          EFAError("general elem is not of type EFAelement3D");

        // check if a general neighbor is an old crack tip element and will be split
        std::set<EFAElement *>::iterator sit;
        sit = CrackTipElements.find(neigh_elem);
        if (sit != CrackTipElements.end() && neigh_elem->numFragments() > 1)
        {
          for (unsigned int i = 0; i < neigh_elem->numFaces(); ++i)
          {
            std::set<EFANode *> neigh_face_nodes = neigh_elem->getFaceNodes(i);
            if (neigh_elem->numFaceNeighbors(i) == 2 &&
                Efa::numCommonElems(neigh_face_nodes, non_physical_nodes) > 0)
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
  } // IF only one fragment
  return should_duplicate;
}

bool
EFAElement3D::shouldDuplicateForPhantomCorner()
{
  // if a partial element will be split for a second time and it has two neighbor elements
  // sharing one phantom node with the aforementioned partial element, then the two neighbor
  // elements should be duplicated
  bool should_duplicate = false;
  if (_fragments.size() == 1 && (!_crack_tip_split_element))
  {
    for (unsigned int i = 0; i < _num_faces; ++i)
    {
      std::set<EFANode *> phantom_nodes = getPhantomNodeOnFace(i);
      if (phantom_nodes.size() > 0 && numFaceNeighbors(i) == 1)
      {
        EFAElement3D * neighbor_elem = _face_neighbors[i][0];
        if (neighbor_elem->numFragments() > 1) // neighbor will be split
        {
          for (unsigned int j = 0; j < neighbor_elem->numFaces(); ++j)
          {
            if (!neighbor_elem->getFace(j)->equivalent(_faces[i]) &&
                neighbor_elem->numFaceNeighbors(j) > 0)
            {
              std::set<EFANode *> neigh_phantom_nodes = neighbor_elem->getPhantomNodeOnFace(j);
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
EFAElement3D::willCrackTipExtend(std::vector<unsigned int> & split_neighbors) const
{
  // Determine whether the current element is a crack tip element for which the crack will
  // extend into the next element.
  // N.B. this is called at the beginning of createChildElements
  bool will_extend = false;
  if (_fragments.size() == 1 && _crack_tip_split_element)
  {
    for (unsigned int i = 0; i < _crack_tip_neighbors.size(); ++i)
    {
      unsigned int neigh_idx = _crack_tip_neighbors[i]; // essentially a face_id
      if (numFaceNeighbors(neigh_idx) != 1)
        EFAError("in will_crack_tip_extend() element ",
                 _id,
                 " has ",
                 _face_neighbors[neigh_idx].size(),
                 " neighbors on face ",
                 neigh_idx);

      EFAElement3D * neighbor_elem = _face_neighbors[neigh_idx][0];
      if (neighbor_elem->numFragments() > 2)
      {
        EFAError("in will_crack_tip_extend() element ",
                 neighbor_elem->id(),
                 " has ",
                 neighbor_elem->numFragments(),
                 " fragments");
      }
      else if (neighbor_elem->numFragments() == 2)
      {
        EFAFragment3D * neigh_frag1 = neighbor_elem->getFragment(0);
        EFAFragment3D * neigh_frag2 = neighbor_elem->getFragment(1);
        std::vector<EFANode *> neigh_cut_nodes = neigh_frag1->getCommonNodes(neigh_frag2);
        unsigned int counter = 0; // counter how many common nodes are contained by current face
        for (unsigned int j = 0; j < neigh_cut_nodes.size(); ++j)
        {
          if (_faces[neigh_idx]->containsNode(neigh_cut_nodes[j]))
            counter += 1;
        }
        if (counter == 2)
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
EFAElement3D::isCrackTipElement() const
{
  return fragmentHasTipFaces();
}

unsigned int
EFAElement3D::getNumCuts() const
{
  unsigned int num_cut_faces = 0;
  for (unsigned int i = 0; i < _num_faces; ++i)
    if (_faces[i]->hasIntersection())
      num_cut_faces += 1;
  return num_cut_faces;
}

bool
EFAElement3D::isFinalCut() const
{
  // if an element has been cut third times its fragment must have 3 interior faces
  // and at this point, we do not want it to be further cut
  bool cut_third = false;
  if (_fragments.size() > 0)
  {
    unsigned int num_interior_faces = 0;
    for (unsigned int i = 0; i < _fragments[0]->numFaces(); ++i)
    {
      if (_fragments[0]->isFaceInterior(i))
        num_interior_faces += 1;
    }
    if (num_interior_faces == 3)
      cut_third = true;
  }
  return cut_third;
}

void
EFAElement3D::updateFragments(const std::set<EFAElement *> & CrackTipElements,
                              std::map<unsigned int, EFANode *> & EmbeddedNodes)
{
  // combine the crack-tip faces in a fragment to a single intersected face
  std::set<EFAElement *>::iterator sit;
  sit = CrackTipElements.find(this);
  if (sit != CrackTipElements.end()) // curr_elem is a crack tip element
  {
    if (_fragments.size() == 1)
      _fragments[0]->combine_tip_faces();
    else
      EFAError("crack tip elem ", _id, " must have 1 fragment");
  }

  // remove the inappropriate embedded nodes on interior faces
  // (MUST DO THIS AFTER combine_tip_faces())
  if (_fragments.size() == 1)
    _fragments[0]->removeInvalidEmbeddedNodes(EmbeddedNodes);

  // for an element with no fragment, create one fragment identical to the element
  if (_fragments.size() == 0)
    _fragments.push_back(new EFAFragment3D(this, true, this));
  if (_fragments.size() != 1)
    EFAError("Element ", _id, " must have 1 fragment at this point");

  // count fragment's cut faces
  unsigned int num_cut_frag_faces = _fragments[0]->getNumCuts();
  unsigned int num_frag_faces = _fragments[0]->numFaces();
  if (num_cut_frag_faces > _fragments[0]->numFaces())
    EFAError("In element ", _id, " there are too many cut fragment faces");

  // leave the uncut frag as it is
  if (num_cut_frag_faces == 0)
  {
    if (!isPartial()) // delete the temp frag for an uncut elem
    {
      delete _fragments[0];
      _fragments.clear();
    }
    return;
  }

  // split one fragment into one or two new fragments
  std::vector<EFAFragment3D *> new_frags = _fragments[0]->split();
  if (new_frags.size() == 1 || new_frags.size() == 2)
  {
    delete _fragments[0]; // delete the old fragment
    _fragments.clear();
    for (unsigned int i = 0; i < new_frags.size(); ++i)
      _fragments.push_back(new_frags[i]);
  }
  else
    EFAError("Number of fragments must be 1 or 2 at this point");

  fragmentSanityCheck(num_frag_faces, num_cut_frag_faces);
}

void
EFAElement3D::fragmentSanityCheck(unsigned int n_old_frag_faces, unsigned int n_old_frag_cuts) const
{
  unsigned int n_interior_nodes = numInteriorNodes();
  if (n_interior_nodes > 0 && n_interior_nodes != 1)
    EFAError("After update_fragments this element has ", n_interior_nodes, " interior nodes");

  if (n_old_frag_cuts == 0)
  {
    if (_fragments.size() != 1 || _fragments[0]->numFaces() != n_old_frag_faces)
      EFAError("Incorrect link size for element with 0 cuts");
  }
  else if (fragmentHasTipFaces()) // crack tip case
  {
    if (_fragments.size() != 1 || _fragments[0]->numFaces() != n_old_frag_faces + n_old_frag_cuts)
      EFAError("Incorrect link size for element with crack-tip faces");
  }
  else // frag is thoroughly cut
  {
    if (_fragments.size() != 2 || (_fragments[0]->numFaces() + _fragments[1]->numFaces()) !=
                                      n_old_frag_faces + n_old_frag_cuts + 2)
      EFAError("Incorrect link size for element that has been completely cut");
  }
}

void
EFAElement3D::restoreFragment(const EFAElement * const from_elem)
{
  const EFAElement3D * from_elem3d = dynamic_cast<const EFAElement3D *>(from_elem);
  if (!from_elem3d)
    EFAError("from_elem is not of EFAelement3D type");

  // restore fragments
  if (_fragments.size() != 0)
    EFAError("in restoreFragmentInfo elements must not have any pre-existing fragments");
  for (unsigned int i = 0; i < from_elem3d->numFragments(); ++i)
    _fragments.push_back(new EFAFragment3D(this, true, from_elem3d, i));

  // restore interior nodes
  if (_interior_nodes.size() != 0)
    EFAError("in restoreFragmentInfo elements must not have any pre-exsiting interior nodes");
  for (unsigned int i = 0; i < from_elem3d->_interior_nodes.size(); ++i)
    _interior_nodes.push_back(new EFAVolumeNode(*from_elem3d->_interior_nodes[i]));

  // restore face intersections
  if (getNumCuts() != 0)
    EFAError("In restoreEdgeIntersection: edge cuts already exist in element ", _id);
  for (unsigned int i = 0; i < _num_faces; ++i)
    _faces[i]->copyIntersection(*from_elem3d->_faces[i]);

  // replace all local nodes with global nodes
  for (unsigned int i = 0; i < from_elem3d->numNodes(); ++i)
  {
    if (from_elem3d->_nodes[i]->category() == EFANode::N_CATEGORY_LOCAL_INDEX)
      switchNode(
          _nodes[i], from_elem3d->_nodes[i], false); // EFAelement is not a child of any parent
    else
      EFAError("In restoreFragmentInfo all of from_elem's nodes must be local");
  }
}

void
EFAElement3D::createChild(const std::set<EFAElement *> & CrackTipElements,
                          std::map<unsigned int, EFAElement *> & Elements,
                          std::map<unsigned int, EFAElement *> & newChildElements,
                          std::vector<EFAElement *> & ChildElements,
                          std::vector<EFAElement *> & ParentElements,
                          std::map<unsigned int, EFANode *> & TempNodes)
{
  if (_children.size() != 0)
    EFAError("Element cannot have existing children in createChildElements");

  if (_fragments.size() > 1 || shouldDuplicateForCrackTip(CrackTipElements))
  {
    if (_fragments.size() > 2)
      EFAError("More than 2 fragments not yet supported");

    // set up the children
    ParentElements.push_back(this);
    for (unsigned int ichild = 0; ichild < _fragments.size(); ++ichild)
    {
      unsigned int new_elem_id;
      if (newChildElements.size() == 0)
        new_elem_id = Efa::getNewID(Elements);
      else
        new_elem_id = Efa::getNewID(newChildElements);

      EFAElement3D * childElem = new EFAElement3D(new_elem_id, this->numNodes(), this->numFaces());
      newChildElements.insert(std::make_pair(new_elem_id, childElem));

      ChildElements.push_back(childElem);
      childElem->setParent(this);
      _children.push_back(childElem);

      std::vector<std::vector<EFANode *>> cut_plane_nodes;
      for (unsigned int i = 0; i < this->getFragment(ichild)->numFaces(); ++i)
      {
        if (this->getFragment(ichild)->isFaceInterior(i))
        {
          EFAFace * face = this->getFragment(ichild)->getFace(i);
          std::vector<EFANode *> node_line;
          for (unsigned int j = 0; j < face->numNodes(); ++j)
            node_line.push_back(face->getNode(j));
          cut_plane_nodes.push_back(node_line);
        }
      }

      std::vector<EFAPoint> cut_plane_points;

      EFAPoint normal(0.0, 0.0, 0.0);
      EFAPoint orig(0.0, 0.0, 0.0);

      if (cut_plane_nodes.size())
      {
        for (unsigned int i = 0; i < cut_plane_nodes[0].size(); ++i)
        {
          std::vector<EFANode *> master_nodes;
          std::vector<double> master_weights;

          this->getMasterInfo(cut_plane_nodes[0][i], master_nodes, master_weights);
          EFAPoint coor(0.0, 0.0, 0.0);
          for (unsigned int i = 0; i < master_nodes.size(); ++i)
          {
            EFANode * local = this->createLocalNodeFromGlobalNode(master_nodes[i]);
            coor += _local_node_coor[local->id()] * master_weights[i];
            delete local;
          }
          cut_plane_points.push_back(coor);
        }
        for (unsigned int i = 0; i < cut_plane_points.size(); ++i)
          orig += cut_plane_points[i];
        orig /= cut_plane_points.size();

        EFAPoint center(0.0, 0.0, 0.0);
        for (unsigned int i = 0; i < cut_plane_points.size(); ++i)
          center += cut_plane_points[i];
        center /= cut_plane_points.size();

        for (unsigned int i = 0; i < cut_plane_points.size(); ++i)
        {
          unsigned int iplus1 = i < cut_plane_points.size() - 1 ? i + 1 : 0;
          EFAPoint ray1 = cut_plane_points[i] - center;
          EFAPoint ray2 = cut_plane_points[iplus1] - center;
          normal += ray1.cross(ray2);
        }
        normal /= cut_plane_points.size();
        Xfem::normalizePoint(normal);
      }

      // get child element's nodes
      for (unsigned int j = 0; j < _num_nodes; ++j)
      {
        EFAPoint p(0.0, 0.0, 0.0);
        p = _local_node_coor[j];
        EFAPoint origin_to_point = p - orig;
        if (_fragments.size() == 1 && !shouldDuplicateForCrackTip(CrackTipElements))
          childElem->setNode(j, _nodes[j]); // inherit parent's node
        else if (origin_to_point * normal < Xfem::tol)
          childElem->setNode(j, _nodes[j]); // inherit parent's node
        else                                // parent element's node is not in fragment
        {
          unsigned int new_node_id = Efa::getNewID(TempNodes);
          EFANode * newNode = new EFANode(new_node_id, EFANode::N_CATEGORY_TEMP, _nodes[j]);
          TempNodes.insert(std::make_pair(new_node_id, newNode));
          childElem->setNode(j, newNode); // be a temp node
        }
      }

      // get child element's fragments
      EFAFragment3D * new_frag = new EFAFragment3D(childElem, true, this, ichild);
      childElem->_fragments.push_back(new_frag);

      // get child element's faces and set up adjacent faces
      childElem->createFaces();
      for (unsigned int j = 0; j < _num_faces; ++j)
        childElem->_faces[j]->copyIntersection(*_faces[j]);
      childElem->removePhantomEmbeddedNode(); // IMPORTANT

      // inherit old interior nodes
      for (unsigned int j = 0; j < _interior_nodes.size(); ++j)
        childElem->_interior_nodes.push_back(new EFAVolumeNode(*_interior_nodes[j]));
    }
  }
  else // num_links == 1 || num_links == 0
  {
    // child is itself - but don't insert into the list of ChildElements!!!
    _children.push_back(this);
  }
}

void
EFAElement3D::removePhantomEmbeddedNode()
{
  // remove the embedded nodes on faces that are outside the real domain
  if (_fragments.size() > 0)
  {
    for (unsigned int i = 0; i < _num_faces; ++i)
    {
      // get emb nodes to be removed on edges
      std::vector<EFANode *> nodes_to_delete;
      for (unsigned int j = 0; j < _faces[i]->numEdges(); ++j)
      {
        EFAEdge * edge = _faces[i]->getEdge(j);
        for (unsigned int k = 0; k < edge->numEmbeddedNodes(); ++k)
        {
          if (!_fragments[0]->containsNode(edge->getEmbeddedNode(k)))
            nodes_to_delete.push_back(edge->getEmbeddedNode(k));
        } // k
      }   // j

      // get emb nodes to be removed in the face interior
      for (unsigned int j = 0; j < _faces[i]->numInteriorNodes(); ++j)
      {
        EFANode * face_node = _faces[i]->getInteriorNode(j)->getNode();
        if (!_fragments[0]->containsNode(face_node))
          nodes_to_delete.push_back(face_node);
      } // j

      // remove all invalid embedded nodes
      for (unsigned int j = 0; j < nodes_to_delete.size(); ++j)
        _faces[i]->removeEmbeddedNode(nodes_to_delete[j]);
    } // i
  }
}

void
EFAElement3D::connectNeighbors(std::map<unsigned int, EFANode *> & PermanentNodes,
                               std::map<unsigned int, EFANode *> & TempNodes,
                               std::map<EFANode *, std::set<EFAElement *>> & InverseConnectivityMap,
                               bool merge_phantom_faces)
{
  // N.B. "this" must point to a child element that was just created
  if (!_parent)
    EFAError("no parent element for child element ", _id, " in connect_neighbors");
  EFAElement3D * parent3d = dynamic_cast<EFAElement3D *>(_parent);
  if (!parent3d)
    EFAError("cannot dynamic cast to parent3d in connect_neighbors");

  // First loop through edges and merge nodes with neighbors as appropriate
  for (unsigned int j = 0; j < _num_faces; ++j)
  {
    for (unsigned int k = 0; k < parent3d->numFaceNeighbors(j); ++k)
    {
      EFAElement3D * NeighborElem = parent3d->getFaceNeighbor(j, k);
      unsigned int neighbor_face_id = NeighborElem->getNeighborIndex(parent3d);

      if (_faces[j]->hasIntersection())
      {
        for (unsigned int l = 0; l < NeighborElem->numChildren(); ++l)
        {
          EFAElement3D * childOfNeighborElem =
              dynamic_cast<EFAElement3D *>(NeighborElem->getChild(l));
          if (!childOfNeighborElem)
            EFAError("dynamic cast childOfNeighborElem fails");

          // Check to see if the nodes are already merged.  There's nothing else to do in that case.
          EFAFace * neighborChildFace = childOfNeighborElem->getFace(neighbor_face_id);
          if (_faces[j]->equivalent(neighborChildFace))
            continue;

          if (_fragments[0]->isConnected(childOfNeighborElem->getFragment(0)))
          {
            for (unsigned int i = 0; i < _faces[j]->numNodes(); ++i)
            {
              unsigned int childNodeIndex = i;
              unsigned int neighborChildNodeIndex =
                  parent3d->getNeighborFaceNodeID(j, childNodeIndex, NeighborElem);

              EFANode * childNode = _faces[j]->getNode(childNodeIndex);
              EFANode * childOfNeighborNode = neighborChildFace->getNode(neighborChildNodeIndex);
              mergeNodes(
                  childNode, childOfNeighborNode, childOfNeighborElem, PermanentNodes, TempNodes);
            } // i

            for (unsigned int m = 0; m < _num_interior_face_nodes; ++m)
            {
              unsigned int childNodeIndex = m;
              unsigned int neighborChildNodeIndex =
                  parent3d->getNeighborFaceInteriorNodeID(j, childNodeIndex, NeighborElem);

              EFANode * childNode = _faces[j]->getInteriorFaceNode(childNodeIndex);
              EFANode * childOfNeighborNode =
                  neighborChildFace->getInteriorFaceNode(neighborChildNodeIndex);
              mergeNodes(
                  childNode, childOfNeighborNode, childOfNeighborElem, PermanentNodes, TempNodes);
            } // m
          }
        } // l, loop over NeighborElem's children
      }
      else // No edge intersection -- optionally merge non-material nodes if they share a common
           // parent
      {
        if (merge_phantom_faces)
        {
          for (unsigned int l = 0; l < NeighborElem->numChildren(); ++l)
          {
            EFAElement3D * childOfNeighborElem =
                dynamic_cast<EFAElement3D *>(NeighborElem->getChild(l));
            if (!childOfNeighborElem)
              EFAError("dynamic cast childOfNeighborElem fails");

            EFAFace * neighborChildFace = childOfNeighborElem->getFace(neighbor_face_id);
            if (!neighborChildFace
                     ->hasIntersection()) // neighbor face must NOT have intersection either
            {
              // Check to see if the nodes are already merged.  There's nothing else to do in that
              // case.
              if (_faces[j]->equivalent(neighborChildFace))
                continue;

              for (unsigned int i = 0; i < _faces[j]->numNodes(); ++i)
              {
                unsigned int childNodeIndex = i;
                unsigned int neighborChildNodeIndex =
                    parent3d->getNeighborFaceNodeID(j, childNodeIndex, NeighborElem);

                EFANode * childNode = _faces[j]->getNode(childNodeIndex);
                EFANode * childOfNeighborNode = neighborChildFace->getNode(neighborChildNodeIndex);

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
  }         // j, loop over all faces

  // Now do a second loop through faces and convert remaining nodes to permanent nodes.
  // If there is no neighbor on that face, also duplicate the embedded node if it exists
  for (unsigned int j = 0; j < _num_nodes; ++j)
  {
    EFANode * childNode = _nodes[j];
    if (childNode->category() == EFANode::N_CATEGORY_TEMP)
    {
      // if current child element does not have siblings, and if current temp node is a lone one
      // this temp node should be merged back to its parent permanent node. Otherwise we would have
      // permanent nodes that are not connected to any element
      std::set<EFAElement *> patch_elems = InverseConnectivityMap[childNode->parent()];
      if (parent3d->numFragments() == 1 && patch_elems.size() == 1)
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
EFAElement3D::printElement(std::ostream & ostream) const
{
  // first line: all elem faces
  ostream << std::setw(5);
  ostream << _id << "| ";
  for (unsigned int j = 0; j < _num_faces; ++j)
  {
    for (unsigned int k = 0; k < _faces[j]->numNodes(); ++k)
      ostream << std::setw(5) << _faces[j]->getNode(k)->idCatString();
    ostream << " | ";
  }
  ostream << std::endl;

  // second line: emb nodes in all faces + neighbor of each face
  ostream << std::setw(5);
  ostream << "embed"
          << "| ";
  for (unsigned int j = 0; j < _num_faces; ++j)
  {
    for (unsigned int k = 0; k < _faces[j]->numEdges(); ++k)
    {
      ostream << std::setw(4);
      if (_faces[j]->getEdge(k)->hasIntersection())
      {
        if (_faces[j]->getEdge(k)->numEmbeddedNodes() > 1)
        {
          ostream << "[";
          for (unsigned int l = 0; l < _faces[j]->getEdge(k)->numEmbeddedNodes(); ++l)
          {
            ostream << _faces[j]->getEdge(k)->getEmbeddedNode(l)->id() << " ";
            if (l == _faces[j]->getEdge(k)->numEmbeddedNodes() - 1)
              ostream << "]";
            else
              ostream << " ";
          } // l
        }
        else
          ostream << _faces[j]->getEdge(k)->getEmbeddedNode(0)->id() << " ";
      }
      else
        ostream << "  -- ";
    } // k
    ostream << " | ";
  } // j
  ostream << std::endl;

  // third line: neighbors
  ostream << std::setw(5);
  ostream << "neigh"
          << "| ";
  for (unsigned int j = 0; j < _num_faces; ++j)
  {
    ostream << std::setw(4);
    if (numFaceNeighbors(j) > 1)
    {
      ostream << "[";
      for (unsigned int k = 0; k < numFaceNeighbors(j); ++k)
      {
        ostream << getFaceNeighbor(j, k)->id();
        if (k == numFaceNeighbors(j) - 1)
          ostream << "]";
        else
          ostream << " ";
      }
    }
    else
    {
      if (numFaceNeighbors(j) == 1)
        ostream << getFaceNeighbor(j, 0)->id() << " ";
      else
        ostream << "  -- ";
    }
  }
  ostream << std::endl;

  // fourth line: fragments
  for (unsigned int j = 0; j < _fragments.size(); ++j)
  {
    ostream << std::setw(4);
    ostream << "frag" << j << "| ";
    for (unsigned int k = 0; k < _fragments[j]->numFaces(); ++k)
    {
      for (unsigned int l = 0; l < _fragments[j]->getFace(k)->numNodes(); ++l)
        ostream << std::setw(5) << _fragments[j]->getFace(k)->getNode(l)->idCatString();
      ostream << " | ";
    }
    ostream << std::endl;
  }
  ostream << std::endl;
}

EFAFragment3D *
EFAElement3D::getFragment(unsigned int frag_id) const
{
  if (frag_id < _fragments.size())
    return _fragments[frag_id];
  else
    EFAError("frag_id out of bounds");
}

std::set<EFANode *>
EFAElement3D::getFaceNodes(unsigned int face_id) const
{
  std::set<EFANode *> face_nodes;
  for (unsigned int i = 0; i < _faces[face_id]->numNodes(); ++i)
    face_nodes.insert(_faces[face_id]->getNode(i));
  return face_nodes;
}

bool
EFAElement3D::getFaceNodeParametricCoordinates(EFANode * node, std::vector<double> & xi_3d) const
{
  // get the parametric coords of a node in an element face
  unsigned int face_id = std::numeric_limits<unsigned int>::max();
  bool face_found = false;
  for (unsigned int i = 0; i < _num_faces; ++i)
  {
    if (_faces[i]->containsNode(node))
    {
      face_id = i;
      face_found = true;
      break;
    }
  }
  if (face_found)
  {
    std::vector<double> xi_2d(2, 0.0);
    if (_faces[face_id]->getFaceNodeParametricCoords(node, xi_2d))
      mapParametricCoordinateFrom2DTo3D(face_id, xi_2d, xi_3d);
    else
      EFAError("failed to get the 2D para coords on the face");
  }
  return face_found;
}

EFAVolumeNode *
EFAElement3D::getInteriorNode(unsigned int interior_node_id) const
{
  if (interior_node_id < _interior_nodes.size())
    return _interior_nodes[interior_node_id];
  else
    EFAError("interior_node_id out of bounds");
}

void
EFAElement3D::removeEmbeddedNode(EFANode * emb_node, bool remove_for_neighbor)
{
  for (unsigned int i = 0; i < _fragments.size(); ++i)
    _fragments[i]->removeEmbeddedNode(emb_node);

  for (unsigned int i = 0; i < _faces.size(); ++i)
    _faces[i]->removeEmbeddedNode(emb_node);

  if (remove_for_neighbor)
  {
    for (unsigned int i = 0; i < numFaces(); ++i)
      for (unsigned int j = 0; j < numFaceNeighbors(i); ++j)
        getFaceNeighbor(i, j)->removeEmbeddedNode(emb_node, false);
  }
}

unsigned int
EFAElement3D::numFaces() const
{
  return _faces.size();
}

void
EFAElement3D::setFace(unsigned int face_id, EFAFace * face)
{
  _faces[face_id] = face;
}

void
EFAElement3D::createFaces()
{
  // create element faces based on existing element nodes
  int hex_local_node_indices[6][4] = {
      {0, 3, 2, 1}, {0, 1, 5, 4}, {1, 2, 6, 5}, {2, 3, 7, 6}, {3, 0, 4, 7}, {4, 5, 6, 7}};
  int tet_local_node_indices[4][3] = {{0, 2, 1}, {0, 1, 3}, {1, 2, 3}, {2, 0, 3}};

  int hex_interior_face_node_indices[6][5] = {{8, 9, 10, 11, 20},
                                              {8, 13, 16, 12, 21},
                                              {9, 14, 17, 13, 22},
                                              {10, 14, 18, 15, 23},
                                              {11, 15, 19, 12, 24},
                                              {16, 17, 18, 19, 25}};
  int tet_interior_face_node_indices[4][3] = {{4, 5, 6}, {4, 7, 8}, {5, 8, 9}, {6, 7, 9}};

  _faces = std::vector<EFAFace *>(_num_faces, NULL);
  if (_num_nodes == 8 || _num_nodes == 20 || _num_nodes == 27)
  {
    if (_num_faces != 6)
      EFAError("num_faces of hexes must be 6");
    for (unsigned int i = 0; i < _num_faces; ++i)
    {
      _faces[i] = new EFAFace(4, _num_interior_face_nodes);
      for (unsigned int j = 0; j < 4; ++j)
        _faces[i]->setNode(j, _nodes[hex_local_node_indices[i][j]]);
      _faces[i]->createEdges();
      for (unsigned int k = 0; k < _num_interior_face_nodes; ++k)
        _faces[i]->setInteriorFaceNode(k, _nodes[hex_interior_face_node_indices[i][k]]);
    }
  }
  else if (_num_nodes == 4 || _num_nodes == 10)
  {
    if (_num_faces != 4)
      EFAError("num_faces of tets must be 4");
    for (unsigned int i = 0; i < _num_faces; ++i)
    {
      _faces[i] = new EFAFace(3, _num_interior_face_nodes);
      for (unsigned int j = 0; j < 3; ++j)
        _faces[i]->setNode(j, _nodes[tet_local_node_indices[i][j]]);
      _faces[i]->createEdges();
      for (unsigned int k = 0; k < _num_interior_face_nodes; ++k)
        _faces[i]->setInteriorFaceNode(k, _nodes[tet_interior_face_node_indices[i][k]]);
    }
  }
  else
    EFAError("unknown 3D element type in createFaces()");

  for (unsigned int face_iter = 0; face_iter < _num_faces; ++face_iter)
  {
    _face_edge_neighbors[face_iter].resize(_faces[face_iter]->numEdges());
    for (unsigned int edge_iter = 0; edge_iter < _faces[face_iter]->numEdges(); ++edge_iter)
      _face_edge_neighbors[face_iter][edge_iter].clear();
  }

  // create element face connectivity array
  findFacesAdjacentToFaces(); // IMPORTANT
}

EFAFace *
EFAElement3D::getFace(unsigned int face_id) const
{
  return _faces[face_id];
}

unsigned int
EFAElement3D::getFaceID(EFAFace * face) const
{
  bool found_face_id = false;
  unsigned int face_id;
  for (unsigned int iface = 0; iface < _num_faces; ++iface)
  {
    if (_faces[iface]->equivalent(face))
    {
      face_id = iface;
      found_face_id = true;
      break;
    }
  }
  if (!found_face_id)
    EFAError("input face not found in get_face_id()");
  return face_id;
}

std::vector<unsigned int>
EFAElement3D::getCommonFaceID(const EFAElement3D * other_elem) const
{
  std::vector<unsigned int> face_id;
  for (unsigned int i = 0; i < _num_faces; ++i)
  {
    for (unsigned int j = 0; j < other_elem->_num_faces; ++j)
    {
      if (_faces[i]->equivalent(other_elem->_faces[j]))
      {
        face_id.push_back(i);
        break;
      }
    }
  }
  return face_id;
}

bool
EFAElement3D::getCommonEdgeID(const EFAElement3D * other_elem,
                              std::vector<unsigned int> & face_id,
                              std::vector<unsigned int> & edge_id) const
{
  bool has_common_edge = false;
  bool move_to_next_edge = false;
  face_id.clear();
  edge_id.clear();
  for (unsigned int i = 0; i < _num_faces; ++i)
    for (unsigned int j = 0; j < _faces[i]->numEdges(); ++j)
    {
      move_to_next_edge = false;
      for (unsigned int k = 0; k < other_elem->_num_faces; ++k)
      {
        for (unsigned int l = 0; l < other_elem->_faces[k]->numEdges(); ++l)
          if ((_faces[i]->getEdge(j)->equivalent(*(other_elem->_faces[k]->getEdge(l)))) &&
              !(_faces[i]->equivalent(other_elem->_faces[k])))
          {
            face_id.push_back(i);
            edge_id.push_back(j);
            move_to_next_edge = true;
            has_common_edge = true;
            break;
          }

        if (move_to_next_edge)
          break;
      }
    }

  return has_common_edge;
}

unsigned int
EFAElement3D::getNeighborFaceNodeID(unsigned int face_id,
                                    unsigned int node_id,
                                    EFAElement3D * neighbor_elem) const
{
  // get the corresponding node_id on the corresponding face of neighbor_elem
  bool found_id = false;
  unsigned int neigh_face_node_id;
  unsigned int common_face_id = getNeighborIndex(neighbor_elem);
  if (common_face_id == face_id)
  {
    unsigned int neigh_face_id = neighbor_elem->getNeighborIndex(this);
    EFAFace * neigh_face = neighbor_elem->getFace(neigh_face_id);
    for (unsigned int i = 0; i < neigh_face->numNodes(); ++i)
    {
      if (_faces[face_id]->getNode(node_id) == neigh_face->getNode(i))
      {
        neigh_face_node_id = i;
        found_id = true;
        break;
      }
    }
  }
  else
    EFAError("getNeighborFaceNodeID: neighbor_elem is not a neighbor on face_id");
  if (!found_id)
    EFAError("getNeighborFaceNodeID: could not find neighbor face node id");
  return neigh_face_node_id;
}

unsigned int
EFAElement3D::getNeighborFaceInteriorNodeID(unsigned int face_id,
                                            unsigned int node_id,
                                            EFAElement3D * neighbor_elem) const
{
  // get the corresponding node_id on the corresponding face of neighbor_elem
  bool found_id = false;
  unsigned int neigh_face_node_id;
  unsigned int common_face_id = getNeighborIndex(neighbor_elem);
  if (common_face_id == face_id)
  {
    unsigned int neigh_face_id = neighbor_elem->getNeighborIndex(this);
    EFAFace * neigh_face = neighbor_elem->getFace(neigh_face_id);

    for (unsigned int i = 0; i < _num_interior_face_nodes; ++i)
    {
      if (_faces[face_id]->getInteriorFaceNode(node_id) == neigh_face->getInteriorFaceNode(i))
      {
        neigh_face_node_id = i;
        found_id = true;
        break;
      }
    }
  }
  else
    EFAError("getNeighborFaceNodeID: neighbor_elem is not a neighbor on face_id");
  if (!found_id)
    EFAError("getNeighborFaceNodeID: could not find neighbor face node id");
  return neigh_face_node_id;
}

unsigned int
EFAElement3D::getNeighborFaceEdgeID(unsigned int face_id,
                                    unsigned int edge_id,
                                    EFAElement3D * neighbor_elem) const
{
  // get the corresponding edge_id on the corresponding face of neighbor_elem
  bool found_id = false;
  unsigned int neigh_face_edge_id;
  unsigned int common_face_id = getNeighborIndex(neighbor_elem);
  if (common_face_id == face_id)
  {
    unsigned int neigh_face_id = neighbor_elem->getNeighborIndex(this);
    EFAFace * neigh_face = neighbor_elem->getFace(neigh_face_id);
    for (unsigned int i = 0; i < neigh_face->numEdges(); ++i)
    {
      if (_faces[face_id]->getEdge(edge_id)->equivalent(*neigh_face->getEdge(i)))
      {
        neigh_face_edge_id = i;
        found_id = true;
        break;
      }
    }
  }
  else
    EFAError("getNeighborFaceEdgeID: neighbor_elem is not a neighbor on face_id");
  if (!found_id)
    EFAError("getNeighborFaceEdgeID: could not find neighbor face edge id");
  return neigh_face_edge_id;
}

void
EFAElement3D::findFacesAdjacentToFaces()
{
  _faces_adjacent_to_faces.clear();
  for (unsigned int i = 0; i < _faces.size(); ++i)
  {
    std::vector<EFAFace *> face_adjacents(_faces[i]->numEdges(), NULL);
    for (unsigned int j = 0; j < _faces.size(); ++j)
    {
      if (_faces[j] != _faces[i] && _faces[i]->isAdjacent(_faces[j]))
      {
        unsigned int adj_edge = _faces[i]->adjacentCommonEdge(_faces[j]);
        face_adjacents[adj_edge] = _faces[j];
      }
    }
    _faces_adjacent_to_faces.push_back(face_adjacents);
  }
}

EFAFace *
EFAElement3D::getAdjacentFace(unsigned int face_id, unsigned int edge_id) const
{
  return _faces_adjacent_to_faces[face_id][edge_id];
}

EFAFace *
EFAElement3D::getFragmentFace(unsigned int frag_id, unsigned int face_id) const
{
  if (frag_id < _fragments.size())
    return _fragments[frag_id]->getFace(face_id);
  else
    EFAError("frag_id out of bounds in getFragmentFace");
}

std::set<EFANode *>
EFAElement3D::getPhantomNodeOnFace(unsigned int face_id) const
{
  std::set<EFANode *> phantom_nodes;
  if (_fragments.size() > 0)
  {
    for (unsigned int j = 0; j < _faces[face_id]->numNodes(); ++j) // loop ove 2 edge nodes
    {
      bool node_in_frag = false;
      for (unsigned int k = 0; k < _fragments.size(); ++k)
      {
        if (_fragments[k]->containsNode(_faces[face_id]->getNode(j)))
        {
          node_in_frag = true;
          break;
        }
      }
      if (!node_in_frag)
        phantom_nodes.insert(_faces[face_id]->getNode(j));
    }
  }
  return phantom_nodes;
}

bool
EFAElement3D::getFragmentFaceID(unsigned int elem_face_id, unsigned int & frag_face_id) const
{
  // find the fragment face that is contained by given element edge
  // N.B. if the elem edge contains two frag edges, this method will only return
  // the first frag edge ID
  bool frag_face_found = false;
  frag_face_id = std::numeric_limits<unsigned int>::max();
  if (_fragments.size() == 1)
  {
    for (unsigned int j = 0; j < _fragments[0]->numFaces(); ++j)
    {
      if (_faces[elem_face_id]->containsFace(_fragments[0]->getFace(j)))
      {
        frag_face_id = j;
        frag_face_found = true;
        break;
      }
    }
  }
  return frag_face_found;
}

bool
EFAElement3D::getFragmentFaceEdgeID(unsigned int ElemFaceID,
                                    unsigned int ElemFaceEdgeID,
                                    unsigned int & FragFaceID,
                                    unsigned int & FragFaceEdgeID) const
{
  // Purpose: given an edge of an elem face, find which frag face's edge it contains
  bool frag_edge_found = false;
  FragFaceID = FragFaceEdgeID = std::numeric_limits<unsigned int>::max();
  if (getFragmentFaceID(ElemFaceID, FragFaceID))
  {
    EFAEdge * elem_edge = _faces[ElemFaceID]->getEdge(ElemFaceEdgeID);
    EFAFace * frag_face = getFragmentFace(0, FragFaceID);
    for (unsigned int i = 0; i < frag_face->numEdges(); ++i)
    {
      if (elem_edge->containsEdge(*frag_face->getEdge(i)))
      {
        FragFaceEdgeID = i;
        frag_edge_found = true;
        break;
      }
    }
  }
  return frag_edge_found;
}

bool
EFAElement3D::isPhysicalEdgeCut(unsigned int ElemFaceID,
                                unsigned int ElemFaceEdgeID,
                                double position) const
{
  unsigned int FragFaceID, FragFaceEdgeID = std::numeric_limits<unsigned int>::max();
  bool is_in_real = false;
  if (_fragments.size() == 0)
  {
    is_in_real = true;
  }
  else if (getFragmentFaceEdgeID(ElemFaceID, ElemFaceEdgeID, FragFaceID, FragFaceEdgeID))
  {
    EFAEdge * elem_edge = _faces[ElemFaceID]->getEdge(ElemFaceEdgeID);
    EFAEdge * frag_edge = getFragmentFace(0, FragFaceID)->getEdge(FragFaceEdgeID);
    double xi[2] = {-1.0, -1.0}; // relative coords of two frag edge nodes
    xi[0] = elem_edge->distanceFromNode1(frag_edge->getNode(0));
    xi[1] = elem_edge->distanceFromNode1(frag_edge->getNode(1));
    if ((position - xi[0]) * (position - xi[1]) <
        0.0) // the cut to be added is within the real part of the edge
      is_in_real = true;
  }
  return is_in_real;
}

bool
EFAElement3D::isFacePhantom(unsigned int face_id) const
{
  bool is_phantom = false;
  if (_fragments.size() > 0)
  {
    bool contains_frag_face = false;
    for (unsigned int i = 0; i < _fragments.size(); ++i)
    {
      for (unsigned int j = 0; j < _fragments[i]->numFaces(); ++j)
      {
        if (_faces[face_id]->containsFace(_fragments[i]->getFace(j)))
        {
          contains_frag_face = true;
          break;
        }
      }
      if (contains_frag_face)
        break;
    }
    if (!contains_frag_face)
      is_phantom = true;
  }
  return is_phantom;
}

unsigned int
EFAElement3D::numFaceNeighbors(unsigned int face_id) const
{
  return _face_neighbors[face_id].size();
}

unsigned int
EFAElement3D::numEdgeNeighbors(unsigned int face_id, unsigned int edge_id) const
{
  return _face_edge_neighbors[face_id][edge_id].size();
}

EFAElement3D *
EFAElement3D::getFaceNeighbor(unsigned int face_id, unsigned int neighbor_id) const
{
  if (_face_neighbors[face_id][0] != NULL && neighbor_id < _face_neighbors[face_id].size())
    return _face_neighbors[face_id][neighbor_id];
  else
    EFAError("edge neighbor does not exist");
}

EFAElement3D *
EFAElement3D::getEdgeNeighbor(unsigned int face_id,
                              unsigned int edge_id,
                              unsigned int neighbor_id) const
{
  if (neighbor_id < _face_edge_neighbors[face_id][edge_id].size())
    return _face_edge_neighbors[face_id][edge_id][neighbor_id];
  else
    EFAError("edge neighbor does not exist");
}

bool
EFAElement3D::fragmentHasTipFaces() const
{
  bool has_tip_faces = false;
  if (_fragments.size() == 1)
  {
    for (unsigned int i = 0; i < _num_faces; ++i)
    {
      unsigned int num_frag_faces = 0; // count how many fragment edges this element edge contains
      if (_faces[i]->hasIntersection())
      {
        for (unsigned int j = 0; j < _fragments[0]->numFaces(); ++j)
        {
          if (_faces[i]->containsFace(_fragments[0]->getFace(j)))
            num_frag_faces += 1;
        } // j
        if (num_frag_faces == 2)
        {
          has_tip_faces = true;
          break;
        }
      }
    } // i
  }
  return has_tip_faces;
}

std::vector<unsigned int>
EFAElement3D::getTipFaceIDs() const
{
  // if this element is a crack tip element, returns the crack tip faces' ID
  std::vector<unsigned int> tip_face_id;
  if (_fragments.size() == 1) // crack tip element with a partial fragment saved
  {
    for (unsigned int i = 0; i < _num_faces; ++i)
    {
      unsigned int num_frag_faces = 0; // count how many fragment faces this element edge contains
      if (_faces[i]->hasIntersection())
      {
        for (unsigned int j = 0; j < _fragments[0]->numFaces(); ++j)
        {
          if (_faces[i]->containsFace(_fragments[0]->getFace(j)))
            num_frag_faces += 1;
        }                        // j
        if (num_frag_faces == 2) // element face contains two fragment edges
          tip_face_id.push_back(i);
      }
    } // i
  }
  return tip_face_id;
}

std::set<EFANode *>
EFAElement3D::getTipEmbeddedNodes() const
{
  // if this element is a crack tip element, returns the crack tip edge's ID
  std::set<EFANode *> tip_emb;
  if (_fragments.size() == 1) // crack tip element with a partial fragment saved
  {
    for (unsigned int i = 0; i < _num_faces; ++i)
    {
      std::vector<EFAFace *> frag_faces; // count how many fragment edges this element edge contains
      if (_faces[i]->hasIntersection())
      {
        for (unsigned int j = 0; j < _fragments[0]->numFaces(); ++j)
        {
          if (_faces[i]->containsFace(_fragments[0]->getFace(j)))
            frag_faces.push_back(_fragments[0]->getFace(j));
        }                           // j
        if (frag_faces.size() == 2) // element edge contains two fragment edges
        {
          unsigned int edge_id = frag_faces[0]->adjacentCommonEdge(frag_faces[1]);
          tip_emb.insert(frag_faces[0]->getEdge(edge_id)->getNode(0));
          tip_emb.insert(frag_faces[0]->getEdge(edge_id)->getNode(1));
        }
      }
    } // i
  }
  return tip_emb;
}

bool
EFAElement3D::faceContainsTip(unsigned int face_id) const
{
  bool contain_tip = false;
  if (_fragments.size() == 1)
  {
    unsigned int num_frag_faces = 0; // count how many fragment faces this element face contains
    if (_faces[face_id]->hasIntersection())
    {
      for (unsigned int j = 0; j < _fragments[0]->numFaces(); ++j)
      {
        if (_faces[face_id]->containsFace(_fragments[0]->getFace(j)))
          num_frag_faces += 1;
      } // j
      if (num_frag_faces == 2)
        contain_tip = true;
    }
  }
  return contain_tip;
}

bool
EFAElement3D::fragmentFaceAlreadyCut(unsigned int ElemFaceID) const
{
  // when marking cuts, check if the corresponding frag face already has been cut
  bool has_cut = false;
  if (faceContainsTip(ElemFaceID))
    has_cut = true;
  else
  {
    unsigned int FragFaceID = std::numeric_limits<unsigned int>::max();
    if (getFragmentFaceID(ElemFaceID, FragFaceID))
    {
      EFAFace * frag_face = getFragmentFace(0, FragFaceID);
      if (frag_face->hasIntersection())
        has_cut = true;
    }
  }
  return has_cut;
}

void
EFAElement3D::addFaceEdgeCut(unsigned int face_id,
                             unsigned int edge_id,
                             double position,
                             EFANode * embedded_node,
                             std::map<unsigned int, EFANode *> & EmbeddedNodes,
                             bool add_to_neighbor,
                             bool add_to_adjacent)
{
  // Purpose: add intersection on Edge edge_id of Face face_id
  EFANode * local_embedded = NULL;
  EFAEdge * cut_edge = _faces[face_id]->getEdge(edge_id);
  EFANode * edge_node1 = cut_edge->getNode(0);
  if (embedded_node) // use the existing embedded node if it was passed in
    local_embedded = embedded_node;

  // get adjacent face info
  EFAFace * adj_face = getAdjacentFace(face_id, edge_id);
  unsigned int adj_face_id = getFaceID(adj_face);
  unsigned int adj_edge_id = adj_face->adjacentCommonEdge(_faces[face_id]);

  // check if cut has already been added to this face edge
  bool cut_exist = false;

  if (cut_edge->hasIntersectionAtPosition(position, edge_node1))
  {
    unsigned int emb_id = cut_edge->getEmbeddedNodeIndex(position, edge_node1);
    EFANode * old_emb = cut_edge->getEmbeddedNode(emb_id);
    if (embedded_node && embedded_node != old_emb)
      EFAError("Attempting to add edge intersection when one already exists with different node.",
               " elem: ",
               _id,
               " edge: ",
               edge_id,
               " position: ",
               position);
    local_embedded = old_emb;
    cut_exist = true;
  }

  if (!cut_exist && !fragmentFaceAlreadyCut(face_id) &&
      isPhysicalEdgeCut(face_id, edge_id, position))
  {
    // check if cut has already been added to the neighbor edges
    checkNeighborFaceCut(face_id, edge_id, position, edge_node1, embedded_node, local_embedded);
    checkNeighborFaceCut(
        adj_face_id, adj_edge_id, position, edge_node1, embedded_node, local_embedded);

    if (!local_embedded) // need to create new embedded node
    {
      unsigned int new_node_id = Efa::getNewID(EmbeddedNodes);
      local_embedded = new EFANode(new_node_id, EFANode::N_CATEGORY_EMBEDDED);
      EmbeddedNodes.insert(std::make_pair(new_node_id, local_embedded));
    }

    // add to elem face edge
    cut_edge->addIntersection(position, local_embedded, edge_node1);
    if (cut_edge->numEmbeddedNodes() > 2)
      EFAError("element edge can't have >2 embedded nodes");

    // cut fragment faces, which is an essential addition to other code in this method that cuts
    // element faces
    unsigned int FragFaceID;
    unsigned int FragFaceEdgeID;
    if (getFragmentFaceEdgeID(face_id, edge_id, FragFaceID, FragFaceEdgeID))
    {
      EFAEdge * elem_edge = _faces[face_id]->getEdge(edge_id);
      EFAEdge * frag_edge = getFragmentFace(0, FragFaceID)->getEdge(FragFaceEdgeID);
      double xi[2] = {-1.0, -1.0}; // relative coords of two frag edge nodes
      xi[0] = elem_edge->distanceFromNode1(frag_edge->getNode(0));
      xi[1] = elem_edge->distanceFromNode1(frag_edge->getNode(1));
      double frag_pos = (position - xi[0]) / (xi[1] - xi[0]);
      EFANode * frag_edge_node1 = frag_edge->getNode(0);

      if (!frag_edge->hasIntersection())
        frag_edge->addIntersection(frag_pos, local_embedded, frag_edge_node1);
    }

    // add to adjacent face edge
    if (add_to_adjacent)
    {
      double adj_pos = 1.0 - position;
      addFaceEdgeCut(
          adj_face_id, adj_edge_id, adj_pos, local_embedded, EmbeddedNodes, false, false);
    }
  }

  // add cut to neighbor face edge
  if (add_to_neighbor)
  {
    for (unsigned int en_iter = 0; en_iter < numFaceNeighbors(face_id); ++en_iter)
    {
      EFAElement3D * face_neighbor = getFaceNeighbor(face_id, en_iter);
      unsigned int neigh_face_id = face_neighbor->getNeighborIndex(this);
      unsigned neigh_edge_id = getNeighborFaceEdgeID(face_id, edge_id, face_neighbor);
      double neigh_pos = 1.0 - position; // get emb node's postion on neighbor edge
      face_neighbor->addFaceEdgeCut(
          neigh_face_id, neigh_edge_id, neigh_pos, local_embedded, EmbeddedNodes, false, true);
    }

    for (unsigned int en_iter = 0; en_iter < numEdgeNeighbors(face_id, edge_id); ++en_iter)
    {
      EFAElement3D * edge_neighbor = getEdgeNeighbor(face_id, edge_id, en_iter);
      unsigned int neigh_face_id, neigh_edge_id;
      getNeighborEdgeIndex(edge_neighbor, face_id, edge_id, neigh_face_id, neigh_edge_id);

      // Check the ordering of the node and the assign the position
      double neigh_pos;
      if (_faces[face_id]->getEdge(edge_id)->getNode(0) ==
          edge_neighbor->getFace(neigh_face_id)->getEdge(neigh_edge_id)->getNode(0))
        neigh_pos = position;
      else if (_faces[face_id]->getEdge(edge_id)->getNode(1) ==
               edge_neighbor->getFace(neigh_face_id)->getEdge(neigh_edge_id)->getNode(0))
        neigh_pos = 1.0 - position;
      else
        EFAError("The EFANodes on commaon edge are not matched.");

      edge_neighbor->addFaceEdgeCut(
          neigh_face_id, neigh_edge_id, neigh_pos, local_embedded, EmbeddedNodes, false, true);
    }
  } // If add_to_neighbor required
}

void
EFAElement3D::addFragFaceEdgeCut(unsigned int /*frag_face_id*/,
                                 unsigned int /*frag_edge_id*/,
                                 double /*position*/,
                                 std::map<unsigned int, EFANode *> & /*EmbeddedNodes*/,
                                 bool /*add_to_neighbor*/,
                                 bool /*add_to_adjacent*/)
{
  // TODO: mark frag face edges
  // also need to check if cut has been added to this frag face edge or neighbor edge of adjacent
  // face
}

void
EFAElement3D::checkNeighborFaceCut(unsigned int face_id,
                                   unsigned int edge_id,
                                   double position,
                                   EFANode * from_node,
                                   EFANode * embedded_node,
                                   EFANode *& local_embedded)
{
  // N.B. this is important. We are checking if the corresponding edge of the neighbor face or of
  // the adjacent
  // face's neighbor face has a cut at the same position. If so, use the existing embedded node as
  // local_embedded
  for (unsigned int en_iter = 0; en_iter < numFaceNeighbors(face_id); ++en_iter)
  {
    EFAElement3D * face_neighbor = getFaceNeighbor(face_id, en_iter);
    unsigned int neigh_face_id = face_neighbor->getNeighborIndex(this);
    unsigned neigh_edge_id = getNeighborFaceEdgeID(face_id, edge_id, face_neighbor);
    EFAEdge * neigh_edge = face_neighbor->getFace(neigh_face_id)->getEdge(neigh_edge_id);

    if (neigh_edge->hasIntersectionAtPosition(position, from_node))
    {
      unsigned int emb_id = neigh_edge->getEmbeddedNodeIndex(position, from_node);
      EFANode * old_emb = neigh_edge->getEmbeddedNode(emb_id);

      if (embedded_node && embedded_node != old_emb)
        EFAError(
            "attempting to add edge intersection when one already exists with different node.");
      if (local_embedded && local_embedded != old_emb)
        EFAError("attempting to assign contradictory pointer to local_embedded.");

      local_embedded = old_emb;
    }
  } // en_iter
}

void
EFAElement3D::mapParametricCoordinateFrom2DTo3D(unsigned int face_id,
                                                std::vector<double> & xi_2d,
                                                std::vector<double> & xi_3d) const
{
  // given the coords of a point in a 2D face, translate it to 3D parametric coords
  xi_3d.resize(3, 0.0);
  if (_num_faces == 6)
  {
    if (face_id == 0)
    {
      xi_3d[0] = xi_2d[1];
      xi_3d[1] = xi_2d[0];
      xi_3d[2] = -1.0;
    }
    else if (face_id == 1)
    {
      xi_3d[0] = xi_2d[0];
      xi_3d[1] = -1.0;
      xi_3d[2] = xi_2d[1];
    }
    else if (face_id == 2)
    {
      xi_3d[0] = 1.0;
      xi_3d[1] = xi_2d[0];
      xi_3d[2] = xi_2d[1];
    }
    else if (face_id == 3)
    {
      xi_3d[0] = -xi_2d[0];
      xi_3d[1] = 1.0;
      xi_3d[2] = xi_2d[1];
    }
    else if (face_id == 4)
    {
      xi_3d[0] = -1.0;
      xi_3d[1] = -xi_2d[0];
      xi_3d[2] = xi_2d[1];
    }
    else if (face_id == 5)
    {
      xi_3d[0] = xi_2d[0];
      xi_3d[1] = xi_2d[1];
      xi_3d[2] = 1.0;
    }
    else
      EFAError("face_id out of bounds");
  }
  else if (_num_faces == 4)
  {
    if (face_id == 0)
    {
      xi_3d[0] = xi_2d[0];
      xi_3d[1] = xi_2d[1];
      xi_3d[2] = 0.0;
    }
    else if (face_id == 1)
    {
      xi_3d[0] = 0.0;
      xi_3d[1] = xi_2d[0];
      xi_3d[2] = xi_2d[1];
    }
    else if (face_id == 2)
    {
      xi_3d[0] = xi_2d[1];
      xi_3d[1] = 0.0;
      xi_3d[2] = xi_2d[0];
    }
    else if (face_id == 3)
    {
      xi_3d[0] = xi_2d[0];
      xi_3d[1] = xi_2d[2];
      xi_3d[2] = xi_2d[1];
    }
    else
      EFAError("face_id out of bounds");
  }
  else
    EFAError("unknown element for 3D");
}

std::vector<EFANode *>
EFAElement3D::getCommonNodes(const EFAElement3D * other_elem) const
{
  std::set<EFANode *> e1nodes(_nodes.begin(),
                              _nodes.begin() + _num_vertices); // only account for corner nodes
  std::set<EFANode *> e2nodes(other_elem->_nodes.begin(),
                              other_elem->_nodes.begin() + _num_vertices);
  std::vector<EFANode *> common_nodes = Efa::getCommonElems(e1nodes, e2nodes);
  return common_nodes;
}
