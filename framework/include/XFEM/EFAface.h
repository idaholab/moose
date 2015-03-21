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
  EFAface(const EFAfragment2D * frag);

  ~EFAface();

private:

  unsigned int _num_nodes;
  std::vector<EFAnode*> _nodes;
  unsigned int _num_edges;
  std::vector<EFAedge*> _edges;
  std::vector<FaceNode*> _interior_nodes;

public:

  // common methods
  unsigned int num_nodes() const;
  void set_node(unsigned int node_id, EFAnode* node);
  EFAnode* get_node(unsigned int node_id) const;
  void switchNode(EFAnode *new_node, EFAnode *old_node);
  void switchEmbeddedNode(EFAnode *new_node, EFAnode *old_node);
  unsigned int num_edges() const;
  EFAedge* get_edge(unsigned int edge_id) const;
  void set_edge(unsigned int edge_id, EFAedge* new_edge);
  void createEdges();

  bool overlap_with(const EFAface* other_face) const;
  bool containsNode(const EFAnode* node) const;
  bool containsFace(const EFAface* other_face) const;
  void remove_embedded_node(EFAnode* emb_node);
  std::vector<EFAface*> split() const;
};

#endif
