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
#include "EFAelement3D.h"

EFAelement3D::EFAelement3D(unsigned int eid, unsigned int n_nodes, unsigned int n_faces):
  EFAelement(eid, n_nodes),
  _num_faces(n_faces),
  _faces(_num_faces, NULL),
  _face_neighbors(_num_faces, std::vector<EFAelement3D*>(1,NULL))
{}

EFAelement3D::EFAelement3D(const EFAelement3D* from_elem, bool convert_to_local):
  EFAelement(from_elem->_id, from_elem->_num_nodes),
  _num_faces(from_elem->_num_faces),
  _faces(_num_faces, NULL),
  _face_neighbors(_num_faces, std::vector<EFAelement3D*>(1,NULL))
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
        _local_nodes.push_back(_nodes[i]); // convenient to delete local nodes
      }
      else
        mooseError("In EFAelement3D "<<from_elem->id()<<" the copy constructor must have from_elem w/ global nodes. node: "
                    << i << " category: "<<from_elem->_nodes[i]->category());
    }

    // copy faces, fragments and interior nodes from from_elem
    for (unsigned int i = 0; i < _num_faces; ++i)
      _faces[i] = new EFAface(*from_elem->_faces[i]);
    for (unsigned int i = 0; i < from_elem->_fragments.size(); ++i)
      _fragments.push_back(new EFAfragment3D(this, true, from_elem, i));
    for (unsigned int i = 0; i < from_elem->_interior_nodes.size(); ++i)
      _interior_nodes.push_back(new VolumeNode(*from_elem->_interior_nodes[i]));

    // replace all global nodes with local nodes
    for (unsigned int i = 0; i < _num_nodes; ++i)
    {
      if (_nodes[i]->category() == N_CATEGORY_LOCAL_INDEX)
        switchNode(_nodes[i], from_elem->_nodes[i], false);//when save to _cut_elem_map, the EFAelement is not a child of any parent
      else
        mooseError("In EFAelement2D copy constructor this elem's nodes must be local");
    }

    // create element face connectivity array (IMPORTANT)
    create_adjacent_face_ix();
  }
  else
    mooseError("this EFAelement2D constructor only converts global nodes to local nodes");
}

EFAelement3D::~EFAelement3D()
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

unsigned int
EFAelement3D::num_frags() const
{
  return _fragments.size();
}

bool
EFAelement3D::is_partial() const
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
EFAelement3D::get_non_physical_nodes(std::set<EFAnode*> &non_physical_nodes) const
{
  //Any nodes that don't belong to any fragment are non-physical
  //First add all nodes in the element to the set
  for (unsigned int i = 0; i < _nodes.size(); ++i)
    non_physical_nodes.insert(_nodes[i]);

  //Now delete any nodes that are contained in fragments
  std::set<EFAnode*>::iterator sit;
  for (sit = non_physical_nodes.begin(); sit != non_physical_nodes.end(); ++sit)
  {
    for (unsigned int i = 0; i < _fragments.size(); ++i)
    {
      if (_fragments[i]->containsNode(*sit))
      {
        non_physical_nodes.erase(sit);
        break;
      }
    }
  }
}

void
EFAelement3D::switchNode(EFAnode *new_node, EFAnode *old_node,
                         bool descend_to_parent)
{
  // We are not switching any embedded nodes here
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
    EFAelement3D* parent3d = dynamic_cast<EFAelement3D*>(_parent);
    if (!parent3d) mooseError("Failed to dynamic cast to parent3d");

    parent3d->switchNode(new_node,old_node,false);
    for (unsigned int i = 0; i < parent3d->num_faces(); ++i)
    {
      for (unsigned int j = 0; j < parent3d->num_face_neighbors(i); ++j)
      {
        EFAelement3D* face_neighbor = parent3d->get_face_neighbor(i,j);
        for (unsigned int k = 0; k < face_neighbor->num_children(); ++k)
        {
          face_neighbor->get_child(k)->switchNode(new_node,old_node,false);
        } // k
      } // j
    } // i
  }
}

void
EFAelement3D::switchEmbeddedNode(EFAnode *new_emb_node,
                                 EFAnode *old_emb_node)
{
  for (unsigned int i = 0; i < _num_faces; ++i)
    _faces[i]->switchNode(new_emb_node, old_emb_node);
  for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    _interior_nodes[i]->switchNode(new_emb_node, old_emb_node);
  for (unsigned int i = 0; i < _fragments.size(); ++i)
    _fragments[i]->switchNode(new_emb_node, old_emb_node);
}

void
EFAelement3D::getMasterInfo(EFAnode* node, std::vector<EFAnode*> &master_nodes,
                            std::vector<double> &master_weights) const
{
  //Given a EFAnode, return its master nodes and weights
  master_nodes.clear();
  master_weights.clear();
  bool masters_found = false;
  for (unsigned int i = 0; i < _num_faces; ++i) // check element exterior faces
  {
    if (_faces[i]->containsNode(node))
    {
      masters_found = _faces[i]->getMasterInfo(node,master_nodes,master_weights);
      if (masters_found)
        break;
      else
        mooseError("In getMasterInfo: cannot find master nodes in element faces");
    }
  } // i

  if (!masters_found) // check element interior embedded nodes
  {
    for (unsigned int i = 0; i < _interior_nodes.size(); ++i)
    {
      if (_interior_nodes[i]->get_node() == node)
      {
        std::vector<double> xi_3d(3, -100.0);
        for (unsigned int j = 0; j < 3; ++j)
          xi_3d[j]  = _interior_nodes[i]->get_para_coords(j);
        for (unsigned int j = 0; j < _num_nodes; ++j)
        {
          master_nodes.push_back(_nodes[j]);
          double weight = 0.0;
          if (_num_nodes == 8)
            weight = linearHexShape3D(j, xi_3d);
          else if (_num_nodes == 4)
            weight = linearTetShape3D(j, xi_3d);
          else
            mooseError("unknown 3D element");
          master_weights.push_back(weight);
        } // j
        masters_found = true;
        break;
      }
    } // i
  }

  if (!masters_found)
    mooseError("In EFAelement3D::getMaterInfo, cannot find the given EFAnode");
}

unsigned int
EFAelement3D::num_interior_nodes() const
{
  return _interior_nodes.size();
}

