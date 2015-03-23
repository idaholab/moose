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

#ifndef EFAFACE_H
#define EFAFACE_H

#include "EFAedge.h"
#include "FaceNode.h"
#include "EFAfragment2D.h"

class EFAfragment2D;

class EFAface
{
public:

  EFAface(unsigned int n_nodes);
  EFAface(const EFAface & other_face);
  EFAface(const EFAfragment2D * frag);

  ~EFAface();

private:

  unsigned int _num_nodes;
  std::vector<EFAnode*> _nodes;
  unsigned int _num_edges;
  std::vector<EFAedge*> _edges;
  std::vector<FaceNode*> _interior_nodes;

public:

  unsigned int num_nodes() const;
  void set_node(unsigned int node_id, EFAnode* node);
  EFAnode* get_node(unsigned int node_id) const;
  void switchNode(EFAnode *new_node, EFAnode *old_node);
  bool getMasterInfo(EFAnode* node, std::vector<EFAnode*> &master_nodes,
                     std::vector<double> &master_weights) const;
  bool getEdgeNodeParaCoor(EFAnode* node, std::vector<double> &xi_2d) const;
  bool getFaceNodeParaCoor(EFAnode* node, std::vector<double> &xi_2d) const;
  unsigned int num_interior_nodes() const;
  void createNodes();

  unsigned int num_edges() const;
  EFAedge* get_edge(unsigned int edge_id) const;
  void set_edge(unsigned int edge_id, EFAedge* new_edge);
  void createEdges();
  void combine_two_edges(unsigned int edge_id1, unsigned int edge_id2);
  void sort_edges();
  void reverse_edges();
  bool is_trig_quad() const;

  bool overlap_with(const EFAface* other_face) const;
  bool containsNode(const EFAnode* node) const;
  bool containsFace(const EFAface* other_face) const;
  bool doesOwnEdge(const EFAedge* other_edge) const;
  void remove_embedded_node(EFAnode* emb_node);
  std::vector<EFAface*> split() const;
  EFAface* combine_with(const EFAface* other_face) const;
  void reset_edge_intersection(const EFAface* ref_face);

  unsigned int get_num_cuts() const;
  bool has_intersection() const;
  void copy_intersection(const EFAface &from_face);
  bool isAdjacent(const EFAface* other_face) const;
  unsigned int adjacentCommonEdge(const EFAface* other_face) const;
  bool is_same_orientation(const EFAface* other_face) const;
  FaceNode* get_interior_node(unsigned int index) const;

private:
  void mapParaCoorFrom1Dto2D(unsigned int edge_id, double xi_1d,
                             std::vector<double> &xi_2d) const;
};

#endif
