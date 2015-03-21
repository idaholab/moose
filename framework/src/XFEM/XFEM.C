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

#include <cstdlib> // *must* precede <cmath> for proper std:abs() on PGI, Sun Studio CC
#include <cmath> // for isnan(), when it's defined
#include <limits>

// Local includes
#include "libmesh/libmesh_config.h"

#include "libmesh/boundary_info.h"
#include "libmesh/error_vector.h"
#include "libmesh/libmesh_logging.h"
#include "libmesh/mesh_base.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/mesh_refinement.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_ghost_sync.h"
#include "libmesh/remote_elem.h"

#include "libmesh/elem.h"
#include "libmesh/node.h"

#include "XFEM.h"
#include "XFEM_geometric_cut.h"

#ifdef DEBUG
// Some extra validation for ParallelMesh
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel_mesh.h"
#endif // DEBUG

// XFEM mesh modification methods
XFEM::XFEM (std::vector<MaterialData *> & material_data, MeshBase* m, MeshBase* m2) :
  _material_data(material_data),
  _mesh(m),
  _mesh2(m2)
{
}

XFEM::~XFEM ()
{
  for (unsigned int i=0; i<_geometric_cuts.size(); ++i)
  {
    delete _geometric_cuts[i];
    _geometric_cuts[i] = NULL;
  }
  _geometric_cuts.clear();
}

void
XFEM::setSecondMesh(MeshBase* mesh2)
{
  if (_mesh2)
  {
    libMesh::err << " ERROR: _mesh2 already set"<<std::endl;
    exit(1);
  }
  _mesh2=mesh2;
}

void
XFEM::addGeometricCut(XFEM_geometric_cut* geometric_cut)
{
  _geometric_cuts.push_back(geometric_cut);
}

void
XFEM::addStateMarkedElem(unsigned int elem_id, RealVectorValue normal)
{
  Elem *elem = _mesh->elem(elem_id);
  std::map<const Elem*, RealVectorValue>::iterator mit;
  mit = _state_marked_elems.find(elem);
  if (mit != _state_marked_elems.end())
  {
    libMesh::err << " ERROR: element "<<elem->id()<<" already marked for crack growth."<<std::endl;
    exit(1);
  }
  _state_marked_elems[elem] = normal;
}

void
XFEM::addStateMarkedElem(unsigned int elem_id, RealVectorValue normal, unsigned int marked_side)
{
  addStateMarkedElem(elem_id, normal);
  Elem *elem = _mesh->elem(elem_id);
  std::map<const Elem*, unsigned int>::iterator mit;
  mit = _state_marked_elem_sides.find(elem);
  if (mit != _state_marked_elem_sides.end())
  {
    libMesh::err << " ERROR: side of element "<<elem->id()<<" already marked for crack initiation."<<std::endl;
    exit(1);
  }
  _state_marked_elem_sides[elem] = marked_side;
}

void
XFEM::addStateMarkedFrag(unsigned int elem_id, RealVectorValue normal)
{
  addStateMarkedElem(elem_id, normal);
  Elem *elem = _mesh->elem(elem_id);
  std::set<const Elem*>::iterator mit;
  mit = _state_marked_frags.find(elem);
  if (mit != _state_marked_frags.end())
  {
    libMesh::err << " ERROR: element "<<elem->id()<<" already marked for fragment-secondary crack initiation."<<std::endl;
    exit(1);
  }
  _state_marked_frags.insert(elem);
}

void
XFEM::clearStateMarkedElems()
{
  _state_marked_elems.clear();
  _state_marked_frags.clear();
  _state_marked_elem_sides.clear();
}

