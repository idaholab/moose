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

XFEMCutElemNode::XFEMCutElemNode(CutElemMesh::N_CATEGORY category,
                                 unsigned int index,
                                 std::vector<Node*> master_nodes,
                                 std::vector<Real> weights):
  _category(category),
  _index(index),
  _master_nodes(master_nodes),
  _weights(weights)
{
}

void
XFEMCutElemNode::get_coords(Real *coords, MeshBase *displaced_mesh) const
{
  coords[0] = 0.0;
  coords[1] = 0.0;
  coords[2] = 0.0;
  std::vector<Node*> master_nodes;
  if (displaced_mesh)
  {
    for (unsigned int i=0; i<_master_nodes.size(); ++i)
    {
      master_nodes.push_back(displaced_mesh->node_ptr(_master_nodes[i]->id()));
    }
  }
  else
  {
    master_nodes = _master_nodes;
  }

  for (unsigned int i=0; i<master_nodes.size(); ++i)
  {
    coords[0] += _weights[i] * (*master_nodes[i])(0);
    coords[1] += _weights[i] * (*master_nodes[i])(1);
    coords[2] += _weights[i] * (*master_nodes[i])(2);
  }
}

XFEMCutElem::XFEMCutElem(Elem* elem, const CutElemMesh::element_t * const CEMelem):
  _elem(elem),
  _n_nodes(elem->n_nodes()),
  _nodes(_n_nodes,NULL),
  _physical_nodes(_n_nodes,false)
{
  for (unsigned int i=0; i<_n_nodes; ++i)
  {
    _nodes[i] = elem->get_node(i);
  }
  save_fragment_info(CEMelem);
  calc_physical_volfrac();
}

XFEMCutElem::~XFEMCutElem()
{
}

void
XFEMCutElem::save_fragment_info(const CutElemMesh::element_t * const elem)
{
  _local_edge_has_intersection = elem->local_edge_has_intersection;
  _embedded_nodes_on_edge = elem->embedded_nodes_on_edge;
  _intersection_x = elem->intersection_x;
  if (elem->fragments.size() != 1)
  {
    libMesh::err << " ERROR: In save_fragment_info New elements must have 1 interior link"<<std::endl;
    exit(1);
  }
  for (unsigned int i=0; i<elem->fragments[0]->boundary_nodes.size(); ++i)
  {
    CutElemMesh::node_t * node = elem->fragments[0]->boundary_nodes[i];
    if (node->category == CutElemMesh::N_CATEGORY_EMBEDDED)
    {
      bool found_edge(false);
      std::vector<Node*> master_nodes;
      std::vector<Real> master_weights;
      for (unsigned int iedge=0; iedge<elem->num_edges; ++iedge)
      {
        if (elem->embedded_nodes_on_edge[iedge] == node)
        {
          elem->intersection_x[iedge];
          int iedgeplus1(iedge<(elem->num_edges-1) ? iedge+1 : 0);
          master_nodes.push_back(_nodes[iedge]);
          master_nodes.push_back(_nodes[iedgeplus1]);
          master_weights.push_back(1.0-_intersection_x[iedge]);
          master_weights.push_back(_intersection_x[iedge]);

          found_edge = true;
          break;
        }
      }
      if (!found_edge)
      {
        libMesh::err << " ERROR: In save_fragment_info could not find embedded node on edges of element"<<std::endl;
        exit(1);
      }

      XFEMCutElemNode xfcen(CutElemMesh::N_CATEGORY_EMBEDDED, node->id, master_nodes, master_weights);
      _interior_link.push_back(xfcen);
      _cut_line_nodes.push_back(xfcen);
    }
    else if (node->category == CutElemMesh::N_CATEGORY_PERMANENT)
    {
      bool found_node = false;
      for (unsigned int j=0; j<elem->nodes.size(); ++j)
      {
        if (elem->nodes[j] == node)
        {
          _physical_nodes[j] = true;
          std::vector<Node*> master_nodes;
          std::vector<Real> master_weights;
          master_nodes.push_back(_nodes[j]);
          master_weights.push_back(1.0);
          XFEMCutElemNode xfcen(CutElemMesh::N_CATEGORY_LOCAL_INDEX, j, master_nodes, master_weights);
          _interior_link.push_back(xfcen);
          found_node = true;
          break;
        }
      }
      if (!found_node)
      {
        libMesh::err << " ERROR: In save_fragment_info link could not find permanent node "<<node->id<<" in element "<<elem->id<<std::endl;
        exit(1);
      }
    }
    else
    {
      libMesh::err << " ERROR: In save_fragment_info link nodes must be either embedded or permanent"<<std::endl;
      exit(1);
    }
  }
  if (_cut_line_nodes.size() != 2)
  {
    libMesh::err << " ERROR: In save_fragment_info interior link must contain exactly 2 embedded nodes to define cut line"<<std::endl;
    exit(1);
  }
}

