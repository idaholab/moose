//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EFAFragment3D.h"

#include "EFAVolumeNode.h"
#include "EFANode.h"
#include "EFAEdge.h"
#include "EFAFace.h"
#include "EFAFuncs.h"
#include "EFAElement3D.h"
#include "EFAError.h"

EFAFragment3D::EFAFragment3D(EFAElement3D * host,
                             bool create_faces,
                             const EFAElement3D * from_host,
                             unsigned int frag_id)
  : EFAFragment(), _host_elem(host)
{
  if (create_faces)
  {
    if (!from_host)
      EFAError("EFAfragment3D constructor must have a from_host to copy from");
    if (frag_id == std::numeric_limits<unsigned int>::max()) // copy the from_host itself
    {
      for (unsigned int i = 0; i < from_host->numFaces(); ++i)
        _faces.push_back(new EFAFace(*from_host->getFace(i)));
    }
    else
    {
      if (frag_id > from_host->numFragments() - 1)
        EFAError("In EFAfragment3D constructor fragment_copy_index out of bounds");
      for (unsigned int i = 0; i < from_host->getFragment(frag_id)->numFaces(); ++i)
        _faces.push_back(new EFAFace(*from_host->getFragmentFace(frag_id, i)));
    }
    findFacesAdjacentToFaces(); // IMPORTANT
  }
}

EFAFragment3D::~EFAFragment3D()
{
  for (unsigned int i = 0; i < _faces.size(); ++i)
  {
    if (_faces[i])
    {
      delete _faces[i];
      _faces[i] = nullptr;
    }
  }
}

void
EFAFragment3D::switchNode(EFANode * new_node, EFANode * old_node)
{
  for (unsigned int i = 0; i < _faces.size(); ++i)
    _faces[i]->switchNode(new_node, old_node);
}

bool
EFAFragment3D::containsNode(EFANode * node) const
{
  bool contains = false;
  for (unsigned int i = 0; i < _faces.size(); ++i)
  {
    if (_faces[i]->containsNode(node))
    {
      contains = true;
      break;
    }
  }
  return contains;
}

unsigned int
EFAFragment3D::getNumCuts() const
{
  unsigned int num_cut_faces = 0;
  for (unsigned int i = 0; i < _faces.size(); ++i)
    if (_faces[i]->hasIntersection())
      num_cut_faces += 1;
  return num_cut_faces;
}

unsigned int
EFAFragment3D::getNumCutNodes() const
{
  mooseError("Not implemented yet for 3D.");
}

std::set<EFANode *>
EFAFragment3D::getAllNodes() const
{
  std::set<EFANode *> all_nodes;
  for (unsigned int i = 0; i < _faces.size(); ++i)
    for (unsigned int j = 0; j < _faces[i]->numNodes(); ++j)
      all_nodes.insert(_faces[i]->getNode(j));
  return all_nodes;
}

bool
EFAFragment3D::isConnected(EFAFragment * other_fragment) const
{
  EFAFragment3D * other_frag3d = dynamic_cast<EFAFragment3D *>(other_fragment);
  if (!other_frag3d)
    EFAError("in isConnected other_fragment is not of type EFAfragement3D");

  for (unsigned int i = 0; i < _faces.size(); ++i)
    for (unsigned int j = 0; j < other_frag3d->numFaces(); ++j)
      if (_faces[i]->equivalent(other_frag3d->_faces[j]))
        return true;

  return false;
}

bool
EFAFragment3D::isEdgeConnected(EFAFragment * other_fragment) const
{
  EFAFragment3D * other_frag3d = dynamic_cast<EFAFragment3D *>(other_fragment);
  if (!other_frag3d)
    EFAError("in isEdgeConnected other_fragment is not of type EFAfragement3D");

  for (unsigned int i = 0; i < _faces.size(); ++i)
    for (unsigned int j = 0; j < _faces[i]->numEdges(); ++j)
      for (unsigned int k = 0; k < other_frag3d->numFaces(); ++k)
        for (unsigned int l = 0; l < other_frag3d->_faces[k]->numEdges(); ++l)
          if (_faces[i]->getEdge(j)->equivalent(*(other_frag3d->_faces[k]->getEdge(l))))
            return true;

  return false;
}

