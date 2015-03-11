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

#ifndef EFAELEMENT_H
#define EFAELEMENT_H

#include "EFAedge.h"
#include "FaceNode.h"
#include "EFAfragment.h"


class EFAelement
{
public:

  EFAelement(unsigned int eid, unsigned int n_nodes);
  EFAelement(const EFAelement * from_elem, bool convert_to_local);

  ~EFAelement();

private:

  unsigned int _id;
  unsigned int _num_nodes;
  unsigned int _num_edges;
  //array of nodes
  std::vector<EFAnode*> _nodes;
  //list of cut edges
  std::vector<EFAedge*> _edges;
  //list of interior embedded nodes
  std::vector<FaceNode*> _interior_nodes;
  //local nodes
  std::vector<EFAnode*> _local_nodes;
  //parent
  EFAelement * _parent;
  //neighbors on edge
  std::vector<std::vector<EFAelement*> >_edge_neighbors;
  //fragments
  std::vector<EFAfragment*> _fragments;
  //set of children
  std::vector< EFAelement* > _children;
  //special case at crack tip
  bool _crack_tip_split_element;
  std::vector<unsigned int> _crack_tip_neighbors;

public:

  // nodes
  unsigned int id() const;
  unsigned int num_nodes() const;
  void set_node(unsigned int node_id, EFAnode* node);
  EFAnode* get_node(unsigned int node_id) const;
  void display_nodes();
  void switchNode(EFAnode *new_node, EFAnode *old_node,
                  bool descend_to_parent = true);
  void switchEmbeddedNode(EFAnode *new_node, EFAnode *old_node);
  void get_nodes_on_edge(unsigned int edge_idx, std::vector<EFAnode*> &edge_nodes);
  EFAnode * create_local_node_from_global_node(const EFAnode * global_node) const;
  EFAnode * get_global_node_from_local_node(const EFAnode * local_node) const;
  void getMasterInfo(EFAnode* node, std::vector<EFAnode*> &master_nodes,
                     std::vector<double> &master_weights) const;
  unsigned int getLocalNodeIndex(EFAnode * node) const;
  bool getEmbeddedNodeParaCoor(EFAnode* embedded_node, std::vector<double> &para_coor);
  void add_interior_node(FaceNode* face_node);
  unsigned int getNumInteriorNodes() const;
  FaceNode* get_interior_node(unsigned int interior_node_id) const;

  // edges
  unsigned int num_edges() const;
  void set_edge(unsigned int edge_id, EFAedge* edge);
  void createEdges();
  EFAedge* get_edge(unsigned int edge_id) const;

  void set_parent(EFAelement* parent);
  EFAelement* parent() const;
  EFAelement* get_child(unsigned int child_id) const;
  unsigned int num_children() const;
  void add_child(EFAelement* child);
  void remove_parent_children();

  // fragments
  unsigned int num_frags() const;
  EFAfragment* get_fragment(unsigned int frag_id) const;
  void add_fragment(EFAfragment* frag);
  EFAedge* get_frag_edge(unsigned int frag_id, unsigned int edge_id) const;
  bool is_partial() const;
  void get_non_physical_nodes(std::set<EFAnode*> &non_physical_nodes);
  std::set<EFAnode*> getPhantomNodeOnEdge(unsigned int edge_id);
  bool getFragmentEdgeID(unsigned int elem_edge_id, unsigned int &frag_id, unsigned int &frag_edge_id);
  bool is_edge_phantom(unsigned int edge_id);

  // neighbor elements
  bool overlays_elem(const EFAelement* other_elem) const;
  bool overlays_elem(EFAnode* other_edge_node1, EFAnode* other_edge_node2) const;
  unsigned int get_neighbor_index(EFAelement * neighbor_elem);
  unsigned int edge_index_in_neighbor(EFAelement * neighbor_elem);
  unsigned int num_edge_neighbors(unsigned int edge_id) const;
  EFAelement* get_edge_neighbor(unsigned int edge_id, unsigned int neighbor_id) const;
  void clear_neighbors();
  void setup_neighbors(std::map< EFAnode*, std::set<EFAelement*> > &InverseConnectivityMap);
  void neighbor_sanity_check();
  std::vector<EFAnode*> get_common_nodes(const EFAelement* other_elem) const;