void
XFEMCutElem::calc_physical_volfrac()
{
  Real frag_area = 0.0;
  Real el_area = 0.0;

  //Calculate area of entire element and fragment using the formula:
  // A = 1/2 sum_{i=0}^{n-1} (x_i y_{i+1} - x_{i+1} y{i})

  for (unsigned int i=0; i<_n_nodes; ++i)
  {
    int iplus1(i<(_n_nodes-1) ? i+1 : 0);
    el_area += 0.5 * ((*_nodes[i])(0)*(*_nodes[iplus1])(1) - (*_nodes[iplus1])(0)*(*_nodes[i])(1));
  }

  for (unsigned int i=0; i<_interior_link.size(); ++i)
  {
    int iplus1(i<(_interior_link.size()-1) ? i+1 : 0);
    Real nodeicoords[3];
    Real nodeip1coords[3];
    _interior_link[i].get_coords(nodeicoords);
    _interior_link[iplus1].get_coords(nodeip1coords);
    frag_area += 0.5 * (nodeicoords[0]*nodeip1coords[1] - nodeip1coords[0]*nodeicoords[1]);
  }

  _physical_volfrac = frag_area/el_area;
}

void
XFEMCutElem::get_origin(Real *origin, MeshBase* displaced_mesh)const
{
  _cut_line_nodes[0].get_coords(origin, displaced_mesh);
}

void
XFEMCutElem::get_normal(Real *normal, MeshBase* displaced_mesh)const
{
  Real node1coords[3];
  Real node2coords[3];
  Real p1p2[2];

  _cut_line_nodes[0].get_coords(node1coords, displaced_mesh);
  _cut_line_nodes[1].get_coords(node2coords, displaced_mesh);

  p1p2[0] = node2coords[0] - node1coords[0];
  p1p2[1] = node2coords[1] - node1coords[1];
  Real len = sqrt(p1p2[0]*p1p2[0] + p1p2[1]*p1p2[1]);
  p1p2[0] /= len;
  p1p2[1] /= len;

  //Take the cross product of the unit vector from node 1 to node 2
  //and the vector pointing out of plane to get the outward
  //normal of this cut of the fragment.  Reduces to this in 2d:
  normal[0] =  p1p2[1];
  normal[1] = -p1p2[0];
  normal[2] = 0.0;
}

