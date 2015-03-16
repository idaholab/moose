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

#include "XFEMCutElem.h"

enum XFEM_CUTPLANE_QUANTITY
{
  XCC_ORIGIN_X,
  XCC_ORIGIN_Y,
  XCC_ORIGIN_Z,
  XCC_NORMAL_X,
  XCC_NORMAL_Y,
  XCC_NORMAL_Z
};

class XFEM_geometric_cut;

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
  XFEM(std::vector<MaterialData *> & material_data, MeshBase* mesh, MeshBase* mesh2=NULL);

  /**
   * Destructor
   */
  ~XFEM();

  void setSecondMesh(MeshBase* mesh2);

  void addGeometricCut(XFEM_geometric_cut* geometric_cut);

  void addStateMarkedElem(unsigned int elem_id, RealVectorValue normal);
  void addStateMarkedElem(unsigned int elem_id, RealVectorValue normal, unsigned int marked_side);
  void addStateMarkedFrag(unsigned int elem_id, RealVectorValue normal);

  void clearStateMarkedElems();

  /**
   * Method to update the mesh due to modified cut planes
   */
  bool update(Real time);

  void build_efa_mesh();
  bool mark_cut_edges(Real time);
  bool mark_cut_edges_by_geometry(Real time);
  bool mark_cut_edges_by_state();
  bool init_crack_intersect_edge(Point cut_origin, RealVectorValue cut_normal, 
                                 Point edge_p1, Point edge_p2, Real & dist);
  bool cut_mesh_with_efa();
  Point get_efa_node_coor(EFAnode* CEMnode, EFAelement* CEMElem, 
                          const Elem *elem, MeshBase* displaced_mesh = NULL);

  /**
   * Get the volume fraction of an element that is physical
   */
  Real get_elem_phys_volfrac(const Elem* elem) const;

  /**
   * Get specified component of normal or origin for cut plane for a given element
   */
  Real get_cut_plane(const Elem* elem, const XFEM_CUTPLANE_QUANTITY quantity, 
                     unsigned int plane_id) const;

  bool is_elem_at_crack_tip(const Elem* elem) const;
  bool is_elem_cut(const Elem* elem) const;

private:
  std::vector<MaterialData *> & _material_data;

  /**
   * Reference to the mesh.
   */
  MeshBase* _mesh;
  MeshBase* _mesh2;
  std::vector<XFEM_geometric_cut *> _geometric_cuts;

  std::map<const Elem*, XFEMCutElem*> _cut_elem_map;
  std::set<const Elem*> _crack_tip_elems;

  std::map<const Elem*, RealVectorValue> _state_marked_elems;
  std::set<const Elem*> _state_marked_frags;
  std::map<const Elem*, unsigned int> _state_marked_elem_sides;

  LocationMap<Node> _new_nodes_map;
  LocationMap<Node> _new_nodes_map2;

  ElementFragmentAlgorithm _efa_mesh;
};

#endif // XFEM_H
