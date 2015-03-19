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

#ifndef EFAFRAGMENT2D_H
#define EFAFRAGMENT2D_H

#include "EFAedge.h"
#include "EFAfragment.h"

class EFAelement2D;

class EFAfragment2D : public EFAfragment
{
public:

  EFAfragment2D(EFAelement2D * host, bool create_boundary_edges,
                const EFAelement2D * from_host,
                unsigned int frag_id = std::numeric_limits<unsigned int>::max());

  ~EFAfragment2D();

private:

  EFAelement2D * _host_elem;
  std::vector<EFAedge*> _boundary_edges;

public:
  // override pure virtual methods
  void switchNode(EFAnode *new_node, EFAnode *old_node);
  bool containsNode(EFAnode *node) const;
  unsigned int get_num_cuts() const;
  std::set<EFAnode*> get_all_nodes() const;
  bool isConnected(EFAfragment *other_fragment) const;

  // EFAfragment2D specific methods
  void combine_tip_edges();
  bool is_edge_interior(unsigned int edge_id) const;
  std::vector<unsigned int> get_interior_edge_id() const;
  bool isSecondaryInteriorEdge(unsigned int edge_id) const;
  unsigned int num_edges() const;
  EFAedge* get_edge(unsigned int edge_id) const;
  void add_edge(EFAedge* new_edge);
  std::set<EFAnode*> get_edge_nodes(unsigned int edge_id) const;
  EFAelement2D * get_host() const;
  std::vector<EFAfragment2D*> split();
};

#endif
