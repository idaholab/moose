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

#include "EFAFragment.h"

#include "EFAElement.h"
#include "EFAFuncs.h"

EFAFragment::EFAFragment()
{}

EFAFragment::~EFAFragment()
{}

std::vector<EFANode*>
EFAFragment::getCommonNodes(EFAFragment* other) const
{
  std::set<EFANode*> frag1_nodes = getAllNodes();
  std::set<EFANode*> frag2_nodes = other->getAllNodes();
  std::vector<EFANode*> common_nodes = get_common_elems(frag1_nodes, frag2_nodes);
  return common_nodes;
}
