/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "EFAFragment.h"

#include "EFAElement.h"
#include "EFAFuncs.h"

EFAFragment::EFAFragment() {}

EFAFragment::~EFAFragment() {}

std::vector<EFANode *>
EFAFragment::getCommonNodes(EFAFragment * other) const
{
  std::set<EFANode *> frag1_nodes = getAllNodes();
  std::set<EFANode *> frag2_nodes = other->getAllNodes();
  std::vector<EFANode *> common_nodes = Efa::getCommonElems(frag1_nodes, frag2_nodes);
  return common_nodes;
}