void
EFAFragment3D::removeInvalidEmbeddedNodes(std::map<unsigned int, EFANode *> & EmbeddedNodes,
                                          std::vector<EFANode *> & invalid_emb_out)
{
  // Detect and handle three classes of embedded-node configurations.
  //
  // (A) Lone-edge (emb_faces.size() == 1):
  //     A cut-intersection embedded node should sit on edges of exactly 2 fragment
  //     faces (an element edge is shared by 2 faces in a closed fragment manifold).
  //     A count of 1 is the signature of a multi-cut conflict in addFaceEdgeCut,
  //     where the `fragmentFaceAlreadyCut` gate blocks the second cut's symmetric
  //     propagation across a shared element edge.  We CANNOT drop the EFANode
  //     globally: the same node may sit as a valid cut vertex on another element's
  //     fragment (split() will use it as a cut-plane vertex on that other element,
  //     and getMasterInfo there must still find it).  It may also serve as an
  //     edge endpoint in inherited cut-plane faces.  Instead we fix this fragment
  //     locally by erasing the spurious edge intersection.  The EFANode object
  //     stays alive; other elements that reference it correctly are not perturbed;
  //     this element's element-face edges still list the node so that future
  //     getMasterInfo(node) can locate it.
  //
  // (B) Over-shared (emb_faces.size() > 2):
  //     Non-manifold topology -- the same embedded node sits on edges of more
  //     than 2 fragment faces.  Has not been observed in any failing case; the
  //     local-cleanup approach is unproven here and could mask an upstream bug.
  //     We abort with EFAError until a reproducer exists.
  //
  // (C) Spurious "phantom" cut (size == 2 but no exterior face has multiple cuts):
  //     The 2D-style criterion from the original code.  Here the embedded node
  //     genuinely should be dropped wholesale -- it is not part of any real cut
  //     surface that reaches an exterior face.  These are returned via
  //     invalid_emb_out for the algorithm driver to remove globally.
  if (hasFaceWithOneCut())
  {
    // build a local inverse map for all emb cut nodes in this fragment
    std::map<EFANode *, std::vector<EFAFace *>> emb_inverse_map;
    for (unsigned int i = 0; i < _faces.size(); ++i)
      for (unsigned int j = 0; j < _faces[i]->numEdges(); ++j)
        if (_faces[i]->getEdge(j)->hasIntersection())
        {
          EFANode * emb_node = _faces[i]->getEdge(j)->getEmbeddedNode(0);
          emb_inverse_map[emb_node].push_back(_faces[i]);
        }

    std::map<EFANode *, std::vector<EFAFace *>>::iterator it;
    for (it = emb_inverse_map.begin(); it != emb_inverse_map.end(); ++it)
    {
      EFANode * emb_node = it->first;
      std::vector<EFAFace *> & emb_faces = it->second;
      if (emb_faces.size() == 1)
      {
        // Class A (lone-edge): the topological signature of a multi-cut
        // conflict in addFaceEdgeCut (see the file header).  Locally erase
        // the spurious intersection so this fragment becomes consistent.
        // We do NOT touch EmbeddedNodes, _host_elem element faces, neighbours,
        // or invalid_emb_out -- the EFANode is valid global state and may be a
        // correct cut on other elements' fragments.
        for (EFAFace * lone_face : emb_faces)
          lone_face->removeEmbeddedNode(emb_node);
        continue;
      }
      if (emb_faces.size() > 2)
      {
        // Over-shared: the same embedded node is reported on edges of more
        // than 2 fragment faces.  This is a non-manifold topology that has
        // not been characterised in any failing case we've seen, and silently
        // dropping it could mask a different upstream bug.  Keep the abort
        // until a reproducer is in hand.
        EFAError("EFAFragment3D::removeInvalidEmbeddedNodes: embedded node ",
                 emb_node->id(),
                 " is shared by ",
                 emb_faces.size(),
                 " fragment faces in element ",
                 (_host_elem ? (int)_host_elem->id() : -1),
                 " (expected 2). Non-manifold fragment topology -- needs investigation.");
      }
      unsigned int counter = 0;
      for (unsigned int i = 0; i < emb_faces.size(); ++i)
      {
        unsigned int face_id = getFaceID(emb_faces[i]);
        if (!isFaceInterior(face_id) && emb_faces[i]->hasIntersection())
          counter += 1; // count the appropriate emb's faces
      }
      if (counter == 0)
        invalid_emb_out.push_back(emb_node);
    }
  }
  // Silence unused-parameter warnings when no invalid emb is collected.
  (void)EmbeddedNodes;
}

