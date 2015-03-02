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

  EFAelement(unsigned int eid);
  EFAelement(const EFAelement * from_elem, bool convert_to_local);

  ~EFAelement();

  void createEdges();

  void switchNode(EFAnode *new_node, EFAnode *old_node,
                  bool descend_to_parent = true);

  void switchEmbeddedNode(EFAnode *new_node, EFAnode *old_node);

  unsigned int id() const;
  bool is_partial();

  //Check if two elements that share a common edge (defined by edge end nodes) overlay each other
  //by looking at the ordering of the nodes.
  bool overlays_elem(EFAnode* other_edge_node1, EFAnode* other_edge_node2);

  bool overlays_elem(EFAelement* other_elem);

  //Get the index of the specifed element in the edge_neighbors vector
  unsigned int get_neighbor_index(EFAelement * neighbor_elem);
  unsigned int get_num_edge_neighbors(unsigned int edge_id);

  //Find out what side the specified element is on, and add it as a crack tip neighbor
  //element for that side.
  void add_crack_tip_neighbor(EFAelement * neighbor_elem);

  //Determine whether the current element is a crack tip element for which the crack will
  //extend into the next element.
  bool will_crack_tip_extend(std::vector<unsigned int> &split_neighbors);

  //Get the nodes on the specified edge and return in the edge_nodes vector
  void get_nodes_on_edge(unsigned int edge_idx, std::vector<EFAnode*> &edge_nodes);

  //Create a set of all non-physical nodes in the current element
  void get_non_physical_nodes(std::set<EFAnode*> &non_physical_nodes);

  //Determine whether element at crack tip should be duplicated.  It should be duplicated
  //if the crack will extend into the next element, or if it has a non-physical node
  //connected to a face where a crack terminates, but will extend.
  bool shouldDuplicateCrackTipSplitElem();
  bool shouldDuplicateForPhantomCorner();

  // get all phantom nodes on the specified edge
  std::set<EFAnode*> getPhantomNodeOnEdge(unsigned int edge_id);

  //Given a global node, create a new local node
  EFAnode * create_local_node_from_global_node(const EFAnode * global_node) const;

  //Given a local node, find the global node corresponding to that node
  EFAnode * get_global_node_from_local_node(const EFAnode * local_node) const;

  //Given a EFAnode, find the element edge or fragment edge that contains it
  //Return its master nodes and weights
  void getMasterInfo(EFAnode* node, std::vector<EFAnode*> &master_nodes,
                     std::vector<double> &master_weights) const;

  //Get the local node index of node
  unsigned int getLocalNodeIndex(EFAnode * node) const;

  //Given a element edge ID, get the corresponding fragment ID
  bool getFragmentEdgeID(unsigned int elem_edge_id, unsigned int &frag_id, unsigned int &frag_edge_id);

  //add an interior edge
  bool is_edge_phantom(unsigned int edge_id);

  //check if the element has only one fragment which has tip edges
  bool frag_has_tip_edges();
  unsigned int get_tip_edge_id();

  //get the parametric coords of an embedded node
  bool getEmbeddedNodeParaCoor(EFAnode* embedded_node, std::vector<double> &para_coor);
  unsigned int getNumInteriorNodes();
  unsigned int get_num_cuts();
  bool is_cut_twice();
  void display_nodes();

  //id
  unsigned int _id;
  unsigned int num_nodes;
  unsigned int num_edges;
  //array of nodes
  std::vector<EFAnode*> nodes;
  //list of cut edges
  std::vector<EFAedge*> edges;
  //list of interior embedded nodes
  std::vector<FaceNode*> interior_nodes;
  //local nodes
  std::vector<EFAnode*> local_nodes;
  //parent
  EFAelement * parent;
  //neighbors on edge
  std::vector<std::vector<EFAelement*> >edge_neighbors;
  //fragments
  std::vector<EFAfragment*> fragments;
  //set of children
  std::vector< EFAelement* > children;
  //special case at crack tip
  bool crack_tip_split_element;
  std::vector<unsigned int> crack_tip_neighbors;

private:

  void mapParaCoorFrom1Dto2D(unsigned int edge_id, double xi_1d, std::vector<double> &para_coor);

  template <class T> 
  unsigned int num_common_elems(std::set<T> &v1, std::set<T> &v2);
};

#endif
