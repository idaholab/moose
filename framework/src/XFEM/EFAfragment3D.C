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

#include "EFAelement3D.h"
#include "EFAfragment3D.h"
#include "EFAfuncs.h"
#include <typeinfo>

EFAfragment3D::EFAfragment3D(EFAelement3D * host, bool create_faces,
                             const EFAelement3D * from_host, unsigned int frag_id):
  EFAfragment(),
  _host_elem(host)
{
  if (create_faces)
  {
    if (!from_host)
      mooseError("EFAfragment3D constructor must have a from_host to copy from");
    if (frag_id == std::numeric_limits<unsigned int>::max()) // copy the from_host itself
    {
      for (unsigned int i = 0; i < from_host->num_faces(); ++i)
        _faces.push_back(new EFAface(*from_host->get_face(i)));
    }
    else
    {
      if (frag_id > from_host->num_frags() - 1)
        mooseError("In EFAfragment3D constructor fragment_copy_index out of bounds");
      for (unsigned int i = 0; i < from_host->get_fragment(frag_id)->num_faces(); ++i)
        _faces.push_back(new EFAface(*from_host->get_frag_face(frag_id,i)));
    }
    create_adjacent_face_ix(); // IMPORTANT
  }
}

EFAfragment3D::~EFAfragment3D()
{
  for (unsigned int i = 0; i < _faces.size(); ++i)
  {
    if (_faces[i])
    {
      delete _faces[i];
      _faces[i] = NULL;
    }
  }
}

void
EFAfragment3D::switchNode(EFAnode *new_node, EFAnode *old_node)
{
  for (unsigned int i = 0; i < _faces.size(); ++i)
    _faces[i]->switchNode(new_node, old_node);
}

bool
EFAfragment3D::containsNode(EFAnode *node) const
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
EFAfragment3D::get_num_cuts() const
{
  unsigned int num_cut_faces = 0;
  for (unsigned int i = 0; i < _faces.size(); ++i)
    if (_faces[i]->has_intersection())
      num_cut_faces += 1;
  return num_cut_faces;
}

std::set<EFAnode*>
EFAfragment3D::get_all_nodes() const
{
  std::set<EFAnode*> all_nodes;
  for (unsigned int i = 0; i < _faces.size(); ++i)
    for (unsigned int j = 0; j < _faces[i]->num_nodes(); ++j)
      all_nodes.insert(_faces[i]->get_node(j));
  return all_nodes;
}

