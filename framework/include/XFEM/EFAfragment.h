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

#ifndef EFAFRAGMENT_H
#define EFAFRAGMENT_H

#include "EFAedge.h"

class EFAelement;

class EFAfragment
{
public:

  EFAfragment(EFAelement * host,
             bool create_boundary_edges,
             const EFAelement * from_host,
             unsigned int fragment_copy_index = std::numeric_limits<unsigned int>::max());

  //Construct a fragment from another fragment.
  EFAfragment(const EFAfragment & other_frag,
              EFAelement * host);

  ~EFAfragment();

  void switchNode(EFAnode *new_node, EFAnode *old_node);
  bool containsNode(EFAnode *node) const;
  bool isConnected(EFAfragment &other_fragment) const;
  void combine_tip_edges();
  std::vector<EFAfragment*> split();
//  std::vector<EFAnode*> commonNodesWithEdge(EFAedge & other_edge);
  unsigned int get_num_cuts() const;
  EFAelement* get_host() const;
  bool is_edge_interior(unsigned int edge_id) const;
  std::vector<unsigned int> get_interior_edge_id() const;
  bool isSecondaryInteriorEdge(unsigned int edge_id) const;
  unsigned int num_edges() const;
  EFAedge* get_edge(unsigned int edge_id) const;
  std::set<EFAnode*> get_edge_nodes(unsigned int edge_id) const;
  void add_edge(EFAedge* new_edge);
  std::vector<EFAnode*> get_common_nodes(EFAfragment* other) const;

private:

  std::vector<EFAedge*> _boundary_edges;
  EFAelement * _host_elem;
};

#endif
