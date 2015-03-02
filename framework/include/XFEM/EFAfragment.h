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
  bool containsNode(EFAnode *node);
  bool isConnected(EFAfragment &other_fragment);
  void combine_tip_edges();
  std::vector<EFAfragment*> split();
  std::vector<EFAnode*> commonNodesWithEdge(EFAedge & other_edge);
  unsigned int get_num_cuts();
  EFAelement* get_host();
  std::vector<unsigned int> get_interior_edge_id();
  bool is_edge_second_cut(unsigned int edge_id);

  std::vector<EFAedge*> boundary_edges;

  private:

  EFAelement * host_elem;
};

#endif
