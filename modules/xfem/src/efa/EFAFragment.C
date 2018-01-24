//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