bool
EFAelement3D::overlays_elem(const EFAelement* other_elem) const
{
  bool overlays = false;
  const EFAelement3D* other3d = dynamic_cast<const EFAelement3D*>(other_elem);
  if (!other3d) mooseError("failed to dynamic cast to other3d");

  //Find indices of common nodes
  std::vector<unsigned int> common_face_curr = get_common_face_id(other3d);
  if (common_face_curr.size() == 1)
  {
    unsigned int curr_face_id = common_face_curr[0];
    EFAface* curr_face = _faces[curr_face_id];
    unsigned int other_face_id = other3d->get_face_id(curr_face);
    EFAface* other_face = other3d->_faces[other_face_id];
    if (curr_face->is_same_orientation(other_face))
      overlays = true;
  }
  else if (common_face_curr.size() > 1)
  {
    //TODO: We probably need more error checking here.
    overlays = true;
  }
  return overlays;
}

unsigned int
EFAelement3D::get_neighbor_index(const EFAelement * neighbor_elem) const
{
  for (unsigned int i = 0; i < _num_faces; ++i)
    for (unsigned int j = 0; j < _face_neighbors[i].size(); ++j)
      if (_face_neighbors[i][j] == neighbor_elem)
        return i;
  mooseError("in get_neighbor_index() element " << _id << " does not have neighbor "<< neighbor_elem->id());
  return 99999;
}

void
EFAelement3D::clear_neighbors()
{
  for (unsigned int face_iter = 0; face_iter < _num_faces; ++face_iter)
    _face_neighbors[face_iter] = std::vector<EFAelement3D*>(1,NULL);
}

void
EFAelement3D::setup_neighbors(std::map<EFAnode*, std::set<EFAelement*> > &InverseConnectivityMap)
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
    EFAelement3D* neigh_elem = dynamic_cast<EFAelement3D*>(*eit2);
    if (!neigh_elem) mooseError("neighbor_elem is not of EFAelement2D type");

    if (neigh_elem != this)
    {
      std::vector<unsigned int> common_face_id = get_common_face_id(neigh_elem);
      if (common_face_id.size() == 1 && !overlays_elem(neigh_elem))
      {
        unsigned int face_id = common_face_id[0];
        bool is_face_neighbor = false;

        //Fragments must match up.
        if ((_fragments.size() > 1) || (neigh_elem->num_frags() > 1))
          mooseError("in updateFaceNeighbors: Cannot have more than 1 fragment");
        else if ((_fragments.size() == 1) && (neigh_elem->num_frags() == 1))
        {
          if (_fragments[0]->isConnected(neigh_elem->get_fragment(0)))
            is_face_neighbor = true;
        }
        else //If there are no fragments to match up, consider them neighbors
          is_face_neighbor = true;

        if (is_face_neighbor)
        {
          if (_face_neighbors[face_id][0])
          {
            if (_face_neighbors[face_id].size() > 1)
            {
              std::cout<<"Neighbor: "<<neigh_elem->id()<<std::endl;
              mooseError("Element "<<_id<<" already has 2 face neighbors: "
                          <<_face_neighbors[face_id][0]->id()<<" "
                          <<_face_neighbors[face_id][1]->id());
            }
            _face_neighbors[face_id].push_back(neigh_elem);
          }
          else
            _face_neighbors[face_id][0] = neigh_elem;
        }
      }
    }
  } // eit2
}