bool
XFEM::update(Real time)
{
//  parallel_only();

  bool mesh_changed = false;

  _new_nodes_map.init(*_mesh);
  if (_mesh2)
  {
    _new_nodes_map2.init(*_mesh2);
  }

  build_efa_mesh();
  // DEBUG
//  std::cout << "***** after efa mesh built *****" << std::endl;
//  _efa_mesh.printMesh();
  if (mark_cut_edges(time))
  {
    mesh_changed = cut_mesh_with_efa();
  }

  if (mesh_changed)
  {
    _mesh->update_parallel_id_counts();
    MeshCommunication().make_elems_parallel_consistent(*_mesh);
    MeshCommunication().make_nodes_parallel_consistent(*_mesh,_new_nodes_map);
    _new_nodes_map.clear();
//    _mesh->find_neighbors();
//    _mesh->contract();
    _mesh->prepare_for_use(true); //doing this preserves the numbering, but generates warning

    if (_mesh2)
    {
      _mesh2->update_parallel_id_counts();
      MeshCommunication().make_elems_parallel_consistent(*_mesh2);
      MeshCommunication().make_nodes_parallel_consistent(*_mesh2,_new_nodes_map2);
      _new_nodes_map2.clear();
      _mesh2->prepare_for_use(true);
    }
  }
  clearStateMarkedElems();

  return mesh_changed;
}

void XFEM::build_efa_mesh()
{

  _efa_mesh.reset();
  _efa_mesh.set_dimension(_mesh->mesh_dimension());

  MeshBase::element_iterator       elem_it  = _mesh->elements_begin();
  const MeshBase::element_iterator elem_end = _mesh->elements_end();

  //Load all existing elements in to EFA mesh
  for ( elem_it = _mesh->elements_begin(); elem_it != elem_end; ++elem_it)
  {
    Elem *elem = *elem_it;
    std::vector<unsigned int> quad;
    for (unsigned int i = 0; i < elem->n_nodes(); ++i)
      quad.push_back(elem->node(i));
    if (_mesh->mesh_dimension() == 2)
      _efa_mesh.add2DElement(quad, elem->id());
    else if (_mesh->mesh_dimension() == 3)
    {
     // _efa_mesh.add3DElement(quad, elem->id());
    }
    else
    {
      libMesh::err << " ERROR: XFEM.C only works for 2D and 3D" << std::endl;
      exit(1);
    }
  }

  //Restore fragment information for elements that have been previously cut
  for ( elem_it = _mesh->elements_begin(); elem_it != elem_end; ++elem_it)
  {
    Elem *elem = *elem_it;
    std::map<const Elem*, XFEMCutElem*>::iterator cemit = _cut_elem_map.find(elem);
    if (cemit != _cut_elem_map.end())
    {
      XFEMCutElem *xfce = cemit->second;
      EFAelement * CEMElem = _efa_mesh.getElemByID(elem->id());
      _efa_mesh.restoreFragmentInfo(CEMElem, xfce->get_efa_elem());
    }
  }

  //Must update edge neighbors before restore edge intersections. Otherwise, when we
  //add edge intersections, we do not have neighbor information to use.
  //Correction: no need to use neighbor info now 
  _efa_mesh.updateEdgeNeighbors();
  _efa_mesh.initCrackTipTopology();
}

bool
XFEM::mark_cut_edges(Real time)
{
  bool marked_edges = false;
  if (_mesh->mesh_dimension() == 2)
  {
    marked_edges = mark_cut_edges_by_geometry(time);
    marked_edges |= mark_cut_edges_by_state();
  }
  else
  {
    libMesh::err << " ERROR: 3D XFEM cut marking not supported yet"<<std::endl;
    exit(1);
  }
  return marked_edges;
}