void
EFAFragment3D::combine_tip_faces()
{
  if (!_host_elem)
    EFAError("In combine_tip_faces() the frag must have host_elem");

  for (unsigned int i = 0; i < _host_elem->numFaces(); ++i)
  {
    std::vector<unsigned int> frag_tip_face_id;
    for (unsigned int j = 0; j < _faces.size(); ++j)
      if (_host_elem->getFace(i)->containsFace(_faces[j]))
        frag_tip_face_id.push_back(j);

    if (frag_tip_face_id.size() == 2) // combine the two frag faces on this elem face
      combine_two_faces(frag_tip_face_id[0], frag_tip_face_id[1], _host_elem->getFace(i));
  }
  // TODO: may need to combine other frag faces that have tip edges
}

bool
EFAFragment3D::isFaceInterior(unsigned int face_id) const
{
  if (!_host_elem)
    EFAError("in isFaceInterior() fragment must have host elem");

  bool face_in_elem_face = false;
  for (unsigned int i = 0; i < _host_elem->numFaces(); ++i)
    if (_host_elem->getFace(i)->containsFace(_faces[face_id]))
    {
      face_in_elem_face = true;
      break;
    }

  // the face is interior if it does not coincide with an element face
  return !face_in_elem_face;
}

std::vector<unsigned int>
EFAFragment3D::get_interior_face_id() const
{
  std::vector<unsigned int> interior_face_id;
  for (unsigned int i = 0; i < _faces.size(); ++i)
    if (isFaceInterior(i))
      interior_face_id.push_back(i);

  return interior_face_id;
}

bool
EFAFragment3D::isThirdInteriorFace(unsigned int face_id) const
{
  if (!_host_elem)
    EFAError("in isThirdInteriorFace fragment must have host elem");

  for (unsigned int i = 0; i < _host_elem->numInteriorNodes(); ++i)
    if (_faces[face_id]->containsNode(_host_elem->getInteriorNode(i)->getNode()))
      return true;

  return false;
}

unsigned int
EFAFragment3D::numFaces() const
{
  return _faces.size();
}

EFAFace *
EFAFragment3D::getFace(unsigned int face_id) const
{
  if (face_id > _faces.size() - 1)
    EFAError("in EFAfragment3D::get_face, index out of bounds");
  return _faces[face_id];
}

unsigned int
EFAFragment3D::getFaceID(EFAFace * face) const
{
  for (unsigned int i = 0; i < _faces.size(); ++i)
    if (_faces[i] == face)
      return i;
  EFAError("face not found in get_face_id()");
}

void
EFAFragment3D::addFace(EFAFace * new_face)
{
  _faces.push_back(new_face);
}

std::set<EFANode *>
EFAFragment3D::getFaceNodes(unsigned int face_id) const
{
  std::set<EFANode *> face_nodes;
  for (unsigned int i = 0; i < _faces[face_id]->numNodes(); ++i)
    face_nodes.insert(_faces[face_id]->getNode(i));
  return face_nodes;
}

EFAElement3D *
EFAFragment3D::getHostElement() const
{
  return _host_elem;
}

std::vector<EFAFragment3D *>
EFAFragment3D::split()
{
  // This method will split one existing fragment into one or two new fragments
  std::vector<EFAFragment3D *> new_fragments;
  std::vector<std::vector<EFAFace *>> all_subfaces;
  for (unsigned int i = 0; i < _faces.size(); ++i)
  {
    std::vector<EFAFace *> subfaces = _faces[i]->split();
    all_subfaces.push_back(subfaces);
  }

  // build new frags
  if (hasFaceWithOneCut()) // "fakely" cut fragment
  {
    EFAFragment3D * new_frag = new EFAFragment3D(_host_elem, false, nullptr);
    for (unsigned int i = 0; i < all_subfaces.size(); ++i)
      for (unsigned int j = 0; j < all_subfaces[i].size(); ++j)
        new_frag->addFace(all_subfaces[i][j]);
    new_frag->findFacesAdjacentToFaces();
    new_fragments.push_back(new_frag);
  }
  else // thoroughly cut fragment
  {
    // find the first face with 2 sub-faces
    EFAFace * start_face1 = nullptr;
    EFAFace * start_face2 = nullptr;
    unsigned int startOldFaceID = 0;
    for (unsigned int i = 0; i < _faces.size(); ++i)
    {
      if (all_subfaces[i].size() == 2)
      {
        start_face1 = all_subfaces[i][0];
        start_face2 = all_subfaces[i][1];
        startOldFaceID = i;
        break;
      }
    } // i
    EFAFragment3D * new_frag1 = connectSubfaces(start_face1, startOldFaceID, all_subfaces);
    EFAFragment3D * new_frag2 = connectSubfaces(start_face2, startOldFaceID, all_subfaces);
    new_fragments.push_back(new_frag1);
    new_fragments.push_back(new_frag2);
  }
  return new_fragments;
}

