/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEM.h"

#include "AuxiliarySystem.h"
#include "NonlinearSystem.h"

#include "libmesh/mesh_communication.h"

#include "XFEMGeometricCut.h"
#include "XFEMGeometricCut2D.h"
#include "XFEMCutElem2D.h"
#include "XFEMCutElem3D.h"
#include "XFEMFuncs.h"
#include "EFANode.h"
#include "EFAEdge.h"
#include "EFAFace.h"
#include "EFAFragment2D.h"
#include "EFAFragment3D.h"
#include "EFAFuncs.h"

XFEM::XFEM (MooseApp & app) :
    XFEMInterface(app),
    _efa_mesh(Moose::out)
{
#ifndef LIBMESH_ENABLE_UNIQUE_ID
  mooseError("MOOSE requires unique ids to be enabled in libmesh (configure with --enable-unique-id) to use XFEM!");
#endif
}

XFEM::~XFEM ()
{
  for (unsigned int i=0; i<_geometric_cuts.size(); ++i)
    delete _geometric_cuts[i];

  for (std::map<unique_id_type, XFEMCutElem*>::iterator cemit = _cut_elem_map.begin();
       cemit != _cut_elem_map.end(); ++cemit)
    delete cemit->second;
}

void
XFEM::addGeometricCut(XFEMGeometricCut* geometric_cut)
{
  _geometric_cuts.push_back(geometric_cut);
}

void
XFEM::getCrackTipOrigin(std::map<unsigned int, const Elem* > & elem_id_crack_tip, std::vector<Point> & crack_front_points)
{
  elem_id_crack_tip.clear();
  crack_front_points.clear();
  crack_front_points.resize(_elem_crack_origin_direction_map.size());

  unsigned int crack_tip_index = 0;
  // This map is used to sort the order in _elem_crack_origin_direction_map such that every process has same order
  std::map<unsigned int, const Elem*> elem_id_map;

  int m = -1;
  for (std::map<const Elem*, std::vector<Point> >::iterator mit1 = _elem_crack_origin_direction_map.begin();
       mit1 != _elem_crack_origin_direction_map.end(); ++mit1)
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


  for (std::map<unsigned int, const Elem*> ::iterator mit1 = elem_id_map.begin();
       mit1 != elem_id_map.end(); mit1++)
  {
    const Elem* elem = mit1->second;
    std::map<const Elem*, std::vector<Point> >::iterator mit2 = _elem_crack_origin_direction_map.find(elem);
    if (mit2 != _elem_crack_origin_direction_map.end())
    {
      elem_id_crack_tip[crack_tip_index] = mit2->first;
      crack_front_points[crack_tip_index] = (mit2->second)[0]; // [0] stores origin coordinates and [1] stores direction
      crack_tip_index++;
    }
  }
}

void
XFEM::addStateMarkedElem(unsigned int elem_id, RealVectorValue & normal)
{
  Elem *elem = _mesh->elem(elem_id);
  std::map<const Elem*, RealVectorValue>::iterator mit;
  mit = _state_marked_elems.find(elem);
  if (mit != _state_marked_elems.end())
    mooseError(" ERROR: element "<<elem->id()<<" already marked for crack growth.");
  _state_marked_elems[elem] = normal;
}

void
XFEM::addStateMarkedElem(unsigned int elem_id, RealVectorValue & normal, unsigned int marked_side)
{
  addStateMarkedElem(elem_id, normal);
  Elem *elem = _mesh->elem(elem_id);
  std::map<const Elem*, unsigned int>::iterator mit;
  mit = _state_marked_elem_sides.find(elem);
  if (mit != _state_marked_elem_sides.end())
  {
    mooseError(" ERROR: side of element "<<elem->id()<<" already marked for crack initiation.");
    exit(1);
  }
  _state_marked_elem_sides[elem] = marked_side;
}