bool
XFEM::mark_cut_edges_by_geometry(Real time)
{
  bool marked_edges = false;

  MeshBase::element_iterator       elem_it  = _mesh->elements_begin();
  const MeshBase::element_iterator elem_end = _mesh->elements_end();

  for ( ; elem_it != elem_end; ++elem_it)
  {
    const Elem *elem = *elem_it;
    std::vector<cutEdge> elemCutEdges;
    std::vector<cutEdge> fragCutEdges;
    std::vector<std::vector<Point> > frag_edges;
    EFAelement * EFAelem = _efa_mesh.getElemByID(elem->id());
    EFAelement2D * CEMElem = dynamic_cast<EFAelement2D*>(EFAelem);
    if (!CEMElem)
    {
      libMesh::err << " ERROR: EFAelem is not of EFAelement2D type" << std::endl;
      exit(1);
    }

    // continue if elem has been already cut twice - IMPORTANT
    if (CEMElem->is_cut_twice())
      continue;

    // get fragment edges
    if (CEMElem->num_frags() > 0)
    {
      if (CEMElem->num_frags() > 1)
      {
        libMesh::err << " ERROR: element "<<elem->id()<<" has more than one fragments at this point"<<std::endl;
        exit(1);
      }
      for (unsigned int i = 0; i < CEMElem->get_fragment(0)->num_edges(); ++i)
      {
        std::vector<Point> p_line(2,Point(0.0,0.0,0.0));
        p_line[0] = get_efa_node_coor(CEMElem->get_frag_edge(0,i)->get_node(0), CEMElem, elem);
        p_line[1] = get_efa_node_coor(CEMElem->get_frag_edge(0,i)->get_node(1), CEMElem, elem);
        frag_edges.push_back(p_line);
      } // i
    }

    // mark cut edges for the element and its fragment
    for (unsigned int i=0; i<_geometric_cuts.size(); ++i)
    {
      _geometric_cuts[i]->cut_elem_by_geometry(elem, elemCutEdges, time);
      if (CEMElem->num_frags() > 0)
        _geometric_cuts[i]->cut_frag_by_geometry(frag_edges, fragCutEdges, time);
    }

    for (unsigned int i=0; i<elemCutEdges.size(); ++i)
    {
      if (!CEMElem->is_edge_phantom(elemCutEdges[i].host_side_id)) // must not be phantom edge
      {
        _efa_mesh.addElemEdgeIntersection(elem->id(), elemCutEdges[i].host_side_id,
                                          elemCutEdges[i].distance);
        marked_edges = true;
      }
    }

    for (unsigned int i=0; i<fragCutEdges.size(); ++i) // MUST DO THIS AFTER MARKING ELEMENT EDGES
    {
      if (!CEMElem->get_fragment(0)->isSecondaryInteriorEdge(fragCutEdges[i].host_side_id))
      {
        _efa_mesh.addFragEdgeIntersection(elem->id(), fragCutEdges[i].host_side_id,
                                          fragCutEdges[i].distance);
        marked_edges = true;
      }
    }
  }

  return marked_edges;
}