void
EFAFragment3D::findFacesAdjacentToFaces()
{
  _faces_adjacent_to_faces.clear();
  for (unsigned int i = 0; i < _faces.size(); ++i)
  {
    std::vector<EFAFace *> face_adjacents(_faces[i]->numEdges(), nullptr);
    for (unsigned int j = 0; j < _faces.size(); ++j)
      if (_faces[j] != _faces[i] && _faces[i]->isAdjacent(_faces[j]))
      {
        unsigned int adj_edge = _faces[i]->adjacentCommonEdge(_faces[j]);
        face_adjacents[adj_edge] = _faces[j];
      }

    _faces_adjacent_to_faces.push_back(face_adjacents);
  } // i
}

EFAFace *
EFAFragment3D::getAdjacentFace(unsigned int face_id, unsigned int edge_id) const
{
  return _faces_adjacent_to_faces[face_id][edge_id]; // possibly NULL
}

void
EFAFragment3D::removeEmbeddedNode(EFANode * emb_node)
{
  for (unsigned int i = 0; i < _faces.size(); ++i)
    _faces[i]->removeEmbeddedNode(emb_node);
}

bool
EFAFragment3D::hasFaceWithOneCut() const
{
  // N.B. this method can only be implemented when the fragment has just been marked
  for (unsigned int i = 0; i < _faces.size(); ++i)
    if (_faces[i]->getNumCuts() == 1)
      return true;
  return false;
}

void
EFAFragment3D::getNodeInfo(std::vector<std::vector<unsigned int>> & face_node_indices,
                           std::vector<EFANode *> & nodes) const
{
  // get all nodes' pointers - a vector
  std::set<EFANode *> all_node_set = getAllNodes();
  nodes.resize(all_node_set.size());
  std::copy(all_node_set.begin(), all_node_set.end(), nodes.begin());

  // get face connectivity
  face_node_indices.clear();
  for (unsigned int i = 0; i < _faces.size(); ++i)
  {
    std::vector<unsigned int> line_face_indices;
    for (unsigned int j = 0; j < _faces[i]->numNodes(); ++j)
    {
      EFANode * node = _faces[i]->getNode(j);
      unsigned int vec_index = std::find(nodes.begin(), nodes.end(), node) - nodes.begin();
      line_face_indices.push_back(vec_index);
    }
    face_node_indices.push_back(line_face_indices);
  }
}