bool
EFAfragment3D::isConnected(EFAfragment* other_fragment) const
{
  bool is_connected = false;
  EFAfragment3D* other_frag3d = dynamic_cast<EFAfragment3D*>(other_fragment);
  if (!other_frag3d)
    mooseError("in isConnected other_fragment is not of type EFAfragement3D");

  for (unsigned int i = 0; i < _faces.size(); ++i)
  {
    for (unsigned int j = 0; j < other_frag3d->num_faces(); ++j)
    {
      if (_faces[i]->equivalent(other_frag3d->_faces[j]))
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
EFAfragment3D::combine_tip_faces()
{
  if (!_host_elem)
    mooseError("In combine_tip_faces() the frag must have host_elem");

  bool has_tip_faces = false;
  for (unsigned int i = 0; i < _host_elem->num_faces(); ++i)
  {
    std::vector<unsigned int> frag_tip_face_id;
    for (unsigned int j = 0; j < _faces.size(); ++j)
    {
      if (_host_elem->get_face(i)->containsFace(_faces[j]))
        frag_tip_face_id.push_back(j);
    } // j
    if (frag_tip_face_id.size() == 2) // combine the two frag faces on this elem face
      combine_two_faces(frag_tip_face_id[0], frag_tip_face_id[1], _host_elem->get_face(i));
  } // i, loop over all elem faces
  // TODO: may need to combine other frag faces that have tip edges
}

bool
EFAfragment3D::is_face_interior(unsigned int face_id) const
{
  if (!_host_elem)
    mooseError("in is_face_interior() fragment must have host elem");

  bool face_in_elem_face = false;
  for (unsigned int i = 0; i < _host_elem->num_faces(); ++i)
  {
    if (_host_elem->get_face(i)->containsFace(_faces[face_id]))
    {
      face_in_elem_face = true;
      break;
    }
  } // i
  if (!face_in_elem_face)
    return true; // yes, is interior
  else
    return false;
}

std::vector<unsigned int>
EFAfragment3D::get_interior_face_id() const
{
  std::vector<unsigned int> interior_face_id;
  for (unsigned int i = 0; i < _faces.size(); ++i)
  {
    if (is_face_interior(i))
      interior_face_id.push_back(i);
  }
  return interior_face_id;
}

bool
EFAfragment3D::isThirdInteriorFace(unsigned int face_id) const
{
  bool is_third_cut = false;
  if (!_host_elem)
    mooseError("in isThirdInteriorFace fragment must have host elem");

  for (unsigned int i = 0; i < _host_elem->num_interior_nodes(); ++i)
  {
    if (_faces[face_id]->containsNode(_host_elem->get_interior_node(i)->get_node()))
    {
      is_third_cut = true;
      break;
    }
  }
  return is_third_cut;
}

unsigned int
EFAfragment3D::num_faces() const
{
  return _faces.size();
}

EFAface*
EFAfragment3D::get_face(unsigned int face_id) const
{
  if (face_id > _faces.size() - 1)
    mooseError("in EFAfragment3D::get_face, index out of bounds");
  return _faces[face_id];
}

unsigned int
EFAfragment3D::get_face_id(EFAface* face) const
{
  for (unsigned int i = 0; i < _faces.size(); ++i)
    if (_faces[i] == face)
      return i;
  mooseError("face not found in get_face_id()");
  return 99999;
}

void
EFAfragment3D::add_face(EFAface* new_face)
{
  _faces.push_back(new_face);
}

std::set<EFAnode*>
EFAfragment3D::get_face_nodes(unsigned int face_id) const
{
  std::set<EFAnode*> face_nodes;
  for (unsigned int i = 0; i < _faces[face_id]->num_nodes(); ++i)
    face_nodes.insert(_faces[face_id]->get_node(i));
  return face_nodes;
}

EFAelement3D*
EFAfragment3D::get_host() const
{
  return _host_elem;
}

std::vector<EFAfragment3D*>
EFAfragment3D::split()
{
  // This method will split one existing fragment into one or two new fragments
  std::vector<EFAfragment3D*> new_fragments;
  std::vector<std::vector<EFAface*> > all_subfaces;
  for (unsigned int i = 0; i < _faces.size(); ++i)
  {
    std::vector<EFAface*> subfaces = _faces[i]->split();
    all_subfaces.push_back(subfaces);
  }

  // build new frags
  if (hasFaceWithOneCut()) // "fakely" cut fragment
  {
    EFAfragment3D* new_frag = new EFAfragment3D(_host_elem, false, NULL);
    for (unsigned int i = 0; i < all_subfaces.size(); ++i)
      for (unsigned int j = 0; j < all_subfaces[i].size(); ++j)
        new_frag->add_face(all_subfaces[i][j]);
    new_frag->create_adjacent_face_ix();
    new_fragments.push_back(new_frag);
  }
  else // thoroughly cut fragment
  {
    // find the first face with 2 sub-faces
    EFAface* start_face1 = NULL;
    EFAface* start_face2 = NULL;
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
    EFAfragment3D* new_frag1 = connect_subfaces(start_face1, startOldFaceID, all_subfaces);
    EFAfragment3D* new_frag2 = connect_subfaces(start_face2, startOldFaceID, all_subfaces);
    new_fragments.push_back(new_frag1);
    new_fragments.push_back(new_frag2);
  }
  return new_fragments;
}

void
EFAfragment3D::create_adjacent_face_ix()
{
  _adjacent_face_ix.clear();
  for (unsigned int i = 0; i < _faces.size(); ++i)
  {
    std::vector<EFAface*> face_adjacents(_faces[i]->num_edges(), NULL);
    for (unsigned int j = 0; j < _faces.size(); ++j)
    {
      if (_faces[j] != _faces[i] && _faces[i]->isAdjacent(_faces[j]))
      {
        unsigned int adj_edge = _faces[i]->adjacentCommonEdge(_faces[j]);
        face_adjacents[adj_edge] = _faces[j];
      }
    } // j
    _adjacent_face_ix.push_back(face_adjacents);
  }  // i
}

EFAface*
EFAfragment3D::get_adjacent_face(unsigned int face_id, unsigned int edge_id) const
{
  return _adjacent_face_ix[face_id][edge_id]; // possibly NULL
}

void
EFAfragment3D::remove_invalid_embedded(std::map<unsigned int, EFAnode*> &EmbeddedNodes)
{
  // N.B. this method is only called before we update fragments
  // N.B. an embedded node is valid IF at least one of its host faces is exterior and has more than 1 cuts
  // TODO: the invalid cases are generalized from 2D. The method may need improvements in 3D
  if (hasFaceWithOneCut())
  {
    // build a local inverse map for all emb cut nodes in this fragment
    std::map<EFAnode*, std::vector<EFAface*> > emb_inverse_map;
    for (unsigned int i = 0; i < _faces.size(); ++i)
    {
      for (unsigned int j = 0; j < _faces[i]->num_edges(); ++j)
      {
        if (_faces[i]->get_edge(j)->has_intersection())
        {
          EFAnode* emb_node = _faces[i]->get_edge(j)->get_embedded_node(0);
          emb_inverse_map[emb_node].push_back(_faces[i]);
        }
      } // i
    } // j

    // find all invalid embedded nodes
    std::vector<EFAnode*> invalid_emb;
    std::map<EFAnode*, std::vector<EFAface*> >::iterator it;
    for (it = emb_inverse_map.begin(); it != emb_inverse_map.end(); ++it)
    {
      EFAnode* emb_node = it->first;
      std::vector<EFAface*> &emb_faces = it->second;
      if (emb_faces.size() != 2)
        mooseError("one embedded node must be owned by 2 faces");
      unsigned int counter = 0;
      for (unsigned int i = 0; i < emb_faces.size(); ++i)
      {
        unsigned int face_id = get_face_id(emb_faces[i]);
        if (!is_face_interior(face_id) && emb_faces[i]->has_intersection())
          counter += 1; // count the appropriate emb's faces
      } // i
      if (counter == 0)
        invalid_emb.push_back(emb_node);
    } // it

    // delete all invalid emb nodes
    for (unsigned int i = 0; i < invalid_emb.size(); ++i)
    {
      deleteFromMap(EmbeddedNodes, invalid_emb[i]);
      _host_elem->remove_embedded_node(invalid_emb[i], true); // also remove from neighbors
    } // i
  }
}

void
EFAfragment3D::remove_embedded_node(EFAnode* emb_node)
{
  for (unsigned int i = 0; i < _faces.size(); ++i)
    _faces[i]->remove_embedded_node(emb_node);
}

bool
EFAfragment3D::hasFaceWithOneCut() const
{
  // N.B. this method can only be implemented when the fragment has just been marked
  for (unsigned int i = 0; i < _faces.size(); ++i)
    if (_faces[i]->get_num_cuts() == 1)
      return true;
  return false;
}

void
EFAfragment3D::get_node_info(std::vector<std::vector<unsigned int> > &face_node_ix,
                             std::vector<EFAnode*> &nodes) const
{
  // get all nodes' pointers - a vector
  std::set<EFAnode*> all_node_set = get_all_nodes();
  nodes.resize(all_node_set.size());
  std::copy(all_node_set.begin(), all_node_set.end(), nodes.begin());

  // get face connectivity
  face_node_ix.clear();
  for (unsigned int i = 0; i < _faces.size(); ++i)
  {
    std::vector<unsigned int> line_face_ix;
    for (unsigned int j = 0; j < _faces[i]->num_nodes(); ++j)
    {
      EFAnode* node = _faces[i]->get_node(j);
      unsigned int vec_index = std::find(nodes.begin(), nodes.end(), node) - nodes.begin();
      line_face_ix.push_back(vec_index);
    }
    face_node_ix.push_back(line_face_ix);
  }
}

EFAfragment3D*
EFAfragment3D::connect_subfaces(EFAface* start_face, unsigned int startOldFaceID,
                                std::vector<std::vector<EFAface*> > &subfaces)
{
  // this method is only called in EFAfragment3D::split()
  std::vector<bool> contributed(subfaces.size(), false);
  contributed[startOldFaceID] = true;
  unsigned int num_contrib_faces = 1;
  unsigned int old_num_contrib = 1;
  std::vector<EFAface*> frag_faces(1, start_face);

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
          if (adjacent_found) break;
        } // j
      }
    } // i, loop over all old faces
  }
  while (num_contrib_faces != old_num_contrib);

  // get the cut plane face
  std::vector<EFAedge*> cut_plane_edges;
  EFAfragment3D* new_frag = new EFAfragment3D(_host_elem, false, NULL);
  for (unsigned int i = 0; i < frag_faces.size(); ++i)
    new_frag->add_face(frag_faces[i]);
  new_frag->create_adjacent_face_ix();

  for (unsigned int i = 0; i < new_frag->num_faces(); ++i)
  {
    EFAedge* lonely_edge = new_frag->lonelyEdgeOnFace(i);
    if (lonely_edge != NULL) // valid edge
      cut_plane_edges.push_back(new EFAedge(*lonely_edge));
  } // i

  EFAface* cut_face = new EFAface(cut_plane_edges.size());
  for (unsigned int i = 0; i < cut_plane_edges.size(); ++i)
    cut_face->set_edge(i, cut_plane_edges[i]);
  cut_face->sort_edges();
  cut_face->reverse_edges();
  cut_face->createNodes();

  // finalize the new fragment
  new_frag->add_face(cut_face);
  new_frag->create_adjacent_face_ix();
  return new_frag;
}