bool
XFEM::mark_cut_edges_by_state()
{
  bool marked_edges = false;

  std::map<const Elem*, RealVectorValue>::iterator pmeit;
  for (pmeit = _state_marked_elems.begin(); pmeit != _state_marked_elems.end(); ++pmeit)
  {
    const Elem *elem = pmeit->first;
    RealVectorValue &normal = pmeit->second;
    EFAelement * EFAelem = _efa_mesh.getElemByID(elem->id());
    EFAelement2D * CEMElem = dynamic_cast<EFAelement2D*>(EFAelem);
    if (!CEMElem)
    {
      libMesh::err << " ERROR: EFAelem is not of EFAelement2D type" << std::endl;
      exit(1);
    }

    // continue if elem is already cut twice - IMPORTANT
    if (CEMElem->is_cut_twice())
      continue;

    // find the first cut edge
    unsigned int nsides = CEMElem->num_edges();
    unsigned int orig_cut_side_id = 999999;
    Real orig_cut_distance = -1.0;
    EFAnode * orig_node = NULL;
    EFAedge * orig_edge = NULL;

    if (is_elem_at_crack_tip(elem)) // crack tip element's crack intiation
    {
      orig_cut_side_id = CEMElem->get_tip_edge_id();
      if (orig_cut_side_id < nsides) // valid crack-tip edge found
      {
        orig_edge = CEMElem->get_edge(orig_cut_side_id);
        orig_node = CEMElem->get_tip_embedded();
        unsigned int emb_id = orig_edge->get_embedded_index(orig_node);
        orig_cut_distance = orig_edge->get_intersection(emb_id,orig_edge->get_node(0));
      }
      else
      {
        libMesh::err << " ERROR: element "<<elem->id()<<" has no valid crack-tip edge" << std::endl;
        exit(1);
      }
    }
    else
    {
      std::map<const Elem*, unsigned int>::iterator mit1;
      mit1 = _state_marked_elem_sides.find(elem);
      std::set<const Elem*>::iterator mit2;
      mit2 = _state_marked_frags.find(elem);

      if (mit1 != _state_marked_elem_sides.end()) // specified boundary crack initiation
      {
        orig_cut_side_id = mit1->second;
        if (!CEMElem->is_edge_phantom(orig_cut_side_id) &&
            !CEMElem->get_edge(orig_cut_side_id)->has_intersection())
        {
          orig_cut_distance = 0.5;
          _efa_mesh.addElemEdgeIntersection(elem->id(), orig_cut_side_id, orig_cut_distance);
          orig_edge = CEMElem->get_edge(orig_cut_side_id);
          orig_node = orig_edge->get_embedded_node(0);
        }
        else
          continue; // skip this elem if specified boundary edge is phantom
      }
      else if (mit2 != _state_marked_frags.end()) // cut-surface secondary crack initiation
      {
        if (CEMElem->num_frags() != 1)
        {
          libMesh::err << " ERROR: element "<<elem->id()<<" flagged for a secondary crack, but has "
                       <<CEMElem->num_frags()<<" fragments"<<std::endl;
          exit(1);
        }
        std::vector<unsigned int> interior_edge_id = CEMElem->get_fragment(0)->get_interior_edge_id();
        if (interior_edge_id.size() == 1)
          orig_cut_side_id = interior_edge_id[0];
        else
          continue; // skip this elem if more than one interior edges found (i.e. elem's been cut twice)
        orig_cut_distance = 0.5;
        _efa_mesh.addFragEdgeIntersection(elem->id(), orig_cut_side_id, orig_cut_distance);
        orig_edge = CEMElem->get_frag_edge(0,orig_cut_side_id);
        orig_node = orig_edge->get_embedded_node(0); // must be an interior embedded node
      }
      else
      {
        libMesh::err << " ERROR: element " << elem->id() 
                     << " flagged for state-based growth, but has no edge intersections" << std::endl;
        exit(1);
      }
    }
    Point cut_origin(0.0,0.0,0.0);
    if (orig_node)
      cut_origin = get_efa_node_coor(orig_node, CEMElem, elem);// cutting plane origin's coords
    else
    {
      libMesh::err << " ERROR: element "<<elem->id()<<" does not have valid orig_node"<<std::endl;
      exit(1);
    }

    // loop through element edges to add possible second cut points
    std::vector<Point> edge_ends(2,Point(0.0,0.0,0.0));
    for (unsigned int i = 0; i < nsides; ++i)
    {
      if (!orig_edge->isPartialOverlap(*CEMElem->get_edge(i)))
      {
        edge_ends[0] = get_efa_node_coor(CEMElem->get_edge(i)->get_node(0),CEMElem,elem);
        edge_ends[1] = get_efa_node_coor(CEMElem->get_edge(i)->get_node(1),CEMElem,elem);
        Real distance = 0.0;
        if (init_crack_intersect_edge(cut_origin,normal,edge_ends[0],edge_ends[1],distance) &&
           (!CEMElem->is_edge_phantom(i)))
        {
          _efa_mesh.addElemEdgeIntersection(elem->id(), i, distance);
          break;
        }
      }
    } // i

    // loop though framgent boundary edges to add possible second cut points
    // N.B. must do this after marking element edges
    if (CEMElem->num_frags() > 0)
    {
      for (unsigned int i = 0; i < CEMElem->get_fragment(0)->num_edges(); ++i)
      {
        if (!orig_edge->isPartialOverlap(*CEMElem->get_frag_edge(0,i)))
        {
          edge_ends[0] = get_efa_node_coor(CEMElem->get_frag_edge(0,i)->get_node(0),CEMElem,elem);
          edge_ends[1] = get_efa_node_coor(CEMElem->get_frag_edge(0,i)->get_node(1),CEMElem,elem);
          Real distance = 0.0;
          if (init_crack_intersect_edge(cut_origin,normal,edge_ends[0],edge_ends[1],distance) &&
             (!CEMElem->get_fragment(0)->isSecondaryInteriorEdge(i)))
          { 
            _efa_mesh.addFragEdgeIntersection(elem->id(), i, distance);
            break;
          }
        }
      } // i
    }

    marked_edges = true;
  } // loop over all state_marked_elems

  return marked_edges;
}

