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

#ifndef XFEM_H
#define XFEM_H

// Local Includes -----------------------------------
#include "libmesh/libmesh_common.h"
#include "libmesh/libmesh.h" // libMesh::invalid_uint
#include "libmesh/location_maps.h"
#include "libmesh/mesh.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/vector_value.h"
#include "libmesh/point_locator_base.h"

// C++ Includes   -----------------------------------
#include <vector>

#include "CutElemMesh.h"

enum XFEM_CUTPLANE_QUANTITY
{
  XCC_ORIGIN_X=1,
  XCC_ORIGIN_Y,
  XCC_ORIGIN_Z,
  XCC_NORMAL_X,
  XCC_NORMAL_Y,
  XCC_NORMAL_Z
};

class XFEM_geometric_cut;

class XFEMCutElemNode
{
public:
  XFEMCutElemNode(CutElemMesh::FRAG_NODE_CATEGORY category,
                  unsigned int index,
                  std::vector<Node*> master_nodes,
                  std::vector<Real> weights);
  XFEMCutElemNode(CutElemMesh::FRAG_NODE_CATEGORY category,
                  unsigned int index);
  ~XFEMCutElemNode(){}

  CutElemMesh::FRAG_NODE_CATEGORY get_category() const { return _category; }
  unsigned int get_index() const { return _index; }
  void get_coords(Real *coords, MeshBase *displaced_mesh=NULL) const;

private:
  CutElemMesh::FRAG_NODE_CATEGORY _category;
  unsigned int _index;
  std::vector<Node*> _master_nodes;
  std::vector<Real> _weights;
};

class XFEMCutElem
{
public:
  XFEMCutElem(Elem* elem,const CutElemMesh::element_t * const CEMelem);
  ~XFEMCutElem();

  void set_nodes_from_elem(Elem *elem);
  void save_fragment_info(const CutElemMesh::element_t * const elem);
//  XFEMCutElem cut_with_plane();

private:

  Elem* _elem;
  unsigned int _n_nodes;
  std::vector<Node*> _nodes;
  std::vector<bool> _physical_nodes;

  Real _physical_volfrac;
//  std::vector<XFEMCut> _cuts;
  std::vector<XFEMCutElemNode> _cut_line_nodes;

public:
  void calc_physical_volfrac();
  Real get_physical_volfrac()const {return _physical_volfrac;}
  void get_origin(Real *origin, MeshBase* displaced_mesh=NULL) const;
  void get_normal(Real *normal, MeshBase* displaced_mesh=NULL) const;
  std::vector<XFEMCutElemNode> _interior_link;
  std::vector<bool> _local_edge_has_intersection;
  std::vector<CutElemMesh::node_t*> _embedded_nodes_on_edge;
  std::vector<Real> _intersection_x;
};

/**
 * This is the \p XFEM class.  This class implements
 * algorithms for dynamic mesh modification in support of
 * a phantom node approach for XFEM
 */


// ------------------------------------------------------------
// XFEM class definition
class XFEM
{
public:

  /**
   * Constructor
   */
  explicit
  XFEM(MeshBase* mesh, MeshBase* mesh2=NULL);

  /**
   * Destructor
   */
  ~XFEM();

  void setSecondMesh(MeshBase* mesh2);

  void addGeometricCut(XFEM_geometric_cut* geometric_cut);

  void addStateMarkedElem(unsigned int elem_id, RealVectorValue normal);

  /**
   * Method to update the mesh due to modified cut planes
   */
  bool update(Real time);

  void build_efa_mesh();
  bool mark_cut_edges(Real time);
  bool mark_cut_edges_by_geometry(Real time);
  bool mark_cut_edges_by_state();
  bool cut_mesh_with_efa();

  /**
   * Get the volume fraction of an element that is physical
   */
  Real get_elem_phys_volfrac(const Elem* elem) const;

  /**
   * Get specified component of normal or origin for cut plane for a given element
   */
  Real get_cut_plane(const Elem* elem, const XFEM_CUTPLANE_QUANTITY quantity) const;

  bool is_elem_at_crack_tip(const Elem* elem) const;

private:
  /**
   * Reference to the mesh.
   */
  MeshBase* _mesh;
  MeshBase* _mesh2;
  std::vector<XFEM_geometric_cut *> _geometric_cuts;

  std::map<const Elem*, XFEMCutElem*> _cut_elem_map;
  std::set<const Elem*> _crack_tip_elems;

  std::map<const Elem*, RealVectorValue> _state_marked_elems;

  LocationMap<Node> _new_nodes_map;
  LocationMap<Node> _new_nodes_map2;

  CutElemMesh _efa_mesh;
};

#endif // XFEM_H
