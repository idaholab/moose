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

#ifndef EFAEDGE_H
#define EFAEDGE_H

#include "FaceNode.h"

class EFAedge
{
  public:

  EFAedge(EFAnode * node1, EFAnode * node2);
  EFAedge(const EFAedge & other_edge);

  ~EFAedge();

  private:

  EFAnode * _edge_node1;
  EFAnode * _edge_node2;
  std::vector<EFAnode*> _embedded_nodes;
  std::vector<double> _intersection_x;

  public:

  bool equivalent(const EFAedge & other) const; // compares end nodes and embedded node
  bool isOverlapping(const EFAedge & other) const; // only compares end nodes
  bool isPartialOverlap(const EFAedge & other) const;
  bool containsEdge(const EFAedge & other) const;
  bool getNodeMasters(EFAnode* node, std::vector<EFAnode*> &master_nodes, 
                      std::vector<double> &master_weights) const;
//  bool operator < (const EFAedge & other) const;

  void add_intersection(double position, EFAnode * embedded_node_tmp, EFAnode * from_node);
  void reset_intersection(double position, EFAnode * embedded_node_tmp, EFAnode * from_node);
  void copy_intersection(const EFAedge & other, unsigned int from_node_id);
  EFAnode * get_node(unsigned int index) const;
  void reverse_nodes();

  bool has_intersection() const;
  bool has_intersection_at_position(double position, EFAnode * from_node) const;
  double get_intersection(unsigned int emb_id, EFAnode * from_node) const;
  double distance_from_node1(EFAnode * node) const;
  bool is_embedded_node(const EFAnode * node) const;
  unsigned int get_embedded_index(EFAnode * node) const;
  unsigned int get_embedded_index(double position, EFAnode* from_node) const;

  EFAnode * get_embedded_node(unsigned int index) const;
  unsigned int num_embedded_nodes() const;
  void consistency_check();
  void switchNode(EFAnode *new_node, EFAnode *old_node);
  bool containsNode(const EFAnode *node) const;
  void remove_embedded_node();
  void remove_embedded_node(EFAnode * node);
};

#endif
