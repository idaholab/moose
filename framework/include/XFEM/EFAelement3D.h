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
  virtual unsigned int num_frags() const;
  virtual bool is_partial() const;
  virtual void get_non_physical_nodes(std::set<EFAnode*> &non_physical_nodes) const;

  virtual void switchNode(EFAnode *new_node, EFAnode *old_node, bool descend_to_parent);
  virtual void switchEmbeddedNode(EFAnode *new_node, EFAnode *old_node);
  virtual void getMasterInfo(EFAnode* node, std::vector<EFAnode*> &master_nodes,
                             std::vector<double> &master_weights) const;
  virtual unsigned int num_interior_nodes() const;

  virtual bool overlays_elem(const EFAelement* other_elem) const;
  virtual unsigned int get_neighbor_index(const EFAelement* neighbor_elem) const;
  virtual void clear_neighbors();
  virtual void setup_neighbors(std::map<EFAnode*, std::set<EFAelement*> > &InverseConnectivityMap);
  virtual void neighbor_sanity_check() const;

  virtual void init_crack_tip(std::set<EFAelement*> &CrackTipElements);
  virtual bool should_duplicate_for_crack_tip(const std::set<EFAelement*> &CrackTipElements);
  virtual bool shouldDuplicateCrackTipSplitElem(const std::set<EFAelement*> &CrackTipElements);
  virtual bool shouldDuplicateForPhantomCorner();
  virtual bool will_crack_tip_extend(std::vector<unsigned int> &split_neighbors) const;
  virtual bool is_crack_tip_elem() const;

  virtual unsigned int get_num_cuts() const;
  virtual bool is_final_cut() const;
  virtual void update_fragments(const std::set<EFAelement*> &CrackTipElements,
                                std::map<unsigned int, EFAnode*> &EmbeddedNodes);
  virtual void fragment_sanity_check(unsigned int n_old_frag_faces, unsigned int n_old_frag_cuts) const;
  virtual void restore_fragment(const EFAelement* const from_elem);

  virtual void create_child(const std::set<EFAelement*> &CrackTipElements,
                            std::map<unsigned int, EFAelement*> &Elements,
                            std::map<unsigned int, EFAelement*> &newChildElements,
                            std::vector<EFAelement*> &ChildElements,
                            std::vector<EFAelement*> &ParentElements,
                            std::map<unsigned int, EFAnode*> &TempNodes);
  virtual void remove_phantom_embedded_nodes();
  virtual void connect_neighbors(std::map<unsigned int, EFAnode*> &PermanentNodes,
                                 std::map<unsigned int, EFAnode*> &TempNodes,
                                 std::map<EFAnode*, std::set<EFAelement*> > &InverseConnectivityMap,
                                 bool merge_phantom_faces);
  virtual void print_elem();

  // EFAelement3D specific methods
  EFAfragment3D* get_fragment(unsigned int frag_id) const;
  std::set<EFAnode*> get_face_nodes(unsigned int face_id) const;
  bool getFaceNodeParaCoor(EFAnode* node, std::vector<double> &xi_3d) const;
  VolumeNode* get_interior_node(unsigned int interior_node_id) const;
  void remove_embedded_node(EFAnode* emb_node, bool remove_for_neighbor);

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
  bool getFragmentFaceEdgeID(unsigned int ElemFaceID, unsigned int ElemFaceEdgeID, 
                             unsigned int &FragFaceID, unsigned int &FragFaceEdgeID) const;
  bool is_real_edge_cut(unsigned int ElemFaceID, unsigned int ElemFaceEdgeID, double position) const;
  bool is_face_phantom(unsigned int face_id) const;
  unsigned int num_face_neighbors(unsigned int face_id) const;
  EFAelement3D* get_face_neighbor(unsigned int face_id, unsigned int neighbor_id) const;

  bool frag_has_tip_faces() const;
  std::vector<unsigned int> get_tip_face_id() const;
  std::set<EFAnode*> get_tip_embedded_nodes() const;
  bool face_contains_tip(unsigned int face_id) const;
  bool frag_face_already_cut(unsigned int ElemFaceID) const;

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