//-----------------------------------------------------------------
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
XFEM::clearStateMarkedElems()
{
  _state_marked_elems.clear();
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

  MeshBase::element_iterator       elem_it  = _mesh->elements_begin();
  const MeshBase::element_iterator elem_end = _mesh->elements_end();

  //Load all existing elements in to CutElemMesh
  for ( elem_it = _mesh->elements_begin(); elem_it != elem_end; ++elem_it)
  {
    Elem *elem = *elem_it;
    {
      unsigned int quad_nodes[4]={elem->node(0),elem->node(1),elem->node(2),elem->node(3)};
      std::vector<unsigned int >quad (quad_nodes, quad_nodes+sizeof(quad_nodes)/sizeof(unsigned int));
      _efa_mesh.addElement(quad, elem->id());
    }
  }

  //Restore fragment information for elements that have been previously cut
  for ( elem_it = _mesh->elements_begin(); elem_it != elem_end; ++elem_it)
  {
    Elem *elem = *elem_it;
    {
      std::map<const Elem*, XFEMCutElem*>::iterator cemit = _cut_elem_map.find(elem);
      if (cemit != _cut_elem_map.end())
      {
        XFEMCutElem *xfce = cemit->second;
        std::vector<std::pair<CutElemMesh::N_CATEGORY, unsigned int> > interior_link;
        for (unsigned int iil=0; iil<xfce->_interior_link.size(); ++iil)
        {
          interior_link.push_back(std::make_pair(xfce->_interior_link[iil].get_category(),
                                                 xfce->_interior_link[iil].get_index()));
        }
        CutElemMesh::element_t * CEMElem = _efa_mesh.getElemByID(elem->id());
        _efa_mesh.restoreFragmentInfo(CEMElem,
                                      interior_link);
      }
    }
  }

  _efa_mesh.updateEdgeNeighbors();

  //Restore edge intersection information for elements that have been previously cut
  for ( elem_it = _mesh->elements_begin(); elem_it != elem_end; ++elem_it)
  {
    Elem *elem = *elem_it;
    {
      std::map<const Elem*, XFEMCutElem*>::iterator cemit = _cut_elem_map.find(elem);
      if (cemit != _cut_elem_map.end())
      {
        XFEMCutElem *xfce = cemit->second;
        std::vector<std::pair<CutElemMesh::N_CATEGORY, unsigned int> > interior_link;
        for (unsigned int iil=0; iil<xfce->_interior_link.size(); ++iil)
        {
          interior_link.push_back(std::make_pair(xfce->_interior_link[iil].get_category(),
                                                 xfce->_interior_link[iil].get_index()));
        }
        CutElemMesh::element_t * CEMElem = _efa_mesh.getElemByID(elem->id());
        _efa_mesh.restoreEdgeIntersections(CEMElem,
                                           xfce->_local_edge_has_intersection,
                                           xfce->_embedded_nodes_on_edge,
                                           xfce->_intersection_x);
      }
    }
  }


//  _efa_mesh.updatePhysicalLinksAndFragments();
  _efa_mesh.initCrackTipTopology();

//  std::cout << _efa_mesh << std::endl;
}