void
XFEM::addStateMarkedFrag(unsigned int elem_id, RealVectorValue & normal)
{
  addStateMarkedElem(elem_id, normal);
  Elem *elem = _mesh->elem(elem_id);
  std::set<const Elem*>::iterator mit;
  mit = _state_marked_frags.find(elem);
  if (mit != _state_marked_frags.end())
  {
    mooseError(" ERROR: element "<<elem->id()<<" already marked for fragment-secondary crack initiation.");
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
XFEM::storeCrackTipOriginAndDirection()
{
  _elem_crack_origin_direction_map.clear();
  std::set<EFAElement*> CrackTipElements = _efa_mesh.getCrackTipElements();
  std::set<EFAElement*>::iterator sit;
  for (sit = CrackTipElements.begin(); sit != CrackTipElements.end(); ++sit)
  {
    if (_mesh->mesh_dimension() == 2){
      EFAElement2D * CEMElem = dynamic_cast<EFAElement2D*>(*sit);
      EFANode *tip_node = CEMElem->getTipEmbeddedNode();
      unsigned int cts_id = CEMElem->getCrackTipSplitElementID();

      Point origin(0,0,0);
      Point direction(0,0,0);

      std::map<unique_id_type, XFEMCutElem*>::const_iterator it;
      it = _cut_elem_map.find(_mesh->elem(cts_id)->unique_id());
      if (it != _cut_elem_map.end())
      {
        const XFEMCutElem *xfce = it->second;
        const EFAElement* EFAelem = xfce->getEFAElement();
        if (EFAelem->isPartial()) // exclude the full crack tip elements
        {
          xfce->getCrackTipOriginAndDirection(tip_node->id(),origin,direction);
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
  bool mesh_changed = false;

  buildEFAMesh();

  storeCrackTipOriginAndDirection();

  if (markCuts(time))
    mesh_changed = cutMeshWithEFA();

  if (mesh_changed)
  {
    buildEFAMesh();
    storeCrackTipOriginAndDirection();
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

void XFEM::initSolution(NonlinearSystem & nl, AuxiliarySystem & /*aux*/)
{
  const std::vector<MooseVariable *> & nl_vars = nl.getVariables(0); //TODO pass in real thread id?

  nl.serializeSolution();
//  NumericVector<Number> & c_solution = *nl.sys().solution;
  NumericVector<Number> & current_solution = *nl.sys().current_local_solution;
  NumericVector<Number> & old_solution = *nl.sys().old_local_solution;

  for (std::map<unique_id_type, unique_id_type>::iterator nit = _new_node_to_parent_node.begin();
       nit != _new_node_to_parent_node.end(); ++nit)
  {
    for (unsigned int ivar=0; ivar<nl_vars.size(); ++ivar)
    {
      Node* new_node = getNodeFromUniqueID(nit->first);
      Node* parent_node = getNodeFromUniqueID(nit->second);
      Point new_point(*new_node);
      Point parent_point(*parent_node);
      if (new_point != parent_point)
        mooseError("Points don't match");
      unsigned int new_node_dof = new_node->dof_number(nl.number(), nl_vars[ivar]->number(),0);
      unsigned int parent_node_dof = parent_node->dof_number(nl.number(), nl_vars[ivar]->number(),0);
      if (parent_node->processor_id() == _mesh->processor_id())
      {
        current_solution.set(new_node_dof, current_solution(parent_node_dof));
        old_solution.set(new_node_dof, old_solution(parent_node_dof));
      }
    }
  }

  current_solution.close();
  old_solution.close();
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

void XFEM::buildEFAMesh()
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
      mooseError ("XFEM only works for 2D and 3D");
  }

  //Restore fragment information for elements that have been previously cut
  for ( elem_it = _mesh->elements_begin(); elem_it != elem_end; ++elem_it)
  {
    Elem *elem = *elem_it;
    std::map<unique_id_type, XFEMCutElem*>::iterator cemit = _cut_elem_map.find(elem->unique_id());
    if (cemit != _cut_elem_map.end())
    {
      XFEMCutElem *xfce = cemit->second;
      EFAElement * CEMElem = _efa_mesh.getElemByID(elem->id());
      _efa_mesh.restoreFragmentInfo(CEMElem, xfce->getEFAElement());
    }
  }

  //Must update edge neighbors before restore edge intersections. Otherwise, when we
  //add edge intersections, we do not have neighbor information to use.
  //Correction: no need to use neighbor info now
  _efa_mesh.updateEdgeNeighbors();
  _efa_mesh.initCrackTipTopology();
}

bool
XFEM::markCuts(Real time)
{
  bool marked_sides = false;
  if (_mesh->mesh_dimension() == 2)
  {
    marked_sides = markCutEdgesByGeometry(time);
    marked_sides |= markCutEdgesByState(time);
  }
  else if (_mesh->mesh_dimension() == 3)
  {
    marked_sides = markCutFacesByGeometry(time);
    marked_sides |= markCutFacesByState();
  }
  return marked_sides;
}

bool
XFEM::markCutEdgesByGeometry(Real time)
{
  bool marked_edges = false;

  std::vector<XFEMGeometricCut *> active_geometric_cuts;
  for (unsigned int i = 0; i < _geometric_cuts.size(); ++i)
    if (_geometric_cuts[i]->active(time))
      active_geometric_cuts.push_back(_geometric_cuts[i]);

  if (active_geometric_cuts.size() > 0)
  {
    for (MeshBase::element_iterator elem_it = _mesh->elements_begin();
         elem_it != _mesh->elements_end(); ++elem_it)
    {
      const Elem *elem = *elem_it;
      std::vector<CutEdge> elem_cut_edges;
      std::vector<CutEdge> frag_cut_edges;
      std::vector<std::vector<Point> > frag_edges;
      EFAElement * EFAelem = _efa_mesh.getElemByID(elem->id());
      EFAElement2D * CEMElem = dynamic_cast<EFAElement2D*>(EFAelem);

      if (!CEMElem)
        mooseError("EFAelem is not of EFAelement2D type");

      // continue if elem has been already cut twice - IMPORTANT
      if (CEMElem->isFinalCut())
        continue;

      // get fragment edges
      getFragmentEdges(elem, CEMElem, frag_edges);

      // mark cut edges for the element and its fragment
      for (unsigned int i = 0; i < active_geometric_cuts.size(); ++i)
      {
        active_geometric_cuts[i]->cutElementByGeometry(elem, elem_cut_edges, time);
        if (CEMElem->numFragments() > 0)
          active_geometric_cuts[i]->cutFragmentByGeometry(frag_edges, frag_cut_edges, time);
      }

      for (unsigned int i = 0; i < elem_cut_edges.size(); ++i) // mark element edges
      {
        if (!CEMElem->isEdgePhantom(elem_cut_edges[i].host_side_id)) // must not be phantom edge
        {
          _efa_mesh.addElemEdgeIntersection(elem->id(), elem_cut_edges[i].host_side_id,
                                            elem_cut_edges[i].distance);
          marked_edges = true;
        }
      }

      for (unsigned int i = 0; i < frag_cut_edges.size(); ++i) // MUST DO THIS AFTER MARKING ELEMENT EDGES
      {
        if (!CEMElem->getFragment(0)->isSecondaryInteriorEdge(frag_cut_edges[i].host_side_id))
        {
          _efa_mesh.addFragEdgeIntersection(elem->id(), frag_cut_edges[i].host_side_id,
                                            frag_cut_edges[i].distance);
          marked_edges = true;
        }
      }
    }
  }

  return marked_edges;
}

void
XFEM::correctCrackExtensionDirection(const Elem * elem,
                                     EFAElement2D * CEMElem,
                                     EFAEdge * orig_edge,
                                     Point normal,
                                     Point crack_tip_origin,
                                     Point crack_tip_direction,
                                     Real & distance_keep,
                                     unsigned int & edge_id_keep,
                                     Point & normal_keep)
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
  unsigned int nsides = CEMElem->numEdges();

  for (unsigned int i = 0; i < nsides; ++i)
  {
    if (!orig_edge->isPartialOverlap(*CEMElem->getEdge(i)))
    {
      edge_ends[0] = getEFANodeCoords(CEMElem->getEdge(i)->getNode(0),CEMElem,elem);
      edge_ends[1] = getEFANodeCoords(CEMElem->getEdge(i)->getNode(1),CEMElem,elem);

      edge1_to_tip = (edge_ends[0]*0.95 + edge_ends[1]*0.05) - crack_tip_origin;
      edge2_to_tip = (edge_ends[0]*0.05 + edge_ends[1]*0.95) - crack_tip_origin;

      edge1_to_tip /= pow(edge1_to_tip.norm_sq(),0.5);
      edge2_to_tip /= pow(edge2_to_tip.norm_sq(),0.5);

      edge1_to_tip_normal(0) = -edge1_to_tip(1);
      edge1_to_tip_normal(1) = edge1_to_tip(0);

      edge2_to_tip_normal(0) = -edge2_to_tip(1);
      edge2_to_tip_normal(1) = edge2_to_tip(0);

      Real angle_edge1_normal = edge1_to_tip_normal * normal;
      Real angle_edge2_normal = edge2_to_tip_normal * normal;

      if (std::abs(angle_edge1_normal) > std::abs(angle_min) && (edge1_to_tip*crack_tip_direction) > std::cos(45.0/180.0*3.14159))
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

      if (initCutIntersectionEdge(crack_tip_origin,left_angle_normal,edge_ends[0],edge_ends[1],distance) &&  (!CEMElem->isEdgePhantom(i)) )
      {
        if (std::abs(left_angle_normal*normal) > std::abs(angle_min) && (edge1_to_tip*crack_tip_direction) > std::cos(45.0/180.0*3.14159))
        {
          edge_id_keep = i;
          distance_keep = distance;
          normal_keep = left_angle_normal;
          angle_min = left_angle_normal*normal;
        }
      }
      else if (initCutIntersectionEdge(crack_tip_origin,right_angle_normal,edge_ends[0],edge_ends[1],distance) && (!CEMElem->isEdgePhantom(i)))
      {
        if (std::abs(right_angle_normal*normal) > std::abs(angle_min) && (edge2_to_tip*crack_tip_direction) > std::cos(45.0/180.0*3.14159))
        {
          edge_id_keep = i;
          distance_keep = distance;
          normal_keep = right_angle_normal;
          angle_min = right_angle_normal*normal;
        }
      }
      else if (initCutIntersectionEdge(crack_tip_origin,crack_direction_normal,edge_ends[0],edge_ends[1],distance) && (!CEMElem->isEdgePhantom(i)))
      {
        if (std::abs(crack_direction_normal*normal) > std::abs(angle_min) && (crack_tip_direction*crack_tip_direction) > std::cos(45.0/180.0*3.14159))
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
  if ((distance_keep-0.05) < 0.0)
  {
    distance_keep = 0.05;
  }
  else if ((distance_keep - 0.95) > 0.0)
  {
    distance_keep = 0.95;
  }
}


bool
XFEM::markCutEdgesByState(Real time)
{
  bool marked_edges = false;
  for (std::map<const Elem*, RealVectorValue>::iterator pmeit = _state_marked_elems.begin();
       pmeit != _state_marked_elems.end(); ++pmeit)
  {
    const Elem *elem = pmeit->first;
    RealVectorValue normal = pmeit->second;
    EFAElement * EFAelem = _efa_mesh.getElemByID(elem->id());
    EFAElement2D * CEMElem = dynamic_cast<EFAElement2D*>(EFAelem);

    Real volfrac_elem = getPhysicalVolumeFraction(elem);
    if (volfrac_elem < 0.25)
      continue;

    if (!CEMElem)
      mooseError("EFAelem is not of EFAelement2D type");

    // continue if elem is already cut twice - IMPORTANT
    if (CEMElem->isFinalCut())
      continue;

    // find the first cut edge
    unsigned int nsides = CEMElem->numEdges();
    unsigned int orig_cut_side_id = 999999;
    Real orig_cut_distance = -1.0;
    EFANode * orig_node = NULL;
    EFAEdge * orig_edge = NULL;

    //crack tip origin coordinates and direction
    Point crack_tip_origin(0,0,0);
    Point crack_tip_direction(0,0,0);

    if (isElemAtCrackTip(elem)) // crack tip element's crack intiation
    {
      orig_cut_side_id = CEMElem->getTipEdgeID();
      if (orig_cut_side_id < nsides) // valid crack-tip edge found
      {
        orig_edge = CEMElem->getEdge(orig_cut_side_id);
        orig_node = CEMElem->getTipEmbeddedNode();
        unsigned int emb_id = orig_edge->getEmbeddedNodeIndex(orig_node);
        orig_cut_distance = orig_edge->getIntersection(emb_id,orig_edge->getNode(0));
      }
      else
        mooseError("element "<<elem->id()<<" has no valid crack-tip edge");

      //obtain the crack tip origin coordinates and direction.
      std::map<const Elem*, std::vector<Point> >::iterator ecodm = _elem_crack_origin_direction_map.find(elem);
      if (ecodm != _elem_crack_origin_direction_map.end()){
        crack_tip_origin = (ecodm->second)[0];
        crack_tip_direction = (ecodm->second)[1];
      }
      else
        mooseError("element " << elem->id() << " cannot find its crack tip origin and direction.");
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
        if (!CEMElem->isEdgePhantom(orig_cut_side_id) &&
            !CEMElem->getEdge(orig_cut_side_id)->hasIntersection())
        {
          orig_cut_distance = 0.5;
          _efa_mesh.addElemEdgeIntersection(elem->id(), orig_cut_side_id, orig_cut_distance);
          orig_edge = CEMElem->getEdge(orig_cut_side_id);
          orig_node = orig_edge->getEmbeddedNode(0);
          //get a virtual crack tip direction
          Point elem_center(0.0,0.0,0.0);
          Point edge_center;
          for (unsigned int i = 0; i < nsides; ++i){
            elem_center += getEFANodeCoords(CEMElem->getEdge(i)->getNode(0),CEMElem,elem);
            elem_center += getEFANodeCoords(CEMElem->getEdge(i)->getNode(1),CEMElem,elem);
          }
          elem_center /= nsides*2.0;
          edge_center = getEFANodeCoords(orig_edge->getNode(0),CEMElem,elem) + getEFANodeCoords(orig_edge->getNode(1),CEMElem,elem);
          edge_center /= 2.0;
          crack_tip_origin = edge_center;
          crack_tip_direction = elem_center - edge_center;
          crack_tip_direction /= pow(crack_tip_direction.norm_sq(),0.5);
        }
        else
          continue; // skip this elem if specified boundary edge is phantom
      }
      else if (mit2 != _state_marked_frags.end()) // cut-surface secondary crack initiation
      {
        if (CEMElem->numFragments() != 1)
          mooseError("element "<<elem->id()<<" flagged for a secondary crack, but has "
                     <<CEMElem->numFragments()<<" fragments");
        std::vector<unsigned int> interior_edge_id = CEMElem->getFragment(0)->getInteriorEdgeID();
        if (interior_edge_id.size() == 1)
          orig_cut_side_id = interior_edge_id[0];
        else
          continue; // skip this elem if more than one interior edges found (i.e. elem's been cut twice)
        orig_cut_distance = 0.5;
        _efa_mesh.addFragEdgeIntersection(elem->id(), orig_cut_side_id, orig_cut_distance);
        orig_edge = CEMElem->getFragmentEdge(0,orig_cut_side_id);
        orig_node = orig_edge->getEmbeddedNode(0); // must be an interior embedded node
        Point elem_center(0.0,0.0,0.0);
        Point edge_center;
        unsigned int nsides_frag = CEMElem->getFragment(0)->numEdges();
        for (unsigned int i = 0; i < nsides_frag; ++i){
          elem_center += getEFANodeCoords(CEMElem->getFragmentEdge(0,i)->getNode(0),CEMElem,elem);
          elem_center += getEFANodeCoords(CEMElem->getFragmentEdge(0,i)->getNode(1),CEMElem,elem);
        }
        elem_center /= nsides_frag*2.0;
        edge_center = getEFANodeCoords(orig_edge->getNode(0),CEMElem,elem) + getEFANodeCoords(orig_edge->getNode(1),CEMElem,elem);
        edge_center /= 2.0;
        crack_tip_origin = edge_center;
        crack_tip_direction = elem_center - edge_center;
        crack_tip_direction /= pow(crack_tip_direction.norm_sq(),0.5);
      }
      else
        mooseError ("element " << elem->id() << " flagged for state-based growth, but has no edge intersections");
    }

    Point cut_origin(0.0,0.0,0.0);
    if (orig_node)
      cut_origin = getEFANodeCoords(orig_node, CEMElem, elem);// cutting plane origin's coords
    else
      mooseError("element "<<elem->id()<<" does not have valid orig_node");

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
      if (!orig_edge->isPartialOverlap(*CEMElem->getEdge(i)))
      {
        edge_ends[0] = getEFANodeCoords(CEMElem->getEdge(i)->getNode(0),CEMElem,elem);
        edge_ends[1] = getEFANodeCoords(CEMElem->getEdge(i)->getNode(1),CEMElem,elem);
        if ((initCutIntersectionEdge(crack_tip_origin,normal,edge_ends[0],edge_ends[1],distance) && (!CEMElem->isEdgePhantom(i))))
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
    between_two_cuts /= pow(between_two_cuts.norm_sq(),0.5);
    Real angle_between_two_cuts = between_two_cuts * crack_tip_direction;

    if (angle_between_two_cuts > std::cos(45.0/180.0*3.14159)) //original cut direction is good
      find_compatible_direction = true;

    if (!find_compatible_direction && edge_cut)
      correctCrackExtensionDirection(elem, CEMElem, orig_edge, normal, crack_tip_origin, crack_tip_direction, distance_keep, edge_id_keep, normal_keep);

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

        XFEMGeometricCut2D geometric_cut(x0, y0, x1, y1, time * 0.9, time * 0.9);

        for ( ; elem_it != elem_end; ++elem_it)
        {
          const Elem *elem = *elem_it;
          std::vector<CutEdge> elem_cut_edges;
          EFAElement * EFAelem = _efa_mesh.getElemByID(elem->id());
          EFAElement2D * CEMElem = dynamic_cast<EFAElement2D*>(EFAelem);

          if (!CEMElem)
            mooseError("EFAelem is not of EFAelement2D type");

          // continue if elem has been already cut twice - IMPORTANT
          if (CEMElem->isFinalCut())
            continue;

          // mark cut edges for the element and its fragment
          geometric_cut.cutElementByGeometry(elem, elem_cut_edges, time);

          for (unsigned int i = 0; i < elem_cut_edges.size(); ++i) // mark element edges
          {
            if (!CEMElem->isEdgePhantom(elem_cut_edges[i].host_side_id)) // must not be phantom edge
            {
              _efa_mesh.addElemEdgeIntersection(elem->id(), elem_cut_edges[i].host_side_id,
                                                elem_cut_edges[i].distance);
              marked_edges = true;
            }
          }
        }
      }
    }
    // loop though framgent boundary edges to add possible second cut points
    // N.B. must do this after marking element edges
    if (CEMElem->numFragments() > 0 && !edge_cut)
    {
      for (unsigned int i = 0; i < CEMElem->getFragment(0)->numEdges(); ++i)
      {
        if (!orig_edge->isPartialOverlap(*CEMElem->getFragmentEdge(0,i)))
        {
          edge_ends[0] = getEFANodeCoords(CEMElem->getFragmentEdge(0,i)->getNode(0),CEMElem,elem);
          edge_ends[1] = getEFANodeCoords(CEMElem->getFragmentEdge(0,i)->getNode(1),CEMElem,elem);
          if (initCutIntersectionEdge(crack_tip_origin,normal,edge_ends[0],edge_ends[1],distance) &&
              (!CEMElem->getFragment(0)->isSecondaryInteriorEdge(i)))
          {
            _efa_mesh.addFragEdgeIntersection(elem->id(), edge_id_keep, distance_keep);
            break;
          }
        }
      }
    }

    marked_edges = true;

  }// loop over all state_marked_elems

  return marked_edges;
}

bool
XFEM::markCutFacesByGeometry(Real time)
{
  bool marked_faces = false;

  MeshBase::element_iterator       elem_it  = _mesh->elements_begin();
  const MeshBase::element_iterator elem_end = _mesh->elements_end();

  std::vector<XFEMGeometricCut *> active_geometric_cuts;
  for (unsigned int i = 0; i < _geometric_cuts.size(); ++i)
    if (_geometric_cuts[i]->active(time))
      active_geometric_cuts.push_back(_geometric_cuts[i]);

  if (active_geometric_cuts.size() > 0)
  {
    for (MeshBase::element_iterator elem_it = _mesh->elements_begin();
         elem_it != _mesh->elements_end(); ++elem_it)
    {
      const Elem *elem = *elem_it;
      std::vector<CutFace> elem_cut_faces;
      std::vector<CutFace> frag_cut_faces;
      std::vector<std::vector<Point> > frag_faces;
      EFAElement * EFAelem = _efa_mesh.getElemByID(elem->id());
      EFAElement3D * CEMElem = dynamic_cast<EFAElement3D*>(EFAelem);
      if (!CEMElem)
        mooseError("EFAelem is not of EFAelement3D type");

      // continue if elem has been already cut twice - IMPORTANT
      if (CEMElem->isFinalCut())
        continue;

      // get fragment faces
      getFragmentFaces(elem, CEMElem, frag_faces);

      // mark cut faces for the element and its fragment
      for (unsigned int i = 0; i < active_geometric_cuts.size(); ++i)
      {
        active_geometric_cuts[i]->cutElementByGeometry(elem, elem_cut_faces, time);
        // TODO: This would be done for branching, which is not yet supported in 3D
        //      if (CEMElem->numFragments() > 0)
        //        active_geometric_cuts[i]->cutFragmentByGeometry(frag_faces, frag_cut_faces, time);
      }

      for (unsigned int i = 0; i < elem_cut_faces.size(); ++i) // mark element faces
      {
        if (!CEMElem->isFacePhantom(elem_cut_faces[i].face_id)) // must not be phantom face
        {
          _efa_mesh.addElemFaceIntersection(elem->id(), elem_cut_faces[i].face_id,
                                            elem_cut_faces[i].face_edge, elem_cut_faces[i].position);
          marked_faces = true;
        }
      }

      for (unsigned int i = 0; i < frag_cut_faces.size(); ++i) // MUST DO THIS AFTER MARKING ELEMENT EDGES
      {
        if (!CEMElem->getFragment(0)->isThirdInteriorFace(frag_cut_faces[i].face_id))
        {
          _efa_mesh.addFragFaceIntersection(elem->id(), frag_cut_faces[i].face_id,
                                            frag_cut_faces[i].face_edge, frag_cut_faces[i].position);
          marked_faces = true;
        }
      }
    }
  }

  return marked_faces;
}

bool
XFEM::markCutFacesByState()
{
  bool marked_faces = false;
  // TODO: need to finish this for 3D problems
  return marked_faces;
}


bool
XFEM::initCutIntersectionEdge(Point cut_origin,
                              RealVectorValue cut_normal,
                              Point & edge_p1,
                              Point & edge_p2,
                              Real & dist)
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
XFEM::cutMeshWithEFA()
{
  bool mesh_changed = false;

  std::map<unsigned int, Node*> efa_id_to_new_node;
  std::map<unsigned int, Node*> efa_id_to_new_node2;
  std::map<unsigned int, Elem*> efa_id_to_new_elem;
  _new_node_to_parent_node.clear();

  _efa_mesh.updatePhysicalLinksAndFragments();
  // DEBUG
//  _efa_mesh.printMesh();

  _efa_mesh.updateTopology();
  // DEBUG
//  _efa_mesh.printMesh();

  //Add new nodes
  const std::vector<EFANode*> NewNodes = _efa_mesh.getNewNodes();
  for (unsigned int i = 0; i < NewNodes.size(); ++i)
  {
    unsigned int new_node_id = NewNodes[i]->id();
    unsigned int parent_id = NewNodes[i]->parent()->id();

    Node *parent_node = _mesh->node_ptr(parent_id);
    Node *new_node = Node::build(*parent_node,_mesh->n_nodes()).release();
    new_node->processor_id() = parent_node->processor_id();
    _mesh->add_node(new_node);
    _new_node_to_parent_node[new_node->unique_id()] = parent_node->unique_id();

    new_node->set_n_systems(parent_node->n_systems());
    efa_id_to_new_node.insert(std::make_pair(new_node_id,new_node));
    _console << "XFEM added new node: " << new_node->id() << "\n";
    mesh_changed = true;
    if (_mesh2)
    {
      const Node *parent_node2 = _mesh2->node_ptr(parent_id);
      Node *new_node2 = Node::build(*parent_node2,_mesh2->n_nodes()).release();
      new_node2->processor_id() = parent_node2->processor_id();
      _mesh2->add_node(new_node2);

      new_node2->set_n_systems(parent_node2->n_systems());
      efa_id_to_new_node2.insert(std::make_pair(new_node_id,new_node2));
    }
  }

  //Add new elements
  const std::vector<EFAElement*> NewElements = _efa_mesh.getChildElements();

  std::map<unsigned int, std::vector<const Elem *> > temporary_parent_children_map;

  for (unsigned int i = 0; i < NewElements.size(); ++i)
  {
    unsigned int parent_id = NewElements[i]->getParent()->id();
    unsigned int efa_child_id = NewElements[i]->id();

    Elem *parent_elem = _mesh->elem(parent_id);
    Elem *libmesh_elem = Elem::build(parent_elem->type()).release();

    // parent has at least two children
    if ( NewElements[i]->getParent()->numChildren() > 1)
      temporary_parent_children_map[parent_id].push_back(libmesh_elem);

    Elem *parent_elem2 = NULL;
    Elem *libmesh_elem2 = NULL;
    if (_mesh2)
    {
      parent_elem2 = _mesh2->elem(parent_id);
      libmesh_elem2 = Elem::build(parent_elem2->type()).release();
    }

    for (unsigned int j = 0; j < NewElements[i]->numNodes(); ++j)
    {
      unsigned int node_id = NewElements[i]->getNode(j)->id();
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
      (*_material_data)[0]->copy(*libmesh_elem, *parent_elem, 0);

    // The crack tip origin map is stored before cut, thus the elem should be updated with new element.
    std::map<const Elem*, std::vector<Point> >::iterator mit = _elem_crack_origin_direction_map.find(parent_elem);
    if (mit != _elem_crack_origin_direction_map.end())
    {
      std::vector<Point> crack_data = _elem_crack_origin_direction_map[parent_elem];
      _elem_crack_origin_direction_map.erase(mit);
      _elem_crack_origin_direction_map[libmesh_elem] = crack_data;
    }

    _console << "XFEM added new element: " << libmesh_elem->id() << "\n";

    XFEMCutElem * xfce = NULL;
    if (_mesh->mesh_dimension() == 2)
    {
      EFAElement2D* new_efa_elem2d = dynamic_cast<EFAElement2D*>(NewElements[i]);
      if (!new_efa_elem2d)
        mooseError("EFAelem is not of EFAelement2D type");
      xfce = new XFEMCutElem2D(libmesh_elem, new_efa_elem2d, (*_material_data)[0]->nQPoints());
    }
    else if (_mesh->mesh_dimension() == 3)
    {
      EFAElement3D* new_efa_elem3d = dynamic_cast<EFAElement3D*>(NewElements[i]);
      if (!new_efa_elem3d)
        mooseError("EFAelem is not of EFAelement3D type");
      xfce = new XFEMCutElem3D(libmesh_elem, new_efa_elem3d, (*_material_data)[0]->nQPoints());
    }
    _cut_elem_map.insert(std::pair<unique_id_type,XFEMCutElem*>(libmesh_elem->unique_id(), xfce));
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
  const std::vector<EFAElement*> DeleteElements = _efa_mesh.getParentElements();
  for (unsigned int i = 0; i < DeleteElements.size(); ++i)
  {
    Elem *elem_to_delete = _mesh->elem(DeleteElements[i]->id());

    //delete the XFEMCutElem object for any elements that are to be deleted
    std::map<unique_id_type, XFEMCutElem*>::iterator cemit = _cut_elem_map.find(elem_to_delete->unique_id());
    if (cemit != _cut_elem_map.end())
    {
      delete cemit->second;
      _cut_elem_map.erase(cemit);
    }

    elem_to_delete->nullify_neighbors();
    _mesh->boundary_info->remove(elem_to_delete);
    unsigned int deleted_elem_id = elem_to_delete->id();
    _mesh->delete_elem(elem_to_delete);
    _console << "XFEM deleted element: " << deleted_elem_id << "\n";
    mesh_changed = true;

    if (_mesh2)
    {
      Elem *elem_to_delete2 = _mesh2->elem(DeleteElements[i]->id());
      elem_to_delete2->nullify_neighbors();
      _mesh2->boundary_info->remove(elem_to_delete2);
      _mesh2->delete_elem(elem_to_delete2);
    }
  }

  for (std::map<unsigned int, std::vector<const Elem *> > :: iterator it = temporary_parent_children_map.begin(); it != temporary_parent_children_map.end(); ++it)
  {
      _sibling_elems.push_back(std::make_pair((it->second[0]), (it->second)[1]));
  }
  
  //clear the temporary map
  temporary_parent_children_map.clear();

  //Store information about crack tip elements
  if (mesh_changed)
  {
    _crack_tip_elems.clear();
    const std::set<EFAElement*> CrackTipElements = _efa_mesh.getCrackTipElements();
    std::set<EFAElement*>::const_iterator sit;
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
  _console << std::flush;

  //store virtual nodes
  //store cut edge info
  return mesh_changed;
}

Point
XFEM::getEFANodeCoords(EFANode* CEMnode,
                       EFAElement* CEMElem,
                       const Elem *elem,
                       MeshBase* displaced_mesh) const
{
  Point node_coor(0.0,0.0,0.0);
  std::vector<EFANode*> master_nodes;
  std::vector<Point> master_points;
  std::vector<double> master_weights;

  CEMElem->getMasterInfo(CEMnode, master_nodes, master_weights);
  for (unsigned int i = 0; i < master_nodes.size(); ++i)
  {
    if (master_nodes[i]->category() == EFANode::N_CATEGORY_PERMANENT)
    {
      unsigned int local_node_id = CEMElem->getLocalNodeIndex(master_nodes[i]);
      Node* node = elem->get_node(local_node_id);
      if (displaced_mesh)
        node = displaced_mesh->node_ptr(node->id());
      Point node_p((*node)(0), (*node)(1), (*node)(2));
      master_points.push_back(node_p);
    }
    else
      mooseError ("master nodes must be permanent");
  }
  for (unsigned int i = 0; i < master_nodes.size(); ++i)
    node_coor += master_weights[i]*master_points[i];

  return node_coor;
}

Real
XFEM::getPhysicalVolumeFraction(const Elem* elem) const
{
  Real phys_volfrac = 1.0;
  std::map<unique_id_type, XFEMCutElem*>::const_iterator it;
  it = _cut_elem_map.find(elem->unique_id());
  if (it != _cut_elem_map.end())
  {
    XFEMCutElem *xfce = it->second;
    const EFAElement* EFAelem = xfce->getEFAElement();
    if (EFAelem->isPartial()){ // exclude the full crack tip elements
      xfce->computePhysicalVolumeFraction();
      phys_volfrac = xfce->getPhysicalVolumeFraction();
    }
  }

  return phys_volfrac;
}

Real
XFEM::getCutPlane(const Elem* elem,
                  const Xfem::XFEM_CUTPLANE_QUANTITY quantity,
                  unsigned int plane_id) const
{
  Real comp=0.0;
  Point planedata(0.0,0.0,0.0);
  std::map<unique_id_type, XFEMCutElem*>::const_iterator it;
  it = _cut_elem_map.find(elem->unique_id());
  if (it != _cut_elem_map.end())
  {
    const XFEMCutElem *xfce = it->second;
    const EFAElement* EFAelem = xfce->getEFAElement();
    if (EFAelem->isPartial()) // exclude the full crack tip elements
    {
      if ((unsigned int)quantity < 3)
      {
        unsigned int index = (unsigned int)quantity;
        planedata = xfce->getCutPlaneOrigin(plane_id,_mesh2);
        comp = planedata(index);
      }
      else if ((unsigned int)quantity < 6)
      {
        unsigned int index = (unsigned int)quantity - 3;
        planedata = xfce->getCutPlaneNormal(plane_id,_mesh2);
        comp = planedata(index);
      }
      else
        mooseError("In get_cut_plane index out of range");
    }
  }
  return comp;
}

bool
XFEM::isElemAtCrackTip(const Elem* elem) const
{
  return (_crack_tip_elems.find(elem) != _crack_tip_elems.end());
}

bool
XFEM::isElemCut(const Elem* elem, XFEMCutElem *&xfce) const
{
  xfce = NULL;
  bool is_cut = false;
  std::map<unique_id_type, XFEMCutElem*>::const_iterator it;
  it = _cut_elem_map.find(elem->unique_id());
  if (it != _cut_elem_map.end())
  {
    xfce = it->second;
    const EFAElement* EFAelem = xfce->getEFAElement();
    if (EFAelem->isPartial()) // exclude the full crack tip elements
      is_cut = true;
  }
  return is_cut;
}

bool
XFEM::isElemCut(const Elem* elem) const
{
  XFEMCutElem *xfce = NULL;
  return isElemCut(elem,xfce);
}

void
XFEM::getFragmentFaces(const Elem* elem, std::vector<std::vector<Point> > &frag_faces, bool displaced_mesh) const
{
  std::map<unique_id_type, XFEMCutElem*>::const_iterator it;
  it = _cut_elem_map.find(elem->unique_id());
  if (it != _cut_elem_map.end())
  {
    const XFEMCutElem *xfce = it->second;
    if (displaced_mesh)
      xfce->getFragmentFaces(frag_faces, _mesh2);
    else
      xfce->getFragmentFaces(frag_faces);
  }
}

void
XFEM::getFragmentEdges(const Elem* elem,
                       EFAElement2D* CEMElem,
                       std::vector<std::vector<Point> > &frag_edges) const
{
  // N.B. CEMElem here has global EFAnode
  frag_edges.clear();
  if (CEMElem->numFragments() > 0)
  {
    if (CEMElem->numFragments() > 1)
      mooseError("element " << elem->id() << " has more than one fragments at this point");
    for (unsigned int i = 0; i < CEMElem->getFragment(0)->numEdges(); ++i)
    {
      std::vector<Point> p_line(2,Point(0.0,0.0,0.0));
      p_line[0] = getEFANodeCoords(CEMElem->getFragmentEdge(0,i)->getNode(0), CEMElem, elem);
      p_line[1] = getEFANodeCoords(CEMElem->getFragmentEdge(0,i)->getNode(1), CEMElem, elem);
      frag_edges.push_back(p_line);
    }
  }
}

void
XFEM::getFragmentFaces(const Elem* elem,
                       EFAElement3D* CEMElem,
                       std::vector<std::vector<Point> > &frag_faces) const
{
  // N.B. CEMElem here has global EFAnode
  frag_faces.clear();
  if (CEMElem->numFragments() > 0)
  {
    if (CEMElem->numFragments() > 1)
      mooseError("element " << elem->id() << " has more than one fragments at this point");
    for (unsigned int i = 0; i < CEMElem->getFragment(0)->numFaces(); ++i)
    {
      unsigned int num_face_nodes = CEMElem->getFragmentFace(0,i)->numNodes();
      std::vector<Point> p_line(num_face_nodes, Point(0.0,0.0,0.0));
      for (unsigned int j = 0; j < num_face_nodes; ++j)
        p_line[j] = getEFANodeCoords(CEMElem->getFragmentFace(0,i)->getNode(j), CEMElem, elem);
      frag_faces.push_back(p_line);
    }
  }
}

std::vector<Real>&
XFEM::getXFEMCutData()
{
  return _XFEM_cut_data;
}

void
XFEM::setXFEMCutData(std::vector<Real> &cut_data)
{
  _XFEM_cut_data = cut_data;
}

std::string &
XFEM::getXFEMCutType()
{
  return _XFEM_cut_type;
}

void
XFEM::setXFEMCutType(std::string & cut_type)
{
  _XFEM_cut_type = cut_type;
}

Xfem::XFEM_QRULE &
XFEM::getXFEMQRule()
{
  return _XFEM_qrule;
}

void
XFEM::setXFEMQRule(std::string & xfem_qrule)
{
  if (xfem_qrule == "volfrac")
    _XFEM_qrule = Xfem::VOLFRAC;
  else if (xfem_qrule == "moment_fitting")
    _XFEM_qrule = Xfem::MOMENT_FITTING;
  else if (xfem_qrule == "direct")
    _XFEM_qrule = Xfem::DIRECT;
}

void
XFEM::setCrackGrowthMethod(bool use_crack_growth_increment, Real crack_growth_increment)
{
  _use_crack_growth_increment = use_crack_growth_increment;
  _crack_growth_increment = crack_growth_increment;
}

bool
XFEM::getXFEMWeights(MooseArray<Real> &weights, const Elem * elem, QBase * qrule, const MooseArray<Point> & q_points)
{
  bool have_weights = false;
  XFEMCutElem *xfce = NULL;
  if (isElemCut(elem,xfce))
  {
    mooseAssert(xfce != NULL,"Must have valid XFEMCutElem object here");
    xfce->getWeightMultipliers(weights, qrule, getXFEMQRule(), q_points);
    have_weights = true;
  }
  return have_weights;
}

void 
XFEM::getXFEMIntersectionInfo(const Elem* elem, unsigned int plane_id, Point & normal, std::vector<Point> & intersectionPoints, bool displaced_mesh) const
{
  std::map<unique_id_type, XFEMCutElem*>::const_iterator it;
  it = _cut_elem_map.find(elem->unique_id());
  if (it != _cut_elem_map.end())
  {
    const XFEMCutElem *xfce = it->second;
    if (displaced_mesh)
      xfce->getIntersectionInfo(plane_id, normal, intersectionPoints, _mesh2);
    else
      xfce->getIntersectionInfo(plane_id, normal, intersectionPoints);
  }
}

void
XFEM::getXFEMqRuleOnLine(std::vector<Point> & intersection_points, std::vector<Point> & quad_pts, std::vector<Real> & quad_wts) const
{
  Point p1 = intersection_points[0];
  Point p2 = intersection_points[1];

  //number of quadrature points
  unsigned int num_qpoints = 2;

  //quadrature coordinates
  Real xi0 = -std::sqrt(1.0/3.0);
  Real xi1 =  std::sqrt(1.0/3.0);

  quad_wts.resize(num_qpoints);
  quad_pts.resize(num_qpoints);

  Real integ_jacobian =  pow((p1 -  p2).size_sq(), 0.5) * 0.5;

  quad_wts[0] = 1.0 * integ_jacobian;
  quad_wts[1] = 1.0 * integ_jacobian;

  quad_pts[0] = (1.0 - xi0) / 2.0 * p1 + (1.0 + xi0) / 2.0 * p2;
  quad_pts[1] = (1.0 - xi1) / 2.0 * p1 + (1.0 + xi1) / 2.0 * p2;
}

void
XFEM::getXFEMqRuleOnSurface(std::vector<Point> & intersection_points, std::vector<Point> & quad_pts, std::vector<Real> & quad_wts) const
{
  unsigned nnd_pe = intersection_points.size();
  Point xcrd(0.0, 0.0, 0.0);
  for (unsigned int i = 0; i < intersection_points.size(); ++i)
    xcrd += intersection_points[i];
  xcrd *= (1.0/intersection_points.size());

  quad_pts.resize(nnd_pe);
  quad_wts.resize(nnd_pe);

  Real jac = 0.0;

  for (unsigned int j = 0; j < nnd_pe; ++j) // loop all sub-trigs
  {
    std::vector<std::vector<Real> > shape(3, std::vector<Real>(3,0.0));
    std::vector<Point> subtrig_points(3, Point(0.0,0.0,0.0)); // sub-trig nodal coords

    int jplus1(j < nnd_pe-1 ? j+1 : 0);
    subtrig_points[0] = xcrd;
    subtrig_points[1] = intersection_points[j];
    subtrig_points[2] = intersection_points[jplus1];

    std::vector<std::vector<Real> > sg2;
    Xfem::stdQuadr2D(3, 1, sg2); // get sg2
    for (unsigned int l = 0; l < sg2.size(); ++l) // loop all int pts on a sub-trig
    {
      Xfem::shapeFunc2D(3, sg2[l], subtrig_points, shape, jac, true); // Get shape
      std::vector<Real> tsg_line(3,0.0);
      for (unsigned int k = 0; k < 3; ++k) // loop sub-trig nodes
      {
        tsg_line[0] += shape[k][2] * subtrig_points[k](0);
        tsg_line[1] += shape[k][2] * subtrig_points[k](1);
        tsg_line[2] += shape[k][2] * subtrig_points[k](2);
      }
      quad_pts[j + l] = Point(tsg_line[0], tsg_line[1], tsg_line[2]);
      quad_wts[j + l] = sg2[l][3] * jac;
    }
  }
}
