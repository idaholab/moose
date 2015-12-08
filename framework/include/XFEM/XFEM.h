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

#include "XFEMCutElem2D.h"
#include "XFEMCutElem3D.h"
#include "AuxiliarySystem.h"
#include "NonlinearSystem.h"

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

  /**
   * Initialize the solution on newly created nodes
   */
  void initSolution(NonlinearSystem & nl, AuxiliarySystem & aux);

  Node * getNodeFromUniqueID(unique_id_type uid);

  void build_efa_mesh();
  bool mark_cut_edges(Real time);
  bool mark_cut_edges_by_geometry(Real time);
  bool mark_cut_edges_by_state(Real time);
  bool mark_cut_faces_by_geometry(Real time);
  bool mark_cut_faces_by_state();
  bool init_crack_intersect_edge(Point cut_origin, RealVectorValue cut_normal,
                                 Point edge_p1, Point edge_p2, Real & dist);
  bool cut_mesh_with_efa();
  Point get_efa_node_coor(EFAnode* CEMnode, EFAelement* CEMElem,
                          const Elem *elem, MeshBase* displaced_mesh = NULL) const;

  /**
   * Get the volume fraction of an element that is physical
   */
  Real get_elem_phys_volfrac(const Elem* elem) const;
  Real get_elem_new_weights(const Elem* elem, unsigned int i_qp, std::vector<Point> &g_points, std::vector<Real> &g_weights) const;
  Real flag_qp_inside(const Elem* elem, const Point & p) const;

  /**
   * Get specified component of normal or origin for cut plane for a given element
   */
  Real get_cut_plane(const Elem* elem, const XFEM_CUTPLANE_QUANTITY quantity,
                     unsigned int plane_id) const;

  bool is_elem_at_crack_tip(const Elem* elem) const;
  bool is_elem_cut(const Elem* elem) const;
  void get_frag_faces(const Elem* elem, std::vector<std::vector<Point> > &frag_faces,
                      bool displaced_mesh = false) const;
  void store_crack_tip_origin_and_direction();
  void correct_crack_extension_angle(const Elem * elem, EFAelement2D * CEMElem, EFAedge * orig_edge, Point normal, Point crack_tip_origin, Point crack_tip_direction, Real & distance_keep, unsigned int & edge_id_keep, Point & normal_keep);
  void get_crack_tip_origin(std::map<unsigned int, const Elem*> & elem_id_crack_tip, std::vector<Point> &  crack_front_points);
  //void update_crack_propagation_direction(const Elem* elem, Point direction);
  //void clear_crack_propagation_direction();
  /**
   * Set and get xfem cut data and type
   */
  std::vector<Real>& get_xfem_cut_data();
  void set_xfem_cut_data(std::vector<Real> &cut_data);
  std::string & get_xfem_cut_type();
  void set_xfem_cut_type(std::string & cut_type);
  std::string & get_xfem_qrule();
  void set_xfem_qrule(std::string & xfem_qrule);
  void set_crack_growth_method(bool use_crack_growth_increment, Real crack_growth_increment);

private:

  void get_frag_edges(const Elem* elem, EFAelement2D* CEMElem,
                      std::vector<std::vector<Point> > &frag_edges) const;
  void get_frag_faces(const Elem* elem, EFAelement3D* CEMElem,
                      std::vector<std::vector<Point> > &frag_faces) const;

private:
  std::vector<MaterialData *> & _material_data;

  /**
   * XFEM cut type and data
   */
  std::vector<Real> _XFEM_cut_data;
  std::string _XFEM_cut_type;

  std::string _XFEM_qrule;

  bool _use_crack_growth_increment;
  Real _crack_growth_increment;

  /**
   * Reference to the mesh.
   */
  MeshBase* _mesh;
  MeshBase* _mesh2;
  std::vector<XFEM_geometric_cut *> _geometric_cuts;

  std::map<const Elem*, XFEMCutElem*> _cut_elem_map;
  std::set<const Elem*> _crack_tip_elems;

  std::map<const Elem*, std::vector<Point> > _elem_crack_origin_direction_map;
 
  //std::map<const Elem*, Point> _crack_propagation_direction_map;

  std::map<const Elem*, RealVectorValue> _state_marked_elems;
  std::set<const Elem*> _state_marked_frags;
  std::map<const Elem*, unsigned int> _state_marked_elem_sides;

  LocationMap<Node> _new_nodes_map;
  LocationMap<Node> _new_nodes_map2;

  std::map<unique_id_type, unique_id_type> _new_node_to_parent_node;

  ElementFragmentAlgorithm _efa_mesh;
};

#endif // XFEM_H