bool
XFEM::mark_cut_edges(Real time)
{
  bool marked_edges = false;
  marked_edges = mark_cut_edges_by_geometry(time);
  marked_edges |= mark_cut_edges_by_state();
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
    Elem *elem = *elem_it;
    std::vector<cutEdge> cutEdges;
    for (unsigned int i=0; i<_geometric_cuts.size(); ++i)
    {
      _geometric_cuts[i]->cut_elem_by_geometry(elem, cutEdges, time);
    }

    for (unsigned int i=0; i<cutEdges.size(); ++i)
    {
      //Figure out the side id from the edge nodes -- maybe better to
      //just return that from cut_elem_by_geometry
      unsigned int nsides = 4;
      unsigned int side_id = 999999;
      Real distance = -1.0;
      for (unsigned int j=0; j<nsides; ++j)
      {
        Node *node1 = elem->get_node(j);
        Node *node2 = elem->get_node(j<nsides-1?j+1:0);

        if (cutEdges[i].id1 == node1->id() && cutEdges[i].id2 == node2->id())
        {
          side_id = j;
          distance = cutEdges[i].distance;
          break;
        }
        else if (cutEdges[i].id2 == node1->id() && cutEdges[i].id1 == node2->id())
        {
          side_id = j;
          distance = 1.0-cutEdges[i].distance;
          break;
        }
      }
      if (side_id == 999999)
      {
        libMesh::err << " ERROR: could not find side index for nodes: "<<cutEdges[i].id1<<" "<<cutEdges[i].id2<<std::endl;
        exit(1);
      }
      if (distance < 0.0)
      {
        libMesh::err << " ERROR: invalid distance for cut edge with nodes: "<<cutEdges[i].id1<<" "<<cutEdges[i].id2<<" : "<<distance<<std::endl;
        exit(1);
      }

      // elemid, edgeid, location
      _efa_mesh.addEdgeIntersection(elem->id(),side_id,distance);
      marked_edges = true;
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
    CutElemMesh::element_t * CEMElem = _efa_mesh.getElemByID(elem->id());

    //find existing cut edge
    unsigned int nsides = 4;
    unsigned int nnodes = elem->n_nodes();
    unsigned int orig_cut_side_id = 999999;
    Real orig_cut_distance = -1.0;
    for (unsigned int i=0; i<nsides; ++i)
    {
      if (CEMElem->local_edge_has_intersection[i])
      {
        if (orig_cut_side_id != 999999)
        {
          libMesh::err << " ERROR: element "<<elem->id()<<" marked for crack growth, but already has multiple cuts"<<std::endl;
          exit(1);
        }
        orig_cut_side_id = i;
        orig_cut_distance = CEMElem->intersection_x[i];
      }
    }
    if (orig_cut_side_id == 999999)
    {
      std::map<const Elem*, unsigned int>::iterator mit;
      mit = _state_marked_elem_sides.find(elem);
      if (mit != _state_marked_elem_sides.end())
      {
        orig_cut_side_id = mit->second;
        orig_cut_distance = 0.5;
        _efa_mesh.addEdgeIntersection(elem->id(),orig_cut_side_id,orig_cut_distance);
      }
      else
      {
        libMesh::err << " ERROR: element "<<elem->id()<<" flagged for state-based growth, but has no edge intersections"<<std::endl;
        exit(1);
      }
    }

    //find position of existing edge cut to use for cutting plane origin
    Node *node1 = elem->get_node(orig_cut_side_id);
    Node *node2 = elem->get_node(orig_cut_side_id<(nsides-1)?orig_cut_side_id+1:0);

    Real cut_origin_x = (1.0-orig_cut_distance)*(*node1)(0) + orig_cut_distance*(*node2)(0);
    Real cut_origin_y = (1.0-orig_cut_distance)*(*node1)(1) + orig_cut_distance*(*node2)(1);

    //loop through nodes, find distance from cut plane
    std::vector<Real> plane_to_node_dist;
    plane_to_node_dist.reserve(nnodes);

    for (unsigned int i=0; i<nnodes; ++i)
    {
      Node *curr_node = elem->get_node(i);
      Real cut_to_node_x = (*curr_node)(0) - cut_origin_x;
      Real cut_to_node_y = (*curr_node)(1) - cut_origin_y;
      plane_to_node_dist.push_back(normal(0)*cut_to_node_x + normal(1)*cut_to_node_y);
    }

    //loop through edges, see if they are cut, and add intersections if they are
    unsigned int num_cut = 1;
    for (unsigned int i=0; i<nsides; ++i)
    {
      if (i != orig_cut_side_id)
      {
        unsigned int node1idx = i;
        unsigned int node2idx = i<(nsides-1)?i+1:0;
        if (plane_to_node_dist[node1idx]*plane_to_node_dist[node2idx] < 0.0)
        {
          if (num_cut > 1)
          {
            libMesh::err << " ERROR: element "<<elem->id()<<" has too many cuts"<<std::endl;
            exit(1);
          }
          Real distance = -plane_to_node_dist[node1idx] /
                           (plane_to_node_dist[node2idx] - plane_to_node_dist[node1idx]);
          _efa_mesh.addEdgeIntersection(elem->id(),i,distance);
          ++num_cut;
        }
      }
    }

    if (num_cut != 2)
    {
      libMesh::err << " ERROR: element "<<elem->id()<<" must have two cuts.  num: "<<num_cut<<std::endl;
      exit(1);
    }

    marked_edges = true;
  }

  return marked_edges;
}