bool
XFEM::init_crack_intersect_edge(Point cut_origin, RealVectorValue cut_normal, 
                                Point edge_p1, Point edge_p2, Real & dist)
{
  dist = 0.0;
  bool does_intersect = false;
  Point origin2p1 = edge_p1 - cut_origin;
  Real plane2p1 = cut_normal(0)*origin2p1(0) + cut_normal(1)*origin2p1(1);
  Point origin2p2 = edge_p2 - cut_origin;
  Real plane2p2 = cut_normal(0)*origin2p2(0) + cut_normal(1)*origin2p2(1);

  if (plane2p1*plane2p2 < 0.0)
  {
    dist = -plane2p1/(plane2p2 - plane2p1);
    does_intersect = true;
  }
  return does_intersect;
}

bool
XFEM::cut_mesh_with_efa()
{
  bool mesh_changed = false;

  std::map<unsigned int, Node*> efa_id_to_new_node;
  std::map<unsigned int, Node*> efa_id_to_new_node2;
  std::map<unsigned int, Elem*> efa_id_to_new_elem;

  _efa_mesh.updatePhysicalLinksAndFragments();
  // DEBUG
  _efa_mesh.printMesh();
  std::cout<<"BWS before updateTopology"<<std::endl;

  _efa_mesh.updateTopology();
  // DEBUG
  _efa_mesh.printMesh();
  std::cout<<"BWS cut done"<<std::endl;

  //Add new nodes
  const std::vector<EFAnode*> NewNodes = _efa_mesh.getNewNodes();
  for (unsigned int i = 0; i < NewNodes.size(); ++i)
  {
    unsigned int new_node_id = NewNodes[i]->id();
    unsigned int parent_id = NewNodes[i]->parent()->id();

    const Node *parent_node = _mesh->node_ptr(parent_id);

    Point *new_point = new Point(*parent_node);
    Node *new_node = _mesh->add_point(*new_point, DofObject::invalid_id, parent_node->processor_id());

    _new_nodes_map.insert(*new_node);
    new_node->set_n_systems(parent_node->n_systems());
    efa_id_to_new_node.insert(std::make_pair(new_node_id,new_node));
    std::cout<<"XFEM added new node: "<<new_node->id()+1<<std::endl;
    mesh_changed = true;
    if (_mesh2)
    {
      const Node *parent_node2 = _mesh2->node_ptr(parent_id);

      Point *new_point2 = new Point(*parent_node2);
      Node *new_node2 = _mesh2->add_point(*new_point2, DofObject::invalid_id, parent_node2->processor_id());

      _new_nodes_map2.insert(*new_node2);
      new_node2->set_n_systems(parent_node2->n_systems());
      efa_id_to_new_node2.insert(std::make_pair(new_node_id,new_node2));
      std::cout<<"XFEM2 added new node: "<<new_node2->id()+1<<std::endl;
    }
  }

  //Add new elements
  const std::vector<EFAelement*> NewElements = _efa_mesh.getChildElements();
  for (unsigned int i = 0; i < NewElements.size(); ++i)
  {
    unsigned int parent_id = NewElements[i]->parent()->id();
    unsigned int efa_child_id = NewElements[i]->id();

    Elem *parent_elem = _mesh->elem(parent_id);
    Elem *libmesh_elem = Elem::build(parent_elem->type()).release();

    Elem *parent_elem2 = NULL;
    Elem *libmesh_elem2 = NULL;
    if (_mesh2)
    {
      parent_elem2 = _mesh2->elem(parent_id);
      libmesh_elem2 = Elem::build(parent_elem2->type()).release();
    }

    for (unsigned int j = 0; j < NewElements[i]->num_nodes(); ++j)
    {
      unsigned int node_id = NewElements[i]->get_node(j)->id();
      Node *libmesh_node;

      std::map<unsigned int, Node*>::iterator nit = efa_id_to_new_node.find(node_id);
      if (nit != efa_id_to_new_node.end())
        libmesh_node = nit->second;
      else
        libmesh_node = _mesh->node_ptr(node_id);

      libmesh_elem->set_node(j) = libmesh_node;

      Node *parent_node = parent_elem->get_node(j);
      std::vector<boundary_id_type> parent_node_boundary_ids = _mesh->boundary_info->boundary_ids(parent_node);
      _mesh->boundary_info->add_node(libmesh_node, parent_node_boundary_ids);

      if (_mesh2)
      {
        std::map<unsigned int, Node*>::iterator nit2 = efa_id_to_new_node2.find(node_id);
        if (nit2 != efa_id_to_new_node2.end())
          libmesh_node = nit2->second;
        else
          libmesh_node = _mesh2->node_ptr(node_id);

        libmesh_elem2->set_node(j) = libmesh_node;

        parent_node = parent_elem2->get_node(j);
        parent_node_boundary_ids.clear();
        parent_node_boundary_ids = _mesh2->boundary_info->boundary_ids(parent_node);
        _mesh2->boundary_info->add_node(libmesh_node, parent_node_boundary_ids);
      }
    }

    libmesh_elem->set_p_level(parent_elem->p_level());
    libmesh_elem->set_p_refinement_flag(parent_elem->p_refinement_flag());
    _mesh->add_elem(libmesh_elem);
    libmesh_elem->set_n_systems(parent_elem->n_systems());
    libmesh_elem->subdomain_id() = parent_elem->subdomain_id();
    libmesh_elem->processor_id() = parent_elem->processor_id();

    //TODO: The 0 here is the thread ID.  Need to sort out how to do this correctly
    //TODO: Also need to copy surface and neighbor material data
    _material_data[0]->copy(*libmesh_elem, *parent_elem, 0);

    std::cout<<"XFEM added elem "<<libmesh_elem->id()+1<<std::endl;

    XFEMCutElem * xfce;
    if (_mesh->mesh_dimension() == 2)
    {
      EFAelement2D* new_efa_elem2d = dynamic_cast<EFAelement2D*>(NewElements[i]);
      if (!new_efa_elem2d)
      {
        libMesh::err << " ERROR: dynamic_cast to new_efa_elem2d fails" <<std::endl;
        exit(1);
      }
      xfce = new XFEMCutElem2D(libmesh_elem, new_efa_elem2d);
    }
    else if (_mesh->mesh_dimension() == 3)
    {
      libMesh::err << " ERROR: cannot support 3D yet"<<std::endl;
      exit(1);
    }
    _cut_elem_map.insert(std::pair<Elem*,XFEMCutElem*>(libmesh_elem, xfce));
    efa_id_to_new_elem.insert(std::make_pair(efa_child_id, libmesh_elem));

    if (_mesh2)
    {
      libmesh_elem2->set_p_level(parent_elem2->p_level());
      libmesh_elem2->set_p_refinement_flag(parent_elem2->p_refinement_flag());
      _mesh2->add_elem(libmesh_elem2);
      libmesh_elem2->set_n_systems(parent_elem2->n_systems());
      libmesh_elem2->subdomain_id() = parent_elem2->subdomain_id();
      libmesh_elem2->processor_id() = parent_elem2->processor_id();
    }

    unsigned int n_sides = parent_elem->n_sides();
    for (unsigned int side=0; side<n_sides; ++side)
    {
      std::vector<boundary_id_type> parent_elem_boundary_ids = _mesh->boundary_info->boundary_ids(parent_elem, side);
      _mesh->boundary_info->add_side(libmesh_elem, side, parent_elem_boundary_ids);
    }
    if (_mesh2)
    {
      n_sides = parent_elem2->n_sides();
      for (unsigned int side=0; side<n_sides; ++side)
      {
        std::vector<boundary_id_type> parent_elem_boundary_ids = _mesh2->boundary_info->boundary_ids(parent_elem2, side);
        _mesh2->boundary_info->add_side(libmesh_elem2, side, parent_elem_boundary_ids);
      }
    }

    unsigned int n_edges = parent_elem->n_edges();
    for (unsigned int edge=0; edge<n_edges; ++edge)
    {
      std::vector<boundary_id_type> parent_elem_boundary_ids = _mesh->boundary_info->edge_boundary_ids(parent_elem, edge);
      _mesh->boundary_info->add_edge(libmesh_elem, edge, parent_elem_boundary_ids);
    }
    if (_mesh2)
    {
      n_edges = parent_elem2->n_edges();
      for (unsigned int edge=0; edge<n_edges; ++edge)
      {
        std::vector<boundary_id_type> parent_elem_boundary_ids = _mesh2->boundary_info->edge_boundary_ids(parent_elem2, edge);
        _mesh2->boundary_info->add_edge(libmesh_elem2, edge, parent_elem_boundary_ids);
      }
    }

    mesh_changed = true;
  }

  //delete elements
  const std::vector<EFAelement*> DeleteElements = _efa_mesh.getParentElements();
  for (unsigned int i = 0; i < DeleteElements.size(); ++i)
  {
    Elem *elem_to_delete = _mesh->elem(DeleteElements[i]->id());

    //delete the XFEMCutElem object for any elements that are to be deleted
    std::map<const Elem*, XFEMCutElem*>::iterator cemit = _cut_elem_map.find(elem_to_delete);
    if (cemit != _cut_elem_map.end())
    {
      delete cemit->second;
      _cut_elem_map.erase(cemit);
    }

    elem_to_delete->nullify_neighbors();
    _mesh->boundary_info->remove(elem_to_delete);
    _mesh->delete_elem(elem_to_delete);
    std::cout<<"XFEM deleted elem "<<elem_to_delete->id()+1<<std::endl;
    mesh_changed = true;

    if (_mesh2)
    {
      Elem *elem_to_delete2 = _mesh2->elem(DeleteElements[i]->id());
      elem_to_delete2->nullify_neighbors();
      _mesh2->boundary_info->remove(elem_to_delete2);
      _mesh2->delete_elem(elem_to_delete2);
      std::cout<<"XFEM2 deleted elem "<<elem_to_delete2->id()+1<<std::endl;
    }
  }

  //Store information about crack tip elements
  if (mesh_changed)
  {
    _crack_tip_elems.clear();
    const std::set<EFAelement*> CrackTipElements = _efa_mesh.getCrackTipElements();
    std::set<EFAelement*>::const_iterator sit;
    for (sit = CrackTipElements.begin(); sit != CrackTipElements.end(); ++sit)
    {
      unsigned int eid = (*sit)->id();
      Elem * crack_tip_elem;
      std::map<unsigned int, Elem*>::iterator eit = efa_id_to_new_elem.find(eid);
      if (eit != efa_id_to_new_elem.end())
        crack_tip_elem = eit->second;
      else
        crack_tip_elem = _mesh->elem(eid);
      _crack_tip_elems.insert(crack_tip_elem);
    }
  }

  //store virtual nodes
  //store cut edge info

//  std::cout << _efa_mesh << std::endl;

  return mesh_changed;
}

