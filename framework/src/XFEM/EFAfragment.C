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

#include "EFAelement.h"
#include "EFAfragment.h"
#include "EFAfuncs.h"

EFAfragment::EFAfragment()
{}

EFAfragment::~EFAfragment()
{}

std::vector<EFAnode*>
EFAfragment::get_common_nodes(EFAfragment* other) const
{
  bool edge_found = false;
  std::set<EFAnode*> frag1_nodes = get_all_nodes();
  std::set<EFAnode*> frag2_nodes = other->get_all_nodes();
  std::vector<EFAnode*> common_nodes = get_common_elems(frag1_nodes, frag2_nodes);
  return common_nodes;
}