EFAedge*
EFAfragment3D::lonelyEdgeOnFace(unsigned int face_id) const
{
  // if any face edge is not shared by any other face, we call it a lonely edge
  for (unsigned int i = 0; i < _faces[face_id]->num_edges(); ++i)
    if (_adjacent_face_ix[face_id][i] == NULL)
      return _faces[face_id]->get_edge(i);
  return NULL;
}

void
EFAfragment3D::combine_two_faces(unsigned int face_id1, unsigned int face_id2, const EFAface* elem_face)
{
  // get the new full face
  EFAface* full_face = _faces[face_id1]->combine_with(_faces[face_id2]);
  full_face->reset_edge_intersection(elem_face); // IMPORTANT

  // take care of the common adjacent faces (combine their tip edges)
  std::set<EFAface*> face1_neigh;
  face1_neigh.insert(_adjacent_face_ix[face_id1].begin(), _adjacent_face_ix[face_id1].end());
  std::set<EFAface*> face2_neigh;
  face2_neigh.insert(_adjacent_face_ix[face_id2].begin(), _adjacent_face_ix[face_id2].end());
  std::vector<EFAface*> common_adjacent_faces = get_common_elems(face1_neigh, face2_neigh);

  for (unsigned int i = 0; i < common_adjacent_faces.size(); ++i)
  {
    EFAface* comm_face = common_adjacent_faces[i];
    if (comm_face != NULL)
    {
      unsigned int edge_id1 = comm_face->adjacentCommonEdge(_faces[face_id1]);
      unsigned int edge_id2 = comm_face->adjacentCommonEdge(_faces[face_id2]);
      comm_face->combine_two_edges(edge_id1, edge_id2);
      comm_face->reset_edge_intersection(elem_face); // IMPORTANT
    }
  } // i

  // delete old faces and update private members of EFAfragment3D
  delete _faces[face_id1];
  delete _faces[face_id2];
  _faces[face_id1] = full_face;
  _faces.erase(_faces.begin() + face_id2);
  create_adjacent_face_ix(); // rebuild _adjacent_face_ix: IMPORTANT
}
