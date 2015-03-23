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

#ifndef EFAELEMENT3D_H
#define EFAELEMENT3D_H

#include "VolumeNode.h"
#include "EFAfragment3D.h"
#include "EFAelement.h"

class EFAelement3D : public EFAelement
{
public:

  EFAelement3D(unsigned int eid, unsigned int n_nodes, unsigned int n_faces);
  EFAelement3D(const EFAelement3D * from_elem, bool convert_to_local);

  ~EFAelement3D();

private:

  unsigned int _num_faces;
  std::vector<EFAface*> _faces;
  std::vector<VolumeNode*> _interior_nodes;
  std::vector<std::vector<EFAelement3D*> > _face_neighbors;
  std::vector<EFAfragment3D*> _fragments;
  std::vector<std::vector<EFAface*> > _adjacent_face_ix;

public:

  // override virtual methods in base class
  unsigned int num_frags() const;
  bool is_partial() const;
  void get_non_physical_nodes(std::set<EFAnode*> &non_physical_nodes) const;

  void switchNode(EFAnode *new_node, EFAnode *old_node,
                  bool descend_to_parent = true);
  void switchEmbeddedNode(EFAnode *new_node, EFAnode *old_node);
  void getMasterInfo(EFAnode* node, std::vector<EFAnode*> &master_nodes,
                     std::vector<double> &master_weights) const;
  unsigned int num_interior_nodes() const;

  bool overlays_elem(const EFAelement* other_elem) const;
  unsigned int get_neighbor_index(const EFAelement* neighbor_elem) const;
  void clear_neighbors();
  void setup_neighbors(std::map<EFAnode*, std::set<EFAelement*> > &InverseConnectivityMap);
  void neighbor_sanity_check() const;

  void init_crack_tip(std::set<EFAelement*> &CrackTipElements);
  bool should_duplicate_for_crack_tip(const std::set<EFAelement*> &CrackTipElements);
  bool shouldDuplicateCrackTipSplitElem();
  bool shouldDuplicateForPhantomCorner();
  bool will_crack_tip_extend(std::vector<unsigned int> &split_neighbors) const;
  bool is_crack_tip_elem() const;

  unsigned int get_num_cuts() const;
  bool is_cut_twice() const;
  void update_fragments(const std::set<EFAelement*> &CrackTipElements,
                        std::map<unsigned int, EFAnode*> &EmbeddedNodes);
  void fragment_sanity_check(unsigned int n_old_frag_faces, unsigned int n_old_frag_cuts) const;
  void restore_fragment(const EFAelement* const from_elem);

  void create_child(const std::set<EFAelement*> &CrackTipElements,
                    std::map<unsigned int, EFAelement*> &Elements,
                    std::map<unsigned int, EFAelement*> &newChildElements,
                    std::vector<EFAelement*> &ChildElements,
                    std::vector<EFAelement*> &ParentElements,
                    std::map<unsigned int, EFAnode*> &TempNodes);
  void remove_phantom_embedded_nodes();
  void connect_neighbors(std::map<unsigned int, EFAnode*> &PermanentNodes,
                         std::map<unsigned int, EFAnode*> &EmbeddedNodes,
                         std::map<unsigned int, EFAnode*> &TempNodes,
                         std::map<EFAnode*, std::set<EFAelement*> > &InverseConnectivityMap,
                         bool merge_phantom_faces);
  void print_elem();

  // EFAelement3D specific methods
  EFAfragment3D* get_fragment(unsigned int frag_id) const;
  std::set<EFAnode*> get_face_nodes(unsigned int face_id) const;
  bool getFaceNodeParaCoor(EFAnode* node, std::vector<double> &xi_3d) const;
  VolumeNode* get_interior_node(unsigned int interior_node_id) const;
  void remove_embedded_node(EFAnode* emb_node, bool remove_for_neighbor);
  bool is_cut_third_times() const;

  unsigned int num_faces() const;
  void set_face(unsigned int face_id, EFAface* face);
  void createFaces();
  EFAface* get_face(unsigned int face_id) const;
  unsigned int get_face_id(EFAface* face) const;
  std::vector<unsigned int> get_common_face_id(const EFAelement3D* other_elem) const;
  unsigned int getNeighborFaceNodeID(unsigned int face_id, unsigned int node_id,
                                     EFAelement3D* neighbor_elem) const;
  unsigned int getNeighborFaceEdgeID(unsigned int face_id, unsigned int edg_id,
                                     EFAelement3D* neighbor_elem) const;
  void create_adjacent_face_ix();
  EFAface* get_adjacent_face(unsigned int face_id, unsigned int edge_id) const;

  EFAface* get_frag_face(unsigned int frag_id, unsigned int face_id) const;
  std::set<EFAnode*> getPhantomNodeOnFace(unsigned int face_id) const;
  bool getFragmentFaceID(unsigned int elem_face_id, unsigned int &frag_face_id) const;
  bool is_face_phantom(unsigned int face_id) const;
  unsigned int num_face_neighbors(unsigned int face_id) const;
  EFAelement3D* get_face_neighbor(unsigned int face_id, unsigned int neighbor_id) const;

  bool frag_has_tip_faces() const;
  std::vector<unsigned int> get_tip_face_id() const;
  std::set<EFAnode*> get_tip_embedded_nodes() const;
  bool face_contains_tip(unsigned int face_id) const;

  void addFaceEdgeCut(unsigned int face_id, unsigned int edge_id, double position, 
                      EFAnode* embedded_node, std::map<unsigned int, EFAnode*> &EmbeddedNodes,
                      bool add_to_neighbor, bool add_to_adjacent);
  void addFragFaceEdgeCut(unsigned int frag_face_id, unsigned int frag_edge_id, double position,
                          std::map<unsigned int, EFAnode*> &EmbeddedNodes, bool add_to_neighbor,
                          bool add_to_adjacent);

private:

  // EFAelement3D specific methods
  void checkNeighborFaceCut(unsigned int face_id, unsigned int edge_id, double position,
                            EFAnode* from_node, EFAnode* embedded_node, EFAnode* &local_embedded);
  void mapParaCoorFrom2Dto3D(unsigned int face_id, std::vector<double> &xi_2d,
                             std::vector<double> &xi_3d) const;
};

#endif
