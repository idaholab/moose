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
  EFAnode * _embedded_node;
  double _intersection_x;

  public:

  bool equivalent(const EFAedge & other) const; // compares end nodes and embedded node
  bool isOverlapping(const EFAedge & other) const; // only compares end nodes
  bool isPartialOverlap(const EFAedge & other) const;
  bool containsEdge(const EFAedge & other) const;

//  bool operator < (const EFAedge & other) const;

  void add_intersection(double position, EFAnode * embedded_node_tmp, EFAnode * from_node);
  EFAnode * get_node(unsigned int index);

  bool has_intersection();
  bool has_intersection_at_position(double position, EFAnode * from_node);
  double get_intersection(EFAnode * from_node);
  double get_xi(EFAnode * node);

  EFAnode * get_embedded_node();
  void consistency_check();
  void switchNode(EFAnode *new_node, EFAnode *old_node);
  bool containsNode(EFAnode *node) const;
  bool getNodeMasters(EFAnode* node, std::vector<EFAnode*> &master_nodes, std::vector<double> &master_weights);
  bool is_interior_edge();
  bool is_elem_full_edge();
  void remove_embedded_node();
};

#endif