  // crack tip
  void init_crack_tip(std::set< EFAelement*> &CrackTipElements);
  void set_crack_tip_split();
  bool is_crack_tip_split();
  void is_edge_split_or_tip(unsigned int edge_id, bool &edge_split, bool &edge_tip) const;
  bool should_duplicate_for_crack_tip(const std::set<EFAelement*> &CrackTipElements);
  bool shouldDuplicateCrackTipSplitElem();
  bool shouldDuplicateForPhantomCorner();
  unsigned int num_crack_tip_neighbors() const;
  unsigned int get_crack_tip_neighbor(unsigned int index) const;
  void add_crack_tip_neighbor(EFAelement * neighbor_elem);
  bool will_crack_tip_extend(std::vector<unsigned int> &split_neighbors);
  bool frag_has_tip_edges();
  unsigned int get_tip_edge_id();

  // cut mesh
  void add_edge_cut(unsigned int edge_id, double position, EFAnode* embedded_node, 
                    std::map< unsigned int, EFAnode*> &EmbeddedNodes);
  void add_frag_edge_cut(unsigned int frag_edge_id, double position,
                         std::map< unsigned int, EFAnode*> &EmbeddedNodes);
  unsigned int get_num_cuts();
  bool is_cut_twice();
  void update_fragments(const std::set<EFAelement*> &CrackTipElements,
                        std::map<unsigned int, EFAnode*> &EmbeddedNodes);
  void fragment_sanity_check();
  void restore_fragment(const EFAelement* const from_elem);

  void create_child(const std::set<EFAelement*> &CrackTipElements,
                    std::map<unsigned int, EFAelement*> &Elements,
                    std::map<unsigned int, EFAelement*> &newChildElements,
                    std::vector<EFAelement*> &ChildElements,
                    std::vector<EFAelement*> &ParentElements,
                    std::map<unsigned int, EFAnode*> &TempNodes);

  void connect_neighbors(std::map<unsigned int, EFAnode*> &PermanentNodes,
                         std::map<unsigned int, EFAnode*> &EmbeddedNodes,
                         std::map<unsigned int, EFAnode*> &TempNodes,
                         std::map<EFAnode*, std::set<EFAelement*> > &InverseConnectivityMap,
                         bool merge_phantom_edges);
  void print_elem();

private:

  void mapParaCoorFrom1Dto2D(unsigned int edge_id, double xi_1d, std::vector<double> &para_coor);

  // methods only called in connect_fragment()
//  void addToMergedEdgeMap(EFAnode* node1, EFAnode* node2, EFAelement* elem1, EFAelement* elem2,
//                          std::map< std::set<EFAnode*>, std::set<EFAelement*> > &MergedEdgeMap);
  void mergeNodes(EFAnode* &childNode, EFAnode* &childOfNeighborNode,
                  EFAelement* childElem, EFAelement* childOfNeighborElem, 
                  std::map<unsigned int, EFAnode*> &PermanentNodes,
                  std::map<unsigned int, EFAnode*> &TempNodes);

  void duplicateEmbeddedNode(unsigned int edgeID, EFAelement* neighborElem,
                             unsigned int neighborEdgeID,
                             std::map<unsigned int, EFAnode*> &EmbeddedNodes);
  void duplicateEmbeddedNode(unsigned int edgeID,
                             std::map<unsigned int, EFAnode*> &EmbeddedNodes);
  void duplicateInteriorEmbeddedNode(std::map<unsigned int, EFAnode*> &EmbeddedNodes);
};

#endif