void
EFAelement3D::neighbor_sanity_check() const
{
  for (unsigned int face_iter = 0; face_iter < _num_faces; ++face_iter)
  {
    for (unsigned int en_iter = 0; en_iter < _face_neighbors[face_iter].size(); ++en_iter)
    {
      EFAelement3D* neigh_elem = _face_neighbors[face_iter][en_iter];
      if (neigh_elem != NULL)
      {
        bool found_neighbor = false;
        for (unsigned int face_iter2 = 0; face_iter2 < neigh_elem->num_faces(); ++face_iter2)
        {
          for (unsigned int en_iter2 = 0; en_iter2 < neigh_elem->num_face_neighbors(face_iter2); ++en_iter2)
          {
            if (neigh_elem->get_face_neighbor(face_iter2, en_iter2) == this)
            {
              if ((en_iter2 > 1) && (en_iter > 1))
              {
                mooseError("Element and neighbor element cannot both have >1 neighbors on a common face");
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

void
EFAelement3D::init_crack_tip(std::set<EFAelement*> &CrackTipElements)
{
  if (is_crack_tip_elem())
  {
    CrackTipElements.insert(this);
    for (unsigned int face_iter = 0; face_iter < _num_faces; ++face_iter)
    {
      if ((_face_neighbors[face_iter].size() == 2) &&
          (_faces[face_iter]->has_intersection()))
      {
        //Neither neighbor overlays current element.  We are on the uncut element ahead of the tip.
        //Flag neighbors as crack tip split elements and add this element as their crack tip neighbor.
        if  (_face_neighbors[face_iter][0]->overlays_elem(this) ||
             _face_neighbors[face_iter][1]->overlays_elem(this))
	  mooseError("Element has a neighbor that overlays itself");

        //Make sure the current elment hasn't been flagged as a tip element
        if (_crack_tip_split_element)
          mooseError("crack_tip_split_element already flagged.  In elem: "<<_id
		   << " flags: "<<_crack_tip_split_element
		   <<" "<<_face_neighbors[face_iter][0]->is_crack_tip_split()
		   <<" "<<_face_neighbors[face_iter][1]->is_crack_tip_split());

        _face_neighbors[face_iter][0]->set_crack_tip_split();
        _face_neighbors[face_iter][1]->set_crack_tip_split();

        _face_neighbors[face_iter][0]->add_crack_tip_neighbor(this);
        _face_neighbors[face_iter][1]->add_crack_tip_neighbor(this);
      }
    } // face_iter
  }
}

bool
EFAelement3D::should_duplicate_for_crack_tip(const std::set<EFAelement*> &CrackTipElements)
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
    if (sit == CrackTipElements.end() && is_crack_tip_elem())
      should_duplicate = true;
    else if (shouldDuplicateCrackTipSplitElem())
      should_duplicate = true;
    else if (shouldDuplicateForPhantomCorner())
      should_duplicate = true;
  }
  return should_duplicate;
}

bool
EFAelement3D::shouldDuplicateCrackTipSplitElem()
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
      for (unsigned int i = 0; i < _num_faces; ++i)
      {
        for (unsigned int j = 0; j < num_face_neighbors(i); ++j)
        {
          EFAelement3D* face_neighbor = get_face_neighbor(i,j);
          std::vector<unsigned int> neighbor_split_neighbors;
          if (face_neighbor->will_crack_tip_extend(neighbor_split_neighbors))
          {
            for (unsigned int k = 0; k < neighbor_split_neighbors.size(); ++k)
            {
              //Get the nodes on the crack tip face
              std::set<EFAnode*> face_nodes = face_neighbor->get_face_nodes(neighbor_split_neighbors[k]);
              crack_tip_face_nodes.insert(face_nodes.begin(), face_nodes.end());
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
EFAelement3D::shouldDuplicateForPhantomCorner()
{
  // if a partial element will be split for a second time and it has two neighbor elements
  // sharing one phantom node with the aforementioned partial element, then the two neighbor 
  // elements should be duplicated
  bool should_duplicate = false;
  if (_fragments.size() == 1 && (!_crack_tip_split_element))
  {
    for (unsigned int i = 0; i < _num_faces; ++i)
    {
      std::set<EFAnode*> phantom_nodes = getPhantomNodeOnFace(i);
      if (phantom_nodes.size() > 0 && num_face_neighbors(i) == 1)
      {
        EFAelement3D * neighbor_elem = _face_neighbors[i][0];
        if (neighbor_elem->num_frags() > 1) // neighbor will be split
        {
          for (unsigned int j = 0; j < neighbor_elem->num_faces(); ++j)
          {
            if (!neighbor_elem->get_face(j)->overlap_with(_faces[i]) &&
                neighbor_elem->num_face_neighbors(j) > 0)
            {
              std::set<EFAnode*> neigh_phantom_nodes = neighbor_elem->getPhantomNodeOnFace(j);
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

bool
EFAelement3D::will_crack_tip_extend(std::vector<unsigned int> &split_neighbors) const
{
  //Determine whether the current element is a crack tip element for which the crack will
  //extend into the next element.
  // N.B. this is called at the beginning of createChildElements
  bool will_extend = false;
  if (_fragments.size() == 1 && _crack_tip_split_element)
  {
    for (unsigned int i = 0; i < _crack_tip_neighbors.size(); ++i)
    {
      unsigned int neigh_idx = _crack_tip_neighbors[i]; // essentially a face_id
      if (num_face_neighbors(neigh_idx) != 1)
        mooseError("in will_crack_tip_extend() element "<<_id<<" has "
                    <<_face_neighbors[neigh_idx].size()<<" neighbors on face "<<neigh_idx);

      EFAelement3D * neighbor_elem = _face_neighbors[neigh_idx][0];
      if (neighbor_elem->num_frags() > 2)
        mooseError("in will_crack_tip_extend() element "<<neighbor_elem->id()<<" has "
                    <<neighbor_elem->num_frags()<<" fragments");
      else if (neighbor_elem->num_frags() == 2)
      {
        EFAfragment3D* neigh_frag1 = neighbor_elem->get_fragment(0);
        EFAfragment3D* neigh_frag2 = neighbor_elem->get_fragment(1);
        std::vector<EFAnode*> neigh_cut_nodes = neigh_frag1->get_common_nodes(neigh_frag2);
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
EFAelement3D::is_crack_tip_elem() const
{
  return frag_has_tip_faces();
}

unsigned int
EFAelement3D::get_num_cuts() const
{
  unsigned int num_cut_faces = 0;
  for (unsigned int i = 0; i < _num_faces; ++i)
    if (_faces[i]->has_intersection())
      num_cut_faces += 1;
  return num_cut_faces;
}

bool
EFAelement3D::is_cut_twice() const
{
  // if an element has been cut twice its fragment must have two interior edges
  bool cut_twice = false;
  if (_fragments.size() > 0)
  {
    unsigned int num_interior_faces = 0;
    for (unsigned int i = 0; i < _fragments[0]->num_faces(); ++i)
    {
      if (_fragments[0]->is_face_interior(i))
        num_interior_faces += 1;
    }
    if (num_interior_faces == 2)
      cut_twice = true;
  }
  return cut_twice;
}

void
EFAelement3D::update_fragments(const std::set<EFAelement*> &CrackTipElements,
                               std::map<unsigned int, EFAnode*> &EmbeddedNodes)
{
  // combine the crack-tip faces in a fragment to a single intersected face
  std::set<EFAelement*>::iterator sit;
  sit = CrackTipElements.find(this);
  if (sit != CrackTipElements.end()) // curr_elem is a crack tip element
  {
    if (_fragments.size() == 1)
      _fragments[0]->combine_tip_faces();
    else
      mooseError("crack tip elem " << _id << " must have 1 fragment");
  }

  // remove the inappropriate embedded nodes on interior faces
  // (MUST DO THIS AFTER combine_tip_faces())
  if (_fragments.size() == 1)
    _fragments[0]->remove_invalid_embedded(EmbeddedNodes);

  // for an element with no fragment, create one fragment identical to the element
  if (_fragments.size() == 0)
      _fragments.push_back(new EFAfragment3D(this, true, this));
  if (_fragments.size() != 1)
    mooseError("Element " << _id << " must have 1 fragment at this point");

  // count fragment's cut faces
  unsigned int num_cut_frag_faces = _fragments[0]->get_num_cuts();
  unsigned int num_frag_faces = _fragments[0]->num_faces();
  if (num_cut_frag_faces > _fragments[0]->num_faces())
    mooseError("In element " << _id <<" there are too many cut fragment faces");

  // leave the uncut frag as it is
  if (num_cut_frag_faces == 0)
  {
    if (!is_partial()) // delete the temp frag for an uncut elem
    {
      delete _fragments[0];
      _fragments.clear();
    }
    return;
  }

  // split one fragment into one or two new fragments
  std::vector<EFAfragment3D*> new_frags = _fragments[0]->split();
  if (new_frags.size() == 1 || new_frags.size() == 2)
  {
    delete _fragments[0]; // delete the old fragment
    _fragments.clear();
    for (unsigned int i = 0; i < new_frags.size(); ++i)
      _fragments.push_back(new_frags[i]);
  }
  else
    mooseError("Number of fragments must be 1 or 2 at this point");

  fragment_sanity_check(num_frag_faces, num_cut_frag_faces);
}

void
EFAelement3D::fragment_sanity_check(unsigned int n_old_frag_faces, 
                                    unsigned int n_old_frag_cuts) const
{
  unsigned int n_interior_nodes = num_interior_nodes();
  if (n_interior_nodes > 0 && n_interior_nodes != 1)
    mooseError("After update_fragments this element has "<<n_interior_nodes<<" interior nodes");

  if (n_old_frag_cuts == 0)
  {
    if (_fragments.size() != 1 ||
        _fragments[0]->num_faces() != n_old_frag_faces)
      mooseError("Incorrect link size for element with 0 cuts");
  }
  else if (frag_has_tip_faces()) // crack tip case
  {
    if (_fragments.size() != 1 ||
        _fragments[0]->num_faces() != n_old_frag_faces + n_old_frag_cuts)
      mooseError("Incorrect link size for element with crack-tip faces");
  }
  else // frag is thoroughly cut
  {
    if (_fragments.size() != 2 ||
       (_fragments[0]->num_faces()+_fragments[1]->num_faces()) != n_old_frag_faces + n_old_frag_cuts + 2)
      mooseError("Incorrect link size for element that has been completely cut");
  }
}

void
EFAelement3D::restore_fragment(const EFAelement* const from_elem)
{
  const EFAelement3D* from_elem3d = dynamic_cast<const EFAelement3D*>(from_elem);
  if (!from_elem3d)
    mooseError("from_elem is not of EFAelement3D type");

  // restore fragments
  if (_fragments.size() != 0)
    mooseError("in restoreFragmentInfo elements must not have any pre-existing fragments");
  for (unsigned int i = 0; i < from_elem3d->num_frags(); ++i)
    _fragments.push_back(new EFAfragment3D(this, true, from_elem3d, i));

  // restore interior nodes
  if (_interior_nodes.size() != 0)
    mooseError("in restoreFragmentInfo elements must not have any pre-exsiting interior nodes");
  for (unsigned int i = 0; i < from_elem3d->_interior_nodes.size(); ++i)
    _interior_nodes.push_back(new VolumeNode(*from_elem3d->_interior_nodes[i]));

  // restore face intersections
  if (get_num_cuts() != 0)
    mooseError("In restoreEdgeIntersection: edge cuts already exist in element " << _id);
  for (unsigned int i = 0; i < _num_faces; ++i)
    _faces[i]->copy_intersection(*from_elem3d->_faces[i]);

  // replace all local nodes with global nodes
  for (unsigned int i = 0; i < from_elem3d->num_nodes(); ++i)
  {
    if (from_elem3d->_nodes[i]->category() == N_CATEGORY_LOCAL_INDEX)
      switchNode(_nodes[i], from_elem3d->_nodes[i], false); //EFAelement is not a child of any parent
    else
      mooseError("In restoreFragmentInfo all of from_elem's nodes must be local");
  }
}

void
EFAelement3D::create_child(const std::set<EFAelement*> &CrackTipElements,
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

      EFAelement3D* childElem = new EFAelement3D(new_elem_id, this->num_nodes(), this->num_faces());
      newChildElements.insert(std::make_pair(new_elem_id, childElem));

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
      EFAfragment3D* new_frag = new EFAfragment3D(childElem, true, this, ichild);
      childElem->_fragments.push_back(new_frag);

      // get child element's faces and create _adjacent_face_ix
      childElem->createFaces();
      for (unsigned int j = 0; j < _num_faces; ++j)
        childElem->_faces[j]->copy_intersection(*_faces[j]);
      childElem->remove_phantom_embedded_nodes(); // IMPORTANT

      // inherit old interior nodes
      for (unsigned int j = 0; j < _interior_nodes.size(); ++j)
        childElem->_interior_nodes.push_back(new VolumeNode(*_interior_nodes[j]));
    }
  }
  else //num_links == 1 || num_links == 0
  {
    //child is itself - but don't insert into the list of ChildElements!!!
    _children.push_back(this);
  }
}

void
EFAelement3D::remove_phantom_embedded_nodes()
{
  // remove the embedded nodes on faces that are outside the real domain
  if (_fragments.size() > 0)
  {
    for (unsigned int i = 0; i < _num_faces; ++i)
    {
      // get emb nodes to be removed on edges
      std::vector<EFAnode*> nodes_to_delete;
      for (unsigned int j = 0; j < _faces[i]->num_edges(); ++j)
      {
        EFAedge* edge = _faces[i]->get_edge(j);
        for (unsigned int k = 0; k < edge->num_embedded_nodes(); ++k)
        {
          if (!_fragments[0]->containsNode(edge->get_embedded_node(k)))
            nodes_to_delete.push_back(edge->get_embedded_node(k));
        } // k
      } // j

      // get emb nodes to be removed in the face interior
      for (unsigned int j = 0; j < _faces[i]->num_interior_nodes(); ++j)
      {
        EFAnode* face_node = _faces[i]->get_interior_node(j)->get_node();
        if (!_fragments[0]->containsNode(face_node))
          nodes_to_delete.push_back(face_node);
      } // j

      // remove all invalid embedded nodes
      for (unsigned int j = 0; j < nodes_to_delete.size(); ++j)
        _faces[i]->remove_embedded_node(nodes_to_delete[j]);
    } // i
  }
}

void
EFAelement3D::connect_neighbors(std::map<unsigned int, EFAnode*> &PermanentNodes,
                                std::map<unsigned int, EFAnode*> &EmbeddedNodes,
                                std::map<unsigned int, EFAnode*> &TempNodes,
                                std::map<EFAnode*, std::set<EFAelement*> > &InverseConnectivityMap,
                                bool merge_phantom_faces)
{
  // N.B. "this" must point to a child element that was just created
  if (!_parent)
    mooseError("no parent element for child element " << _id << " in connect_neighbors");
  EFAelement3D* parent3d = dynamic_cast<EFAelement3D*>(_parent);
  if (!parent3d)
    mooseError("cannot dynamic cast to parent3d in connect_neighbors");

  //First loop through edges and merge nodes with neighbors as appropriate
  for (unsigned int j = 0; j < _num_faces; ++j)
  {
    for (unsigned int k = 0; k < parent3d->num_face_neighbors(j); ++k)
    {   
      EFAelement3D* NeighborElem = parent3d->get_face_neighbor(j,k);
      unsigned int neighbor_face_id = NeighborElem->get_neighbor_index(parent3d);

      if (_faces[j]->has_intersection())
      {
        for (unsigned int l = 0; l < NeighborElem->num_children(); ++l)
        {
          EFAelement3D *childOfNeighborElem = dynamic_cast<EFAelement3D*>(NeighborElem->get_child(l));
          if (!childOfNeighborElem) mooseError("dynamic cast childOfNeighborElem fails");

          //Check to see if the nodes are already merged.  There's nothing else to do in that case.
          EFAface* neighborChildFace = childOfNeighborElem->get_face(neighbor_face_id);
          if (_faces[j]->overlap_with(neighborChildFace))
            continue;

          if (_fragments[0]->isConnected(childOfNeighborElem->get_fragment(0)))
          {
            for (unsigned int i = 0; i < _faces[j]->num_nodes(); ++i)
            {
              unsigned int childNodeIndex = i;
              unsigned int neighborChildNodeIndex = parent3d->getNeighborFaceNodeID(j, childNodeIndex, NeighborElem);

              EFAnode* childNode = _faces[j]->get_node(childNodeIndex);
              EFAnode* childOfNeighborNode = neighborChildFace->get_node(neighborChildNodeIndex);
              mergeNodes(childNode, childOfNeighborNode, childOfNeighborElem, PermanentNodes, TempNodes);
            } // i
          }
        } // l, loop over NeighborElem's children
      }
      else //No edge intersection -- optionally merge non-material nodes if they share a common parent
      {
        if (merge_phantom_faces)
        {
          for (unsigned int l = 0; l < NeighborElem->num_children(); ++l)
          {
            EFAelement3D *childOfNeighborElem = dynamic_cast<EFAelement3D*>(NeighborElem->get_child(l));
            if (!childOfNeighborElem) mooseError("dynamic cast childOfNeighborElem fails");

            EFAface *neighborChildFace = childOfNeighborElem->get_face(neighbor_face_id);
            if (!neighborChildFace->has_intersection()) //neighbor face must NOT have intersection either
            {
              //Check to see if the nodes are already merged.  There's nothing else to do in that case.
              if (_faces[j]->overlap_with(neighborChildFace))
                continue;

              for (unsigned int i = 0; i < _faces[j]->num_nodes(); ++i)
              {
                unsigned int childNodeIndex = i;
                unsigned int neighborChildNodeIndex = parent3d->getNeighborFaceNodeID(j, childNodeIndex, NeighborElem);

                EFAnode* childNode = _faces[j]->get_node(childNodeIndex);
                EFAnode* childOfNeighborNode = neighborChildFace->get_node(neighborChildNodeIndex);

                if (childNode->parent() != NULL &&
                    childNode->parent() == childOfNeighborNode->parent()) //non-material node and both come from same parent
                  mergeNodes(childNode, childOfNeighborNode, childOfNeighborElem,
                             PermanentNodes, TempNodes);
              } // i
            }
          } // loop over NeighborElem's children
        } // if (merge_phantom_edges)
      } // IF edge-j has_intersection()
    } // k, loop over neighbors on edge j
  } // j, loop over all faces

  //Now do a second loop through faces and convert remaining nodes to permanent nodes.
  //If there is no neighbor on that face, also duplicate the embedded node if it exists
  for (unsigned int j = 0; j < _num_nodes; ++j)
  {
    EFAnode* childNode = _nodes[j];
    if (childNode->category() == N_CATEGORY_TEMP)
    {
      // if current child element does not have siblings, and if current temp node is a lonely one
      // this temp node should be merged back to its parent permanent node. Otherwise we would have
      // permanent nodes that are not connected to any element
      std::set<EFAelement*> patch_elems = InverseConnectivityMap[childNode->parent()];
      if (parent3d->num_frags() == 1 && patch_elems.size() == 1)
        switchNode(childNode->parent(), childNode);
      else
      {
        unsigned int new_node_id = getNewID(PermanentNodes);
        EFAnode* newNode = new EFAnode(new_node_id,N_CATEGORY_PERMANENT,childNode->parent());
        PermanentNodes.insert(std::make_pair(new_node_id,newNode));
        switchNode(newNode, childNode);
      }
      if (!deleteFromMap(TempNodes, childNode))
        mooseError("Attempted to delete node: "<<childNode->id()<<" from TempNodes, but couldn't find it");
    }
  } // j
}

void
EFAelement3D::print_elem()
{
  // first line: all elem faces
  std::cout << std::setw(5);
  std::cout << _id << "| ";
  for (unsigned int j = 0; j < _num_faces; ++j)
  {
    for (unsigned int k = 0; k < _faces[j]->num_nodes(); ++k)
      std::cout << std::setw(5) << _faces[j]->get_node(k)->id_cat_str();
    std::cout << " | ";
  }
  std::cout << std::endl;

  // second line: emb nodes in all faces + neighbor of each face
  std::cout << std::setw(5);
  std::cout << "embd" << "| ";
  for (unsigned int j = 0; j < _num_faces; ++j)
  {
    std::cout<<std::setw(4);
    for (unsigned int k = 0; k < _faces[j]->num_edges(); ++k)
    {
      if (_faces[j]->get_edge(k)->has_intersection())
      {
        if (_faces[j]->get_edge(k)->num_embedded_nodes() > 1)
        {
          std::cout << "[";
          for (unsigned int l = 0; l < _faces[j]->get_edge(k)->num_embedded_nodes(); ++l)
          {
            std::cout << _faces[j]->get_edge(k)->get_embedded_node(l)->id();
            if (l == _faces[j]->get_edge(k)->num_embedded_nodes()-1)
              std::cout<<"]";
            else
              std::cout<<" ";
          } // l
        }
        else
          std::cout << _faces[j]->get_edge(k)->get_embedded_node(0)->id() << " ";
      }
      else
        std::cout << "  -- ";
    } // k
    std::cout << ", ";

    if (num_face_neighbors(j) > 1)
    {
      std::cout << "[";
      for (unsigned int k = 0; k < num_face_neighbors(j); ++k)
      {
        std::cout << get_face_neighbor(j,k)->id();
        if (k == num_face_neighbors(j)-1)
          std::cout<<"]";
        else
          std::cout<<" ";
      }
    }
    else
    {
      std::cout<<std::setw(4);
      if (num_face_neighbors(j) == 1)
        std::cout << get_face_neighbor(j,0)->id() << " ";
      else
        std::cout << "  -- ";
    }
    std::cout << " | ";
  } // j
  std::cout << std::endl;

  // third line: fragments
  std::cout << std::setw(5);
  for (unsigned int j = 0; j < _fragments.size(); ++j)
  {
    std::cout << "frag" << j << "| ";
    for (unsigned int k = 0; k < _fragments[j]->num_faces(); ++k)
    {
      for (unsigned int l = 0; l < _fragments[j]->get_face(k)->num_nodes(); ++l)
        std::cout << std::setw(5) << _fragments[j]->get_face(k)->get_node(l)->id_cat_str();
      std::cout << " | ";
    }
    std::cout << std::endl;
  }
}

EFAfragment3D*
EFAelement3D::get_fragment(unsigned int frag_id) const
{
  if (frag_id < _fragments.size())
    return _fragments[frag_id];
  else
    mooseError("frag_id out of bounds");
}

std::set<EFAnode*>
EFAelement3D::get_face_nodes(unsigned int face_id) const
{
  std::set<EFAnode*> face_nodes;
  for (unsigned int i = 0; i < _faces[face_id]->num_nodes(); ++i)
    face_nodes.insert(_faces[face_id]->get_node(i));
  return face_nodes;
}

bool
EFAelement3D::getFaceNodeParaCoor(EFAnode* node, std::vector<double> &xi_3d) const
{
  //get the parametric coords of a node in an element face
  unsigned int face_id = 99999;
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
    std::vector<double> xi_2d(2,0.0);
    if (_faces[face_id]->getFaceNodeParaCoor(node, xi_2d))
      mapParaCoorFrom2Dto3D(face_id, xi_2d, xi_3d);
    else
      mooseError("failed to get the 2D para coords on the face");
  }
  return face_found;
}

VolumeNode*
EFAelement3D::get_interior_node(unsigned int interior_node_id) const
{
  if (interior_node_id < _interior_nodes.size())
    return _interior_nodes[interior_node_id];
  else
    mooseError("interior_node_id out of bounds");
}

void
EFAelement3D::remove_embedded_node(EFAnode* emb_node, bool remove_for_neighbor)
{
  for (unsigned int i = 0; i < _fragments.size(); ++i)
    _fragments[i]->remove_embedded_node(emb_node);

  for (unsigned int i = 0; i < _faces.size(); ++i)
    _faces[i]->remove_embedded_node(emb_node);

  if (remove_for_neighbor)
  {
    for (unsigned int i = 0; i < num_faces(); ++i)
      for (unsigned int j = 0; j < num_face_neighbors(i); ++j)
        get_face_neighbor(i,j)->remove_embedded_node(emb_node, false);
  }
}

bool
EFAelement3D::is_cut_third_times() const
{
  // if an element has been cut third times its fragment must have 3 interior faces
  bool cut_third = false;
  if (_fragments.size() > 0)
  {
    unsigned int num_interior_faces = 0;
    for (unsigned int i = 0; i < _fragments[0]->num_faces(); ++i)
    {
      if (_fragments[0]->is_face_interior(i))
        num_interior_faces += 1;
    }
    if (num_interior_faces == 3)
      cut_third = true;
  }
  return cut_third;
}

unsigned int
EFAelement3D::num_faces() const
{
  return _faces.size();
}

void
EFAelement3D::set_face(unsigned int face_id, EFAface* face)
{
  _faces[face_id] = face;
}

void
EFAelement3D::createFaces()
{
  // create element faces based on existing element nodes
  int hex_ix[6][4] = {{0,3,2,1},{0,1,5,4},{1,2,6,5},{2,3,7,6},{3,0,4,7},{4,5,6,7}};
  int tet_ix[4][3] = {{0,2,1},{0,1,3},{1,2,3},{2,0,3}};

  _faces = std::vector<EFAface*>(_num_faces, NULL);
  if (_num_nodes == 8)
  {
    if (_num_faces != 6)
      mooseError("num_faces of hexes must be 6");
    for (unsigned int i = 0; i < _num_faces; ++i)
    {
      _faces[i] = new EFAface(4);
      for (unsigned int j = 0; j < 4; ++j)
        _faces[i]->set_node(j, _nodes[hex_ix[i][j]]);
      _faces[i]->createEdges();
    }
  }
  else if (_num_nodes == 4)
  {
    if (_num_faces != 4)
      mooseError("num_faces of tets must be 4");
    for (unsigned int i = 0; i < _num_faces; ++i)
    {
      _faces[i] = new EFAface(3);
      for (unsigned int j = 0; j < 3; ++j)
        _faces[i]->set_node(j, _nodes[tet_ix[i][j]]);
      _faces[i]->createEdges();
    }
  }
  else
    mooseError("unknown 3D element type in createFaces()");

  // create element face connectivity array
  create_adjacent_face_ix(); // IMPORTANT
}

EFAface*
EFAelement3D::get_face(unsigned int face_id) const
{
  return _faces[face_id];
}

unsigned int
EFAelement3D::get_face_id(EFAface* face) const
{
  for (unsigned int iface = 0; iface < _num_faces; ++iface)
    if (_faces[iface]->overlap_with(face))
      return iface;
  mooseError("input face not found in get_face_id()");
  return 99999;
}

std::vector<unsigned int>
EFAelement3D::get_common_face_id(const EFAelement3D* other_elem) const
{
  std::vector<unsigned int> face_id;
  for (unsigned int i = 0; i < _num_faces; ++i)
  {
    for (unsigned int j = 0; j < other_elem->_num_faces; ++j)
    {
      if (_faces[i]->overlap_with(other_elem->_faces[j]))
      {
        face_id.push_back(i);
        break;
      }
    } // j
  } // i
  return face_id;
}

unsigned int
EFAelement3D::getNeighborFaceNodeID(unsigned int face_id, unsigned int node_id,
                                    EFAelement3D* neighbor_elem) const
{
  // get the corresponding node_id on the corresponding face of neighbor_elem
  unsigned int neigh_face_node_id = 99999;
  unsigned int common_face_id = get_neighbor_index(neighbor_elem);
  if (common_face_id == face_id)
  {
    unsigned int neigh_face_id = neighbor_elem->get_neighbor_index(this);
    EFAface* neigh_face = neighbor_elem->get_face(neigh_face_id);
    for (unsigned int i = 0; i < neigh_face->num_nodes(); ++i)
    {
      if (_faces[face_id]->get_node(node_id) == neigh_face->get_node(i))
      {
        neigh_face_node_id = i;
        break;
      }
    } // i
  }
  else
    mooseError("getNeighborFaceNodeID: neighbor_elem is not a neighbor on face_id");
  return neigh_face_node_id;
}

unsigned int
EFAelement3D::getNeighborFaceEdgeID(unsigned int face_id, unsigned int edge_id,
                                    EFAelement3D* neighbor_elem) const
{
  // get the corresponding edge_id on the corresponding face of neighbor_elem
  unsigned int neigh_face_edge_id = 99999;
  unsigned int common_face_id = get_neighbor_index(neighbor_elem);
  if (common_face_id == face_id)
  {
    unsigned int neigh_face_id = neighbor_elem->get_neighbor_index(this);
    EFAface* neigh_face = neighbor_elem->get_face(neigh_face_id);
    for (unsigned int i = 0; i < neigh_face->num_edges(); ++i)
    {
      if (_faces[face_id]->get_edge(edge_id)->isOverlapping(*neigh_face->get_edge(i)))
      {
        neigh_face_edge_id = i;
        break;
      }
    } // i
  }
  else
    mooseError("getNeighborFaceEdgeID: neighbor_elem is not a neighbor on face_id");
  return neigh_face_edge_id;
}

void
EFAelement3D::create_adjacent_face_ix()
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
EFAelement3D::get_adjacent_face(unsigned int face_id, unsigned int edge_id) const
{
  return _adjacent_face_ix[face_id][edge_id];
}

EFAface*
EFAelement3D::get_frag_face(unsigned int frag_id, unsigned int face_id) const
{
  if (frag_id < _fragments.size())
    return _fragments[frag_id]->get_face(face_id);
  else
    mooseError("frag_id out of bounds in get_frag_face()");
}

std::set<EFAnode*>
EFAelement3D::getPhantomNodeOnFace(unsigned int face_id) const
{
  std::set<EFAnode*> phantom_nodes;
  if (_fragments.size() > 0)
  {
    for (unsigned int j = 0; j < _faces[face_id]->num_nodes(); ++j) // loop ove 2 edge nodes
    {
      bool node_in_frag = false;
      for (unsigned int k = 0; k < _fragments.size(); ++k)
      {
        if (_fragments[k]->containsNode(_faces[face_id]->get_node(j)))
        {
          node_in_frag = true;
          break;
        }
      } // k
      if (!node_in_frag)
        phantom_nodes.insert(_faces[face_id]->get_node(j));
    } // j
  }
  return phantom_nodes;
}

bool
EFAelement3D::getFragmentFaceID(unsigned int elem_face_id, unsigned int &frag_face_id) const
{
  // find the fragment face that is contained by given element edge
  // N.B. if the elem edge contains two frag edges, this method will only return
  // the first frag edge ID
  bool frag_face_found = false;
  frag_face_id = 99999;
  if (_fragments.size() == 1)
  {
    for (unsigned int j = 0; j < _fragments[0]->num_faces(); ++j)
    {
      if (_faces[elem_face_id]->containsFace(_fragments[0]->get_face(j)))
      {
        frag_face_id = j;
        frag_face_found = true;
        break;
      }
    } // j
  }
  return frag_face_found;
}

bool
EFAelement3D::is_face_phantom(unsigned int face_id) const
{
  bool is_phantom = false;
  if (_fragments.size() > 0)
  {
    bool contain_frag_face = false;
    for (unsigned int i = 0; i < _fragments.size(); ++i)
    {
      for (unsigned int j = 0; j < _fragments[i]->num_faces(); ++j)
      {
        if (_faces[face_id]->containsFace(_fragments[i]->get_face(j)))
        {
          contain_frag_face = true;
          break;
        }
      } // j
      if (contain_frag_face) break;
    } // i
    if (!contain_frag_face) is_phantom = true;
  }
  return is_phantom;
}

unsigned int
EFAelement3D::num_face_neighbors(unsigned int face_id) const
{
  unsigned int num_neighbors = 0;
  if (_face_neighbors[face_id][0])
    num_neighbors = _face_neighbors[face_id].size();
  return num_neighbors;
}

EFAelement3D*
EFAelement3D::get_face_neighbor(unsigned int face_id, unsigned int neighbor_id) const
{
  if (_face_neighbors[face_id][0] != NULL && neighbor_id < _face_neighbors[face_id].size())
    return _face_neighbors[face_id][neighbor_id];
  else
    mooseError("edge neighbor does not exist");
}

bool
EFAelement3D::frag_has_tip_faces() const
{
  bool has_tip_faces = false;
  if (_fragments.size() == 1)
  {
    for (unsigned int i = 0; i < _num_faces; ++i)
    {
      unsigned int num_frag_faces = 0; // count how many fragment edges this element edge contains
      if (_faces[i]->has_intersection())
      {
        for (unsigned int j = 0; j < _fragments[0]->num_faces(); ++j)
        {
          if (_faces[i]->containsFace(_fragments[0]->get_face(j)))
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
EFAelement3D::get_tip_face_id() const
{
  // if this element is a crack tip element, returns the crack tip faces' ID
  std::vector<unsigned int> tip_face_id;
  if (_fragments.size() == 1) // crack tip element with a partial fragment saved
  {
    for (unsigned int i = 0; i < _num_faces; ++i)
    {
      unsigned int num_frag_faces = 0; // count how many fragment faces this element edge contains
      if (_faces[i]->has_intersection())
      {
        for (unsigned int j = 0; j < _fragments[0]->num_faces(); ++j)
        {
          if (_faces[i]->containsFace(_fragments[0]->get_face(j)))
            num_frag_faces += 1;
        } // j
        if (num_frag_faces == 2) // element face contains two fragment edges
          tip_face_id.push_back(i);
      }
    } // i
  }
  return tip_face_id;
}

std::set<EFAnode*>
EFAelement3D::get_tip_embedded_nodes() const
{
  // if this element is a crack tip element, returns the crack tip edge's ID
  std::set<EFAnode*> tip_emb;
  if (_fragments.size() == 1) // crack tip element with a partial fragment saved
  {
    for (unsigned int i = 0; i < _num_faces; ++i)
    {
      std::vector<EFAface*> frag_faces; // count how many fragment edges this element edge contains
      if (_faces[i]->has_intersection())
      {
        for (unsigned int j = 0; j < _fragments[0]->num_faces(); ++j)
        {
          if (_faces[i]->containsFace(_fragments[0]->get_face(j)))
            frag_faces.push_back(_fragments[0]->get_face(j));
        } // j
        if (frag_faces.size() == 2) // element edge contains two fragment edges
        {
          unsigned int edge_id = frag_faces[0]->adjacentCommonEdge(frag_faces[1]);
          tip_emb.insert(frag_faces[0]->get_edge(edge_id)->get_node(0));
          tip_emb.insert(frag_faces[0]->get_edge(edge_id)->get_node(1));
        }
      }
    } // i
  }
  return tip_emb;
}

bool
EFAelement3D::face_contains_tip(unsigned int face_id) const
{
  bool contain_tip = false;
  if (_fragments.size() == 1)
  {
    unsigned int num_frag_faces = 0; // count how many fragment faces this element face contains
    if (_faces[face_id]->has_intersection())
    {
      for (unsigned int j = 0; j < _fragments[0]->num_faces(); ++j)
      {
        if (_faces[face_id]->containsFace(_fragments[0]->get_face(j)))
          num_frag_faces += 1;
      } // j
      if (num_frag_faces == 2)
        contain_tip = true;
    }
  }
  return contain_tip;
}

void
EFAelement3D::addFaceEdgeCut(unsigned int face_id, unsigned int edge_id, double position,
                             EFAnode* embedded_node, std::map<unsigned int, EFAnode*> &EmbeddedNodes,
                             bool add_to_neighbor, bool add_to_adjacent)
{
  EFAnode* local_embedded = NULL;
  EFAedge* cut_edge = _faces[face_id]->get_edge(edge_id);
  EFAnode* edge_node1 = cut_edge->get_node(0);
  if (embedded_node) // use the existing embedded node if it was passed in
    local_embedded = embedded_node;

  // get adjacent face info
  EFAface* adj_face = get_adjacent_face(face_id, edge_id);
  unsigned int adj_face_id = get_face_id(adj_face);
  unsigned int adj_edge_id = adj_face->adjacentCommonEdge(_faces[face_id]);

  // check if cut has already been added to this face edge
  bool cut_exist = false;
  if (cut_edge->has_intersection_at_position(position, edge_node1))
  {
    unsigned int emb_id = cut_edge->get_embedded_index(position, edge_node1);
    EFAnode* old_emb = cut_edge->get_embedded_node(emb_id);
    if (embedded_node && embedded_node != old_emb)
      mooseError("Attempting to add edge intersection when one already exists with different node."
                 << " elem: "<<_id<<" edge: "<<edge_id<<" position: "<<position);
    local_embedded = old_emb;
    cut_exist = true;
  }
  
  if (!cut_exist)
  {
    // check if cut has already been added to the neighbor edges
    checkNeighborFaceCut(face_id, edge_id, position, edge_node1, embedded_node, local_embedded);
    checkNeighborFaceCut(adj_face_id, adj_edge_id, position, edge_node1, embedded_node, local_embedded);
  
    if (!local_embedded) // need to create new embedded node
    {
      unsigned int new_node_id = getNewID(EmbeddedNodes);
      local_embedded = new EFAnode(new_node_id,N_CATEGORY_EMBEDDED);
      EmbeddedNodes.insert(std::make_pair(new_node_id, local_embedded));
    }

    // add to elem face edge
    cut_edge->add_intersection(position, local_embedded, edge_node1);
    if (cut_edge->num_embedded_nodes() > 2)
      mooseError("element edge can't have >2 embedded nodes");

    // add to adjacent face edge
    if (add_to_adjacent)
    {
      double adj_pos = 1.0 - position;
      addFaceEdgeCut(adj_face_id, adj_edge_id, adj_pos, local_embedded, EmbeddedNodes, false, false);
    }
  }

  // add cut to neighbor face edge
  if (add_to_neighbor)
  {
    for (unsigned int en_iter = 0; en_iter < num_face_neighbors(face_id); ++en_iter)
    {
      EFAelement3D *face_neighbor = get_face_neighbor(face_id, en_iter);
      unsigned int neigh_face_id = face_neighbor->get_neighbor_index(this);
      unsigned neigh_edge_id = getNeighborFaceEdgeID(face_id, edge_id, face_neighbor);
      double neigh_pos = 1.0 - position; // get emb node's postion on neighbor edge
      face_neighbor->addFaceEdgeCut(neigh_face_id, neigh_edge_id, neigh_pos, local_embedded, 
                                    EmbeddedNodes, false, true);
    } // en_iter
  } // If add_to_neighbor required
}

void
EFAelement3D::addFragFaceEdgeCut(unsigned int frag_face_id, unsigned int frag_edge_id, double position,
                                std::map<unsigned int, EFAnode*> &EmbeddedNodes, bool add_to_neighbor,
                                bool add_to_adjacent)
{
  // TODO: mark frag face edges
  // also need to check if cut has been added to this frag face edge or neighbor edge of adjacent face
}

void
EFAelement3D::checkNeighborFaceCut(unsigned int face_id, unsigned int edge_id, double position,
                                   EFAnode* from_node, EFAnode* embedded_node, EFAnode* &local_embedded)
{
  // N.B. this is important. We are checking if the corresponding edge of the neighbor face or of the adjacent
  // face's neighbor face has a cut at the same potition. If so, use the existing embedded node as local_embedded
  for (unsigned int en_iter = 0; en_iter < num_face_neighbors(face_id); ++en_iter)
  {
    EFAelement3D* face_neighbor = get_face_neighbor(face_id, en_iter);
    unsigned int neigh_face_id = face_neighbor->get_neighbor_index(this);
    unsigned neigh_edge_id = getNeighborFaceEdgeID(face_id, edge_id, face_neighbor);
    EFAedge* neigh_edge = face_neighbor->get_face(neigh_face_id)->get_edge(neigh_edge_id);

    if (neigh_edge->has_intersection_at_position(position, from_node))
    {
      unsigned int emb_id = neigh_edge->get_embedded_index(position, from_node);
      EFAnode* old_emb = neigh_edge->get_embedded_node(emb_id);

      if (embedded_node && embedded_node != old_emb)
        mooseError("attempting to add edge intersection when one already exists with different node.");
      if (local_embedded && local_embedded != old_emb)
        mooseError("attempting to assign contradictory pointer to local_embedded.");

      local_embedded = old_emb;
    }
  } // en_iter
}

void
EFAelement3D::mapParaCoorFrom2Dto3D(unsigned int face_id, std::vector<double> &xi_2d,
                                    std::vector<double> &xi_3d) const
{
  // given the 1D parent coord of a point in an 2D element edge, translate it to 2D para coords
  xi_3d.resize(3,0.0);
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
      mooseError("face_id out of bounds");
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
    else if(face_id == 2)
    {
      xi_3d[0] = xi_2d[1];
      xi_3d[1] = 0.0;
      xi_3d[2] = xi_2d[0];
    }
    else if(face_id == 3)
    {
      xi_3d[0] = xi_2d[0];
      xi_3d[1] = xi_2d[2];
      xi_3d[2] = xi_2d[1];
    }
    else
      mooseError("face_id out of bounds");
  }
  else
    mooseError("unknown element for 3D");
}