Point
XFEM::get_efa_node_coor(EFAnode* CEMnode, EFAelement* CEMElem, 
                        const Elem *elem, MeshBase* displaced_mesh)
{
  Point node_coor(0.0,0.0,0.0);
  std::vector<EFAnode*> master_nodes;
  std::vector<Point> master_points;
  std::vector<double> master_weights;

  CEMElem->getMasterInfo(CEMnode, master_nodes, master_weights);
  for (unsigned int i = 0; i < master_nodes.size(); ++i)
  {
    if (master_nodes[i]->category() == N_CATEGORY_PERMANENT)
    {
      unsigned int local_node_id = CEMElem->getLocalNodeIndex(master_nodes[i]);
      Node* node = elem->get_node(local_node_id);
      if (displaced_mesh)
        node = displaced_mesh->node_ptr(node->id());
      Point node_p((*node)(0), (*node)(1), (*node)(2));
      master_points.push_back(node_p);
    }
    else
    {
      libMesh::err << " ERROR: master nodes must be permanent"<<std::endl;
      exit(1);
    }
  } // i
  for (unsigned int i = 0; i < master_nodes.size(); ++i)
    node_coor += master_weights[i]*master_points[i];

  return node_coor;
}

Real
XFEM::get_elem_phys_volfrac(const Elem* elem) const
{
  Real phys_volfrac = 1.0;
  std::map<const Elem*, XFEMCutElem*>::const_iterator it;
  it = _cut_elem_map.find(elem);
  if (it != _cut_elem_map.end())
  {
    const XFEMCutElem *xfce = it->second;
    const EFAelement* EFAelem = xfce->get_efa_elem();
    if (EFAelem->is_partial()) // exclude the full crack tip elements
      phys_volfrac = xfce->get_physical_volfrac();
  }

  return phys_volfrac;
}