bool
XFEM::cut_mesh_with_efa()
{
  bool mesh_changed = false;

  std::map<unsigned int, Node*> efa_id_to_new_node;
  std::map<unsigned int, Node*> efa_id_to_new_node2;

  _efa_mesh.updatePhysicalLinksAndFragments();

  _efa_mesh.printMesh();
  std::cout<<"BWS before updateTopology"<<std::endl;

  _efa_mesh.updateTopology();

  _efa_mesh.printMesh();
  std::cout<<"BWS cut done"<<std::endl;

  //Add new nodes
  const std::vector<CutElemMesh::node_t*> NewNodes = _efa_mesh.getNewNodes();
  for (unsigned int i=0; i<NewNodes.size(); ++i)
  {
    unsigned int new_node_id = NewNodes[i]->id;
    unsigned int parent_id = NewNodes[i]->parent->id;

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
  const std::vector<CutElemMesh::element_t*> NewElements = _efa_mesh.getChildElements();
  for (unsigned int i=0; i<NewElements.size(); ++i)
  {
    unsigned int parent_id = NewElements[i]->parent->id;

    Elem *parent_elem = _mesh->elem(parent_id);
    Elem *libmesh_elem = Elem::build(parent_elem->type()).release();

    Elem *parent_elem2 = NULL;
    Elem *libmesh_elem2 = NULL;
    if (_mesh2)
    {
      parent_elem2 = _mesh2->elem(parent_id);
      libmesh_elem2 = Elem::build(parent_elem2->type()).release();
    }

    for (unsigned int j=0; j<NewElements[i]->num_nodes; ++j)
    {
      unsigned int node_id = NewElements[i]->nodes[j]->id;
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

    XFEMCutElem * xfce = new XFEMCutElem(libmesh_elem, NewElements[i]);
    _cut_elem_map.insert(std::pair<Elem*,XFEMCutElem*>(libmesh_elem,xfce));

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
  const std::vector<CutElemMesh::element_t*> DeleteElements = _efa_mesh.getParentElements();
  for (unsigned int i=0; i<DeleteElements.size(); ++i)
  {
    Elem *elem_to_delete = _mesh->elem(DeleteElements[i]->id);

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
      Elem *elem_to_delete2 = _mesh2->elem(DeleteElements[i]->id);
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
    const std::set<CutElemMesh::element_t*> CrackTipElements = _efa_mesh.getCrackTipElements();
    std::set<CutElemMesh::element_t*>::const_iterator sit;
    for (sit = CrackTipElements.begin(); sit != CrackTipElements.end(); ++sit)
    {
      unsigned int eid = (*sit)->id;
      Elem * crack_tip_elem = _mesh->elem(eid);
      _crack_tip_elems.insert(crack_tip_elem);
    }
  }

  //store virtual nodes
  //store cut edge info

//  std::cout << _efa_mesh << std::endl;

  return mesh_changed;
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
    phys_volfrac = xfce->get_physical_volfrac();
  }

  return phys_volfrac;
}

Real
XFEM::get_cut_plane(const Elem* elem,
                    const XFEM_CUTPLANE_QUANTITY quantity) const
{
  Real comp=0.0;
  Real planedata[3];
  std::map<const Elem*, XFEMCutElem*>::const_iterator it;
  it = _cut_elem_map.find(elem);
  if (it != _cut_elem_map.end())
  {
    const XFEMCutElem *xfce = it->second;
    if ((unsigned int)quantity < 3)
    {
      unsigned int index = (unsigned int)quantity;
      xfce->get_origin(planedata,_mesh2);
      comp = planedata[index];
    }
    else if ((unsigned int)quantity < 6)
    {
      unsigned int index = (unsigned int)quantity - 3;
      xfce->get_normal(planedata,_mesh2);
      comp = planedata[index];
    }
    else
    {
      libMesh::err << " ERROR: In get_cut_plane index out of range"<<std::endl;
      exit(1);
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
  return (_cut_elem_map.find(elem) != _cut_elem_map.end());
}