EFAFragment3D *
EFAFragment3D::connectSubfaces(EFAFace * start_face,
                               unsigned int startOldFaceID,
                               std::vector<std::vector<EFAFace *>> & subfaces)
{
  // this method is only called in EFAfragment3D::split()
  std::vector<bool> contributed(subfaces.size(), false);
  contributed[startOldFaceID] = true;
  unsigned int num_contrib_faces = 1;
  unsigned int old_num_contrib = 1;
  std::vector<EFAFace *> frag_faces(1, start_face);

  // collect all subfaces connected to start_face
  do
  {
    old_num_contrib = num_contrib_faces;
    for (unsigned int i = 0; i < subfaces.size(); ++i)
    {
      if (!contributed[i]) // old face not contributed to new fragment yet
      {
        bool adjacent_found = false;
        for (unsigned int j = 0; j < subfaces[i].size(); ++j)
        {
          for (unsigned int k = 0; k < frag_faces.size(); ++k)
          {
            if (subfaces[i][j]->isAdjacent(frag_faces[k]))
            {
              adjacent_found = true;
              contributed[i] = true;
              frag_faces.push_back(subfaces[i][j]);
              num_contrib_faces += 1;
              break;
            }
          } // k
          if (adjacent_found)
            break;
        } // j
      }
    } // i, loop over all old faces
  } while (num_contrib_faces != old_num_contrib);

  // get the cut plane face
  std::vector<EFAEdge *> cut_plane_edges;
  EFAFragment3D * new_frag = new EFAFragment3D(_host_elem, false, nullptr);
  for (unsigned int i = 0; i < frag_faces.size(); ++i)
    new_frag->addFace(frag_faces[i]);
  new_frag->findFacesAdjacentToFaces();

  // Collect ALL lone edges from every fragment face.
  //
  // The original code called loneEdgeOnFace() which returns only the FIRST lone edge
  // per face.  That assumption breaks when the fragment contains sub-faces of a
  // previously-established cut-plane face (i.e., crack growth cuts through a face that
  // was itself created as the cut-plane in an earlier growth step).  Such sub-faces
  // inherit multiple perimeter edges from the old cut-plane polygon; all of those
  // perimeter edges are "lone" in the new fragment because no adjacent sub-face claims
  // them.  Collecting only the first lone edge per face leaves the remaining perimeter
  // edges out of the new cut-plane edge set, producing an open chain that cannot be
  // sorted into a cycle by sortEdges().
  //
  // Collecting all lone edges is always correct: in the normal case every fragment face
  // contributes exactly one lone edge (the single new cut edge), so the result is
  // identical to the old behaviour.  In the multi-growth case, every lone edge on every
  // face is a genuine boundary of the new cut plane and must appear in the cycle.
  for (unsigned int i = 0; i < new_frag->numFaces(); ++i)
    for (unsigned int j = 0; j < new_frag->getFace(i)->numEdges(); ++j)
      if (new_frag->getAdjacentFace(i, j) == nullptr)
        cut_plane_edges.push_back(new EFAEdge(*new_frag->getFace(i)->getEdge(j)));

  EFAFace * cut_face = new EFAFace(cut_plane_edges.size());
  for (unsigned int i = 0; i < cut_plane_edges.size(); ++i)
    cut_face->setEdge(i, cut_plane_edges[i]);
  cut_face->sortEdges();
  cut_face->reverseEdges();
  cut_face->createNodes();

  // finalize the new fragment
  new_frag->addFace(cut_face);
  new_frag->findFacesAdjacentToFaces();
  return new_frag;
}

EFAEdge *
EFAFragment3D::loneEdgeOnFace(unsigned int face_id) const
{
  // if any face edge is not shared by any other face, we call it a lone edge
  for (unsigned int i = 0; i < _faces[face_id]->numEdges(); ++i)
    if (_faces_adjacent_to_faces[face_id][i] == nullptr)
      return _faces[face_id]->getEdge(i);
  return nullptr;
}

void
EFAFragment3D::combine_two_faces(unsigned int face_id1,
                                 unsigned int face_id2,
                                 const EFAFace * elem_face)
{
  // get the new full face
  EFAFace * full_face = _faces[face_id1]->combineWithFace(_faces[face_id2]);
  full_face->resetEdgeIntersection(elem_face); // IMPORTANT

  // take care of the common adjacent faces (combine their tip edges)
  std::set<EFAFace *> face1_neigh;
  face1_neigh.insert(_faces_adjacent_to_faces[face_id1].begin(),
                     _faces_adjacent_to_faces[face_id1].end());
  std::set<EFAFace *> face2_neigh;
  face2_neigh.insert(_faces_adjacent_to_faces[face_id2].begin(),
                     _faces_adjacent_to_faces[face_id2].end());
  std::vector<EFAFace *> common_adjacent_faces = Efa::getCommonElems(face1_neigh, face2_neigh);

  for (unsigned int i = 0; i < common_adjacent_faces.size(); ++i)
  {
    EFAFace * comm_face = common_adjacent_faces[i];
    if (comm_face != nullptr)
    {
      unsigned int edge_id1 = comm_face->adjacentCommonEdge(_faces[face_id1]);
      unsigned int edge_id2 = comm_face->adjacentCommonEdge(_faces[face_id2]);
      comm_face->combineTwoEdges(edge_id1, edge_id2);
      comm_face->resetEdgeIntersection(elem_face); // IMPORTANT
    }
  }

  // delete old faces and update private members of EFAfragment3D
  delete _faces[face_id1];
  delete _faces[face_id2];
  _faces[face_id1] = full_face;
  _faces.erase(_faces.begin() + face_id2);
  findFacesAdjacentToFaces(); // rebuild _adjacent_face_ix: IMPORTANT
}
