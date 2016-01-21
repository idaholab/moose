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
#include "XFEM_geometric_cut_2d.h"
#include "EFAfuncs.h"

#ifdef DEBUG
// Some extra validation for ParallelMesh
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel_mesh.h"
#endif // DEBUG

// XFEM mesh modification methods
XFEM::XFEM (std::vector<MooseSharedPointer<MaterialData> > & material_data, MeshBase* m, MeshBase* m2) :
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
XFEM::get_crack_tip_origin(std::map<unsigned int, const Elem* > & elem_id_crack_tip, std::vector<Point> & crack_front_points)
{
  elem_id_crack_tip.clear();
  crack_front_points.clear();
  crack_front_points.resize(_elem_crack_origin_direction_map.size());

  std::map<const Elem*, std::vector<Point> >::iterator mit1 = _elem_crack_origin_direction_map.begin();
  unsigned int crack_tip_index = 0;
  // This map is used to sort the order in _elem_crack_origin_direction_map such that every process has same order
  std::map<unsigned int, const Elem*> elem_id_map;
 
  int m = -1;
  for (mit1 = _elem_crack_origin_direction_map.begin(); mit1 != _elem_crack_origin_direction_map.end(); mit1++)
  {
    unsigned int elem_id = mit1->first->id();
    if (elem_id > 999999)
    {
      elem_id_map[m] = mit1->first;
      m--;
    }
    else
    {
      elem_id_map[elem_id] = mit1->first;
    }
  }

  std::map<unsigned int, const Elem*> ::iterator mit2 = elem_id_map.begin();
  
  for (; mit2 != elem_id_map.end(); mit2++)
  {
    const Elem* elem = mit2->second;
    mit1 = _elem_crack_origin_direction_map.find(elem);
    if (mit1 != _elem_crack_origin_direction_map.end())
    {
      elem_id_crack_tip[crack_tip_index] = mit1->first;
      crack_front_points[crack_tip_index] = (mit1->second)[0]; // [0] stores origin coordinates and [1] stores direction
      crack_tip_index++;
    }
  }
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

void
XFEM::store_crack_tip_origin_and_direction()
{
  _elem_crack_origin_direction_map.clear();
  std::set<EFAelement*> CrackTipElements = _efa_mesh.getCrackTipElements();
  std::set<EFAelement*>::iterator sit;
  for (sit = CrackTipElements.begin(); sit != CrackTipElements.end(); ++sit)
  {
    if (_mesh->mesh_dimension() == 2){
      EFAelement2D * CEMElem = dynamic_cast<EFAelement2D*>(*sit);
      EFAnode *tip_node = CEMElem->get_tip_embedded();
      unsigned int cts_id = CEMElem->get_crack_tip_split_element_id();

      Point origin(0,0,0);
      Point direction(0,0,0);

      std::map<const Elem*, XFEMCutElem*>::const_iterator it;
      it = _cut_elem_map.find(_mesh->elem(cts_id));
      if (it != _cut_elem_map.end())
      {
        const XFEMCutElem *xfce = it->second;
        const EFAelement* EFAelem = xfce->get_efa_elem();
        if (EFAelem->is_partial()) // exclude the full crack tip elements
        {
          xfce->get_crack_tip_origin_and_direction(tip_node->id(),origin,direction);
        }
      }

      std::vector<Point> tip_data;
      tip_data.push_back(origin);
      tip_data.push_back(direction);
      const Elem* elem = _mesh->elem((*sit)->id());
      _elem_crack_origin_direction_map.insert(std::pair<const Elem*, std::vector<Point> >(elem,tip_data));
    }
  }
}

bool
XFEM::update(Real time)
{
//  parallel_only();

  bool mesh_changed = false;

  build_efa_mesh();

  store_crack_tip_origin_and_direction();

  if (mark_cut_edges(time))
  {
    mesh_changed = cut_mesh_with_efa();
  }

  if (mesh_changed)
  {
    build_efa_mesh();
    store_crack_tip_origin_and_direction();
  }

  if (mesh_changed)
  {
    _mesh->update_parallel_id_counts();
    MeshCommunication().make_elems_parallel_consistent(*_mesh);
    MeshCommunication().make_nodes_parallel_consistent(*_mesh);
//    _mesh->find_neighbors();
//    _mesh->contract();
    _mesh->allow_renumbering(false);
    _mesh->skip_partitioning(true);
    _mesh->prepare_for_use();
//    _mesh->prepare_for_use(true,true); //doing this preserves the numbering, but generates warning

    if (_mesh2)
    {
      _mesh2->update_parallel_id_counts();
      MeshCommunication().make_elems_parallel_consistent(*_mesh2);
      MeshCommunication().make_nodes_parallel_consistent(*_mesh2);
      _mesh2->allow_renumbering(false);
      _mesh2->skip_partitioning(true);
      _mesh2->prepare_for_use();
//      _mesh2->prepare_for_use(true,true);
    }
  }
  
  clearStateMarkedElems();

  return mesh_changed;
}

void XFEM::initSolution(NonlinearSystem & nl, AuxiliarySystem & aux)
{
  const std::vector<MooseVariable *> & nl_vars = nl.getVariables(0); //TODO pass in real thread id?

  nl.serializeSolution();
//  NumericVector<Number> & c_solution = *nl.sys().solution;
  NumericVector<Number> & current_solution = *nl.sys().current_local_solution;
  NumericVector<Number> & old_solution = *nl.sys().old_local_solution;
//  std::cout<<"pre mod current soln: "<<current_solution;
//  std::cout<<"pre mod old soln: "<<old_solution;

  for (std::map<unique_id_type, unique_id_type>::iterator nit = _new_node_to_parent_node.begin();
       nit != _new_node_to_parent_node.end(); ++nit)
  {
    for (unsigned int ivar=0; ivar<nl_vars.size(); ++ivar)
    {
      Node* new_node = getNodeFromUniqueID(nit->first);
      Node* parent_node = getNodeFromUniqueID(nit->second);
//      std::cout<<"BWS new node : "<<new_node->id() << " parent node: "<<parent_node->id()<<std::endl;
      Point *new_point = new Point(*new_node);
      Point *parent_point = new Point(*parent_node);
      if (*new_point != *parent_point)
        mooseError("Points don't match");
      unsigned int new_node_dof = new_node->dof_number(nl.number(), nl_vars[ivar]->number(),0);
      unsigned int parent_node_dof = parent_node->dof_number(nl.number(), nl_vars[ivar]->number(),0);
//      std::cout<<"BWS setting soln : "<<new_node_dof<<" "<<parent_node_dof<<" "<<current_solution(parent_node_dof)<<std::endl;
      if (parent_node->processor_id() == _mesh->processor_id())
      {
        current_solution.set(new_node_dof, current_solution(parent_node_dof));
//      std::cout<<"BWS setting old soln : "<<new_node_dof<<" "<<parent_node_dof<<" "<<old_solution(parent_node_dof)<<std::endl;
        old_solution.set(new_node_dof, old_solution(parent_node_dof));
      }
    }
  }

  current_solution.close();
  old_solution.close();
//  std::cout<<"post mod current soln: "<<current_solution;
//  std::cout<<"post mod old soln: "<<old_solution;
}

Node * XFEM::getNodeFromUniqueID(unique_id_type uid)
{
  Node *matching_node = NULL;
  MeshBase::node_iterator       node_it = _mesh->nodes_begin();
  const MeshBase::node_iterator node_end = _mesh->nodes_end();

  for ( node_it = _mesh->nodes_begin(); node_it != node_end; ++node_it)
  {
    Node *node = *node_it;
    if (node->unique_id() == uid)
    {
      matching_node = node;
      break;
    }
  }
  if (!matching_node)
    mooseError("Couldn't find node matching unique id: "<<uid);
  return matching_node;
}

void XFEM::build_efa_mesh()
{
  _efa_mesh.reset();

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
      _efa_mesh.add3DElement(quad, elem->id());
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
  bool marked_sides = false;
  if (_mesh->mesh_dimension() == 2)
  {
    marked_sides = mark_cut_edges_by_geometry(time);
    marked_sides |= mark_cut_edges_by_state(time);
  }
  else if (_mesh->mesh_dimension() == 3)
  {
    marked_sides = mark_cut_faces_by_geometry(time);
    marked_sides |= mark_cut_faces_by_state();
  }
  return marked_sides;
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
    if (CEMElem->is_final_cut())
      continue;

    // get fragment edges
    get_frag_edges(elem, CEMElem, frag_edges);

    // mark cut edges for the element and its fragment
    for (unsigned int i=0; i<_geometric_cuts.size(); ++i)
    {
      _geometric_cuts[i]->cut_elem_by_geometry(elem, elemCutEdges, time);
      if (CEMElem->num_frags() > 0)
        _geometric_cuts[i]->cut_frag_by_geometry(frag_edges, fragCutEdges, time);
    }

    for (unsigned int i = 0; i < elemCutEdges.size(); ++i) // mark element edges
    {
      if (!CEMElem->is_edge_phantom(elemCutEdges[i].host_side_id)) // must not be phantom edge
      {
        _efa_mesh.addElemEdgeIntersection(elem->id(), elemCutEdges[i].host_side_id,
                                          elemCutEdges[i].distance);
        marked_edges = true;
      }
    }

    for (unsigned int i = 0; i < fragCutEdges.size(); ++i) // MUST DO THIS AFTER MARKING ELEMENT EDGES
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

void
XFEM::correct_crack_extension_angle(const Elem * elem, EFAelement2D * CEMElem, EFAedge * orig_edge, Point normal, Point crack_tip_origin, Point crack_tip_direction, Real & distance_keep, unsigned int & edge_id_keep, Point & normal_keep)
{
  std::vector<Point> edge_ends(2,Point(0.0,0.0,0.0));
  Point edge1(0.0,0.0,0.0);
  Point edge2(0.0,0.0,0.0);
  Point left_angle(0.0,0.0,0.0);
  Point right_angle(0.0,0.0,0.0);
  Point left_angle_normal(0.0,0.0,0.0);
  Point right_angle_normal(0.0,0.0,0.0);
  Point crack_direction_normal(0.0,0.0,0.0);
  Point edge1_to_tip(0.0,0.0,0.0);
  Point edge2_to_tip(0.0,0.0,0.0);
  Point edge1_to_tip_normal(0.0,0.0,0.0);
  Point edge2_to_tip_normal(0.0,0.0,0.0);

  Real cos_45 = std::cos(45.0/180.0*3.14159);
  Real sin_45 = std::sin(45.0/180.0*3.14159);

  left_angle(0) = cos_45*crack_tip_direction(0) - sin_45*crack_tip_direction(1);
  left_angle(1) = sin_45*crack_tip_direction(0) + cos_45*crack_tip_direction(1);

  right_angle(0) =  cos_45*crack_tip_direction(0) + sin_45*crack_tip_direction(1);
  right_angle(1) = -sin_45*crack_tip_direction(0) + cos_45*crack_tip_direction(1);

  left_angle_normal(0) = -left_angle(1);
  left_angle_normal(1) = left_angle(0);

  right_angle_normal(0) = -right_angle(1);
  right_angle_normal(1) = right_angle(0);

  crack_direction_normal(0) = -crack_tip_direction(1);
  crack_direction_normal(1) = crack_tip_direction(0);

  Real angle_min = 0.0;
  Real distance = 0.0;
  unsigned int nsides = CEMElem->num_edges();

  for (unsigned int i = 0; i < nsides; ++i)
  {
    if (!orig_edge->isPartialOverlap(*CEMElem->get_edge(i)))
    {
      edge_ends[0] = get_efa_node_coor(CEMElem->get_edge(i)->get_node(0),CEMElem,elem);
      edge_ends[1] = get_efa_node_coor(CEMElem->get_edge(i)->get_node(1),CEMElem,elem);

      edge1_to_tip = (edge_ends[0]*0.95 + edge_ends[1]*0.05) - crack_tip_origin;
      edge2_to_tip = (edge_ends[0]*0.05 + edge_ends[1]*0.95) - crack_tip_origin;

      edge1_to_tip /= pow(edge1_to_tip.size_sq(),0.5);
      edge2_to_tip /= pow(edge2_to_tip.size_sq(),0.5);

      edge1_to_tip_normal(0) = -edge1_to_tip(1);
      edge1_to_tip_normal(1) = edge1_to_tip(0);

      edge2_to_tip_normal(0) = -edge2_to_tip(1);
      edge2_to_tip_normal(1) = edge2_to_tip(0);

      Real angle_edge1_normal = edge1_to_tip_normal * normal;
      Real angle_edge2_normal = edge2_to_tip_normal * normal;

      if(std::abs(angle_edge1_normal) > std::abs(angle_min) && (edge1_to_tip*crack_tip_direction) > std::cos(45.0/180.0*3.14159))
      {
        edge_id_keep = i;
        distance_keep = 0.05;
        normal_keep = edge1_to_tip_normal;
        angle_min = angle_edge1_normal;
      }
      else if (std::abs(angle_edge2_normal) > std::abs(angle_min) && (edge2_to_tip*crack_tip_direction) > std::cos(45.0/180.0*3.14159))
      {
        edge_id_keep = i;
        distance_keep = 0.95;
        normal_keep = edge2_to_tip_normal;
        angle_min = angle_edge2_normal;
      }

      if (init_crack_intersect_edge(crack_tip_origin,left_angle_normal,edge_ends[0],edge_ends[1],distance) &&  (!CEMElem->is_edge_phantom(i)) )
      {
        if(std::abs(left_angle_normal*normal) > std::abs(angle_min) && (edge1_to_tip*crack_tip_direction) > std::cos(45.0/180.0*3.14159))
        {
          edge_id_keep = i;
          distance_keep = distance;
          normal_keep = left_angle_normal;
          angle_min = left_angle_normal*normal;
        }
      }
      else if (init_crack_intersect_edge(crack_tip_origin,right_angle_normal,edge_ends[0],edge_ends[1],distance) && (!CEMElem->is_edge_phantom(i)))
      {
        if(std::abs(right_angle_normal*normal) > std::abs(angle_min) && (edge2_to_tip*crack_tip_direction) > std::cos(45.0/180.0*3.14159))
        {
          edge_id_keep = i;
          distance_keep = distance;
          normal_keep = right_angle_normal;
          angle_min = right_angle_normal*normal;
        }
      }
      else if (init_crack_intersect_edge(crack_tip_origin,crack_direction_normal,edge_ends[0],edge_ends[1],distance) && (!CEMElem->is_edge_phantom(i)))
      {
        if(std::abs(crack_direction_normal*normal) > std::abs(angle_min) && (crack_tip_direction*crack_tip_direction) > std::cos(45.0/180.0*3.14159))
        {
          edge_id_keep = i;
          distance_keep = distance;
          normal_keep = crack_direction_normal;
          angle_min = crack_direction_normal*normal;
        }
      }
    }
  }

  //avoid small volume fraction cut
  if((distance_keep-0.05) < 0.0)
  {
    distance_keep = 0.05;
  }
  else if((distance_keep - 0.95) > 0.0)
  {
    distance_keep = 0.95;
  }
}


bool
XFEM::mark_cut_edges_by_state(Real time)
{
  bool marked_edges = false;
  std::map<const Elem*, RealVectorValue>::iterator pmeit;
  for (pmeit = _state_marked_elems.begin(); pmeit != _state_marked_elems.end(); ++pmeit)
  {
    const Elem *elem = pmeit->first;
    RealVectorValue normal = pmeit->second;
    EFAelement * EFAelem = _efa_mesh.getElemByID(elem->id());
    EFAelement2D * CEMElem = dynamic_cast<EFAelement2D*>(EFAelem);

    Real volfrac_elem = get_elem_phys_volfrac(elem);
    if (volfrac_elem < 0.25)
      continue;

    if (!CEMElem)
    {
      libMesh::err << " ERROR: EFAelem is not of EFAelement2D type" << std::endl;
      exit(1);
    }

    // continue if elem is already cut twice - IMPORTANT
    if (CEMElem->is_final_cut())
      continue;

    // find the first cut edge
    unsigned int nsides = CEMElem->num_edges();
    unsigned int orig_cut_side_id = 999999;
    Real orig_cut_distance = -1.0;
    EFAnode * orig_node = NULL;
    EFAedge * orig_edge = NULL;

    //crack tip origin coordinates and direction
    Point crack_tip_origin(0,0,0);
    Point crack_tip_direction(0,0,0);

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

      //obtain the crack tip origin coordinates and direction.
      std::map<const Elem*, std::vector<Point> >::iterator ecodm = _elem_crack_origin_direction_map.find(elem);
      if (ecodm != _elem_crack_origin_direction_map.end()){
        crack_tip_origin = (ecodm->second)[0];
        crack_tip_direction = (ecodm->second)[1];
      }
      else{
        mooseError("element " << elem->id() << " cannot find its crack tip origin and direction.");
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
          //get a virtual crack tip direction
          Point elem_center(0.0,0.0,0.0);
          Point edge_center;
          for (unsigned int i = 0; i < nsides; ++i){
            elem_center += get_efa_node_coor(CEMElem->get_edge(i)->get_node(0),CEMElem,elem);
            elem_center += get_efa_node_coor(CEMElem->get_edge(i)->get_node(1),CEMElem,elem);
          }
          elem_center /= nsides*2.0;
          edge_center = get_efa_node_coor(orig_edge->get_node(0),CEMElem,elem) + get_efa_node_coor(orig_edge->get_node(1),CEMElem,elem);
          edge_center /= 2.0;
          crack_tip_origin = edge_center;
          crack_tip_direction = elem_center - edge_center;
          crack_tip_direction /= pow(crack_tip_direction.size_sq(),0.5);
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
        Point elem_center(0.0,0.0,0.0);
        Point edge_center;
        unsigned int nsides_frag = CEMElem->get_fragment(0)->num_edges();
        for (unsigned int i = 0; i < nsides_frag; ++i){
          elem_center += get_efa_node_coor(CEMElem->get_frag_edge(0,i)->get_node(0),CEMElem,elem);
          elem_center += get_efa_node_coor(CEMElem->get_frag_edge(0,i)->get_node(1),CEMElem,elem);
        }
        elem_center /= nsides_frag*2.0;
        edge_center = get_efa_node_coor(orig_edge->get_node(0),CEMElem,elem) + get_efa_node_coor(orig_edge->get_node(1),CEMElem,elem);
        edge_center /= 2.0;
        crack_tip_origin = edge_center;
        crack_tip_direction = elem_center - edge_center;
        crack_tip_direction /= pow(crack_tip_direction.size_sq(),0.5);
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
    Point edge1(0.0,0.0,0.0);
    Point edge2(0.0,0.0,0.0);
    Point cut_edge_point(0.0,0.0,0.0);
    bool find_compatible_direction = false;
    unsigned int edge_id_keep = 0;
    Real distance_keep = 0.0;
    Point normal_keep(0.0,0.0,0.0);
    Real distance = 0.0;
    bool edge_cut = false;

    for (unsigned int i = 0; i < nsides; ++i)
    {
      if (!orig_edge->isPartialOverlap(*CEMElem->get_edge(i)))
      {
        edge_ends[0] = get_efa_node_coor(CEMElem->get_edge(i)->get_node(0),CEMElem,elem);
        edge_ends[1] = get_efa_node_coor(CEMElem->get_edge(i)->get_node(1),CEMElem,elem);
        if((init_crack_intersect_edge(crack_tip_origin,normal,edge_ends[0],edge_ends[1],distance) && (!CEMElem->is_edge_phantom(i))))
        {
          cut_edge_point = distance * edge_ends[1] + (1.0-distance) * edge_ends[0];
          distance_keep = distance;
          edge_id_keep = i;
          normal_keep = normal;
          edge_cut = true;
          break;
        }
      }
    }

    Point between_two_cuts = (cut_edge_point - crack_tip_origin);
    between_two_cuts /= pow(between_two_cuts.size_sq(),0.5);
    Real angle_between_two_cuts = between_two_cuts * crack_tip_direction;

    if (angle_between_two_cuts > std::cos(45.0/180.0*3.14159)) //original cut direction is good
      find_compatible_direction = true;

    if (!find_compatible_direction && edge_cut)
      correct_crack_extension_angle(elem, CEMElem, orig_edge, normal, crack_tip_origin, crack_tip_direction, distance_keep, edge_id_keep, normal_keep);

    if (edge_cut)
    {
      if (!_use_crack_growth_increment)
        _efa_mesh.addElemEdgeIntersection(elem->id(), edge_id_keep, distance_keep);
      else
      {
        MeshBase::element_iterator       elem_it  = _mesh->elements_begin();
        const MeshBase::element_iterator elem_end = _mesh->elements_end();

        Point growth_direction(0.0,0.0,0.0);

        growth_direction(0) = -normal_keep(1);
        growth_direction(1) = normal_keep(0);

        if (growth_direction * crack_tip_direction < 1.0e-10)
          growth_direction *= (-1.0);

        Real x0 = crack_tip_origin(0);
        Real y0 = crack_tip_origin(1);
        Real x1 = x0 + _crack_growth_increment * growth_direction(0);
        Real y1 = y0 + _crack_growth_increment * growth_direction(1);

        XFEM_geometric_cut * geometric_cut  = new XFEM_geometric_cut_2d( x0, y0, x1, y1, time*0.9, time*0.9);    

        for ( ; elem_it != elem_end; ++elem_it)
        {
          const Elem *elem = *elem_it;
          std::vector<cutEdge> elemCutEdges;
          EFAelement * EFAelem = _efa_mesh.getElemByID(elem->id());
          EFAelement2D * CEMElem = dynamic_cast<EFAelement2D*>(EFAelem);

          if (!CEMElem)
          {
            libMesh::err << " ERROR: EFAelem is not of EFAelement2D type" << std::endl;
            exit(1);
          }

          // continue if elem has been already cut twice - IMPORTANT
          if (CEMElem->is_final_cut())
            continue;

          // mark cut edges for the element and its fragment
          geometric_cut->cut_elem_by_geometry(elem, elemCutEdges, time);

          for (unsigned int i = 0; i < elemCutEdges.size(); ++i) // mark element edges
          {
            if (!CEMElem->is_edge_phantom(elemCutEdges[i].host_side_id)) // must not be phantom edge
            {
              _efa_mesh.addElemEdgeIntersection(elem->id(), elemCutEdges[i].host_side_id,
                                                elemCutEdges[i].distance);
              marked_edges = true;
            }
          }
        }
      }
    }
    // loop though framgent boundary edges to add possible second cut points
    // N.B. must do this after marking element edges
    if (CEMElem->num_frags() > 0 && !edge_cut)
    {
      for (unsigned int i = 0; i < CEMElem->get_fragment(0)->num_edges(); ++i)
      {
        if (!orig_edge->isPartialOverlap(*CEMElem->get_frag_edge(0,i)))
        {
          edge_ends[0] = get_efa_node_coor(CEMElem->get_frag_edge(0,i)->get_node(0),CEMElem,elem);
          edge_ends[1] = get_efa_node_coor(CEMElem->get_frag_edge(0,i)->get_node(1),CEMElem,elem);
          if (init_crack_intersect_edge(crack_tip_origin,normal,edge_ends[0],edge_ends[1],distance) &&
              (!CEMElem->get_fragment(0)->isSecondaryInteriorEdge(i)))
          {
            _efa_mesh.addFragEdgeIntersection(elem->id(), edge_id_keep, distance_keep);
            break;
          }
        }
      } // i
    }

    marked_edges = true;

  }// loop over all state_marked_elems

  return marked_edges;
}

bool
XFEM::mark_cut_faces_by_geometry(Real time)
{
  bool marked_faces = false;

  MeshBase::element_iterator       elem_it  = _mesh->elements_begin();
  const MeshBase::element_iterator elem_end = _mesh->elements_end();

  for ( ; elem_it != elem_end; ++elem_it)
  {
    const Elem *elem = *elem_it;
    std::vector<cutFace> elemCutFaces;
    std::vector<cutFace> fragCutFaces;
    std::vector<std::vector<Point> > frag_faces;
    EFAelement * EFAelem = _efa_mesh.getElemByID(elem->id());
    EFAelement3D * CEMElem = dynamic_cast<EFAelement3D*>(EFAelem);
    if (!CEMElem)
    {
      libMesh::err << " ERROR: EFAelem is not of EFAelement3D type" << std::endl;
      exit(1);
    }

    // continue if elem has been already cut twice - IMPORTANT
    if (CEMElem->is_final_cut())
      continue;

    // get fragment faces
    get_frag_faces(elem, CEMElem, frag_faces);

    // mark cut faces for the element and its fragment
    for (unsigned int i = 0; i < _geometric_cuts.size(); ++i)
    {
      _geometric_cuts[i]->cut_elem_by_geometry(elem, elemCutFaces, time);
      if (CEMElem->num_frags() > 0)
        _geometric_cuts[i]->cut_frag_by_geometry(frag_faces, fragCutFaces, time);
    }

    for (unsigned int i = 0; i < elemCutFaces.size(); ++i) // mark element faces
    {
      if (!CEMElem->is_face_phantom(elemCutFaces[i].face_id)) // must not be phantom face
      {
        _efa_mesh.addElemFaceIntersection(elem->id(), elemCutFaces[i].face_id,
                                          elemCutFaces[i].face_edge, elemCutFaces[i].position);
        marked_faces = true;
      }
    }

    for (unsigned int i = 0; i < fragCutFaces.size(); ++i) // MUST DO THIS AFTER MARKING ELEMENT EDGES
    {
      if (!CEMElem->get_fragment(0)->isThirdInteriorFace(fragCutFaces[i].face_id))
      {
        _efa_mesh.addFragFaceIntersection(elem->id(), fragCutFaces[i].face_id,
                                          fragCutFaces[i].face_edge, fragCutFaces[i].position);
        marked_faces = true;
      }
    }
  }

  return marked_faces;
}

bool
XFEM::mark_cut_faces_by_state()
{
  bool marked_faces = false;
  // TODO: need to finish this for 3D problems
  return marked_faces;
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
  _new_node_to_parent_node.clear();

  _efa_mesh.updatePhysicalLinksAndFragments();
  // DEBUG
//  _efa_mesh.printMesh();
//  std::cout<<"BWS before updateTopology"<<std::endl;

  _efa_mesh.updateTopology();
  // DEBUG
//  _efa_mesh.printMesh();
//  std::cout<<"BWS cut done"<<std::endl;

  //Add new nodes
  const std::vector<EFAnode*> NewNodes = _efa_mesh.getNewNodes();
  for (unsigned int i = 0; i < NewNodes.size(); ++i)
  {
    unsigned int new_node_id = NewNodes[i]->id();
    unsigned int parent_id = NewNodes[i]->parent()->id();

    Node *parent_node = _mesh->node_ptr(parent_id);
//    std::cout<<"BWS n_nodes: "<<_mesh->n_nodes()<<std::endl;
    Point *new_point = new Point(*parent_node);
    Node *new_node = Node::build(*new_point,_mesh->n_nodes()).release();
    new_node->processor_id() = parent_node->processor_id();
    _mesh->add_node(new_node);
//    new_node->set_old_dof_object();
    _new_node_to_parent_node[new_node->unique_id()] = parent_node->unique_id();

    new_node->set_n_systems(parent_node->n_systems());
    efa_id_to_new_node.insert(std::make_pair(new_node_id,new_node));
    std::cout<<"XFEM added new node: "<<new_node->id()+1<<std::endl;
    mesh_changed = true;
    if (_mesh2)
    {
      const Node *parent_node2 = _mesh2->node_ptr(parent_id);

      Point *new_point2 = new Point(*parent_node2);
      Node *new_node2 = Node::build(*new_point2,_mesh2->n_nodes()).release();
      new_node2->processor_id() = parent_node2->processor_id();
      _mesh2->add_node(new_node2);
//      new_node2->set_old_dof_object();

      new_node2->set_n_systems(parent_node2->n_systems());
      efa_id_to_new_node2.insert(std::make_pair(new_node_id,new_node2));
//      std::cout<<"XFEM2 added new node: "<<new_node2->id()+1<<std::endl;
    }
  }

//  std::cout<<"BWS _new_node_to_parent_node:"<<std::endl;
//  for (std::map<Node*, Node*>::iterator nit = _new_node_to_parent_node.begin();
//       nit != _new_node_to_parent_node.end(); ++nit)
//  {
//    Node * new_node = nit->first;
//    Node * parent_node = nit->second;
//    std::cout<<"BWS new node : "<<new_node->id() << " parent node: "<<parent_node->id()<<std::endl;
//  }


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
    if (parent_elem->processor_id() == _mesh->processor_id())
      _material_data[0]->copy(*libmesh_elem, *parent_elem, 0);

    // The crack tip origin map is stored before cut, thus the elem should be updated with new element.
    std::map<const Elem*, std::vector<Point> >::iterator mit = _elem_crack_origin_direction_map.find(parent_elem);
    if (mit != _elem_crack_origin_direction_map.end())
    { 
      std::vector<Point> crack_data = _elem_crack_origin_direction_map[parent_elem];
      _elem_crack_origin_direction_map.erase(mit);
      _elem_crack_origin_direction_map[libmesh_elem] = crack_data;
    }

    std::cout<<"XFEM added elem "<<libmesh_elem->id()+1<<std::endl;

    XFEMCutElem * xfce = NULL;
    if (_mesh->mesh_dimension() == 2)
    {
      EFAelement2D* new_efa_elem2d = dynamic_cast<EFAelement2D*>(NewElements[i]);
      if (!new_efa_elem2d)
      {
        libMesh::err << " ERROR: dynamic_cast to new_efa_elem2d fails" <<std::endl;
        exit(1);
      }
      xfce = new XFEMCutElem2D(libmesh_elem, new_efa_elem2d, _material_data[0]->nQPoints());
    }
    else if (_mesh->mesh_dimension() == 3)
    {
      EFAelement3D* new_efa_elem3d = dynamic_cast<EFAelement3D*>(NewElements[i]);
      if (!new_efa_elem3d)
      {
        libMesh::err << " ERROR: dynamic_cast to new_efa_elem3d fails" <<std::endl;
        exit(1);
      }
      xfce = new XFEMCutElem3D(libmesh_elem, new_efa_elem3d, _material_data[0]->nQPoints());
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
  } // i

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
  return mesh_changed;
}

Point
XFEM::get_efa_node_coor(EFAnode* CEMnode, EFAelement* CEMElem,
                        const Elem *elem, MeshBase* displaced_mesh) const
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
    XFEMCutElem *xfce = it->second;
    const EFAelement* EFAelem = xfce->get_efa_elem();
    if (EFAelem->is_partial()){ // exclude the full crack tip elements
      xfce->calc_physical_volfrac();
      phys_volfrac = xfce->get_physical_volfrac();
    }
  }

  return phys_volfrac;
}

Real
XFEM::get_elem_new_weights(const Elem* elem, unsigned int i_qp, std::vector<Point> &g_points, std::vector<Real> &g_weights) const // ZZY
{
  Real qp_weight = 1.0;
  std::map<const Elem*, XFEMCutElem*>::const_iterator it;
  it = _cut_elem_map.find(elem);
  if (it != _cut_elem_map.end())
  {
    XFEMCutElem *xfce = it->second;
    xfce->set_gauss_points_and_weights(g_points,g_weights);
    xfce->calc_mf_weights();
    qp_weight = xfce->get_mf_weights(i_qp);
  }
  return qp_weight;
}

Real
XFEM::flag_qp_inside(const Elem* elem, const Point & p) const
{
  // get the flag indicating if a QP is inside the physical domain of a partial element
  Real flag = 1.0; // default value - qp inside physical domain
  std::map<const Elem*, XFEMCutElem*>::const_iterator it;
  it = _cut_elem_map.find(elem);
  if (it != _cut_elem_map.end())
  {
    XFEMCutElem *xfce = it->second;
    unsigned int n_cut_planes = xfce->num_cut_planes();
    for (unsigned int plane_id = 0; plane_id < n_cut_planes; ++plane_id)
    {
      Point origin = xfce->get_origin(plane_id);
      Point normal = xfce->get_normal(plane_id);
      Point origin2qp = p - origin;
      normalize(origin2qp);
      if (origin2qp*normal > 0.0)
      {
        flag = 0.0; // QP outside pysical domain
        break;
      }
    } // plane_id
  }
  return flag;
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

void
XFEM::get_frag_faces(const Elem* elem, std::vector<std::vector<Point> > &frag_faces, bool displaced_mesh) const
{
  std::map<const Elem*, XFEMCutElem*>::const_iterator it;
  it = _cut_elem_map.find(elem);
  if (it != _cut_elem_map.end())
  {
    const XFEMCutElem *xfce = it->second;
    if (displaced_mesh)
      xfce->get_frag_faces(frag_faces, _mesh2);
    else
      xfce->get_frag_faces(frag_faces);
  }
}

void
XFEM::get_frag_edges(const Elem* elem, EFAelement2D* CEMElem, std::vector<std::vector<Point> > &frag_edges) const
{
  // N.B. CEMElem here has global EFAnode
  frag_edges.clear();
  if (CEMElem->num_frags() > 0)
  {
    if (CEMElem->num_frags() > 1)
      mooseError("element " << elem->id() << " has more than one fragments at this point");
    for (unsigned int i = 0; i < CEMElem->get_fragment(0)->num_edges(); ++i)
    {
      std::vector<Point> p_line(2,Point(0.0,0.0,0.0));
      p_line[0] = get_efa_node_coor(CEMElem->get_frag_edge(0,i)->get_node(0), CEMElem, elem);
      p_line[1] = get_efa_node_coor(CEMElem->get_frag_edge(0,i)->get_node(1), CEMElem, elem);
      frag_edges.push_back(p_line);
    } // i
  }
}

void
XFEM::get_frag_faces(const Elem* elem, EFAelement3D* CEMElem, std::vector<std::vector<Point> > &frag_faces) const
{
  // N.B. CEMElem here has global EFAnode
  frag_faces.clear();
  if (CEMElem->num_frags() > 0)
  {
    if (CEMElem->num_frags() > 1)
      mooseError("element " << elem->id() << " has more than one fragments at this point");
    for (unsigned int i = 0; i < CEMElem->get_fragment(0)->num_faces(); ++i)
    {
      unsigned int num_face_nodes = CEMElem->get_frag_face(0,i)->num_nodes();
      std::vector<Point> p_line(num_face_nodes, Point(0.0,0.0,0.0));
      for (unsigned int j = 0; j < num_face_nodes; ++j)
        p_line[j] = get_efa_node_coor(CEMElem->get_frag_face(0,i)->get_node(j), CEMElem, elem);
      frag_faces.push_back(p_line);
    } // i
  }
}

std::vector<Real>&
XFEM::get_xfem_cut_data()
{
  return _XFEM_cut_data;
}

void
XFEM::set_xfem_cut_data(std::vector<Real> &cut_data)
{
  _XFEM_cut_data = cut_data;
}

std::string &
XFEM::get_xfem_cut_type()
{
  return _XFEM_cut_type;
}

void
XFEM::set_xfem_cut_type(std::string & cut_type)
{
  _XFEM_cut_type = cut_type;
}

XFEM_QRULE &
XFEM::get_xfem_qrule()
{
  return _XFEM_qrule;
}

void
XFEM::set_xfem_qrule(std::string & xfem_qrule)
{
  if (xfem_qrule == "volfrac")
    _XFEM_qrule = VOLFRAC;
  else if (xfem_qrule == "moment_fitting")
    _XFEM_qrule = MOMENT_FITTING;
  else if (xfem_qrule == "direct")
    _XFEM_qrule = DIRECT;
}

void
XFEM::set_crack_growth_method(bool use_crack_growth_increment, Real crack_growth_increment)
{
  _use_crack_growth_increment = use_crack_growth_increment;
  _crack_growth_increment = crack_growth_increment;
}