Real
XFEM::get_cut_plane(const Elem* elem, const XFEM_CUTPLANE_QUANTITY quantity,
                    unsigned int plane_id) const
{
  Real comp=0.0;
  Point planedata(0.0,0.0,0.0);
  std::map<const Elem*, XFEMCutElem*>::const_iterator it;
  it = _cut_elem_map.find(elem);
  if (it != _cut_elem_map.end())
  {
    const XFEMCutElem *xfce = it->second;
    const EFAelement* EFAelem = xfce->get_efa_elem();
    if (EFAelem->is_partial()) // exclude the full crack tip elements
    {
      if ((unsigned int)quantity < 3)
      {
        unsigned int index = (unsigned int)quantity;
        planedata = xfce->get_origin(plane_id,_mesh2);
        comp = planedata(index);
      }
      else if ((unsigned int)quantity < 6)
      {
        unsigned int index = (unsigned int)quantity - 3;
        planedata = xfce->get_normal(plane_id,_mesh2);
        comp = planedata(index);
      }
      else
      {
        libMesh::err << " ERROR: In get_cut_plane index out of range"<<std::endl;
        exit(1);
      }
    }
  }
  return comp;
}

bool
XFEM::is_elem_at_crack_tip(const Elem* elem) const
{
  return (_crack_tip_elems.find(elem) != _crack_tip_elems.end());
}

bool
XFEM::is_elem_cut(const Elem* elem) const
{
  bool is_cut = false;
  std::map<const Elem*, XFEMCutElem*>::const_iterator it;
  it = _cut_elem_map.find(elem);
  if (it != _cut_elem_map.end())
  {
    const XFEMCutElem *xfce = it->second;
    const EFAelement* EFAelem = xfce->get_efa_elem();
    if (EFAelem->is_partial()) // exclude the full crack tip elements
      is_cut = true;
  }
  return is_cut;
}
