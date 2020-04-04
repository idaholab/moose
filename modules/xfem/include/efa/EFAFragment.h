//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
#include <map>
#include <set>

class EFANode;

class EFAFragment
{
public:
  EFAFragment();
  virtual ~EFAFragment();

  virtual void switchNode(EFANode * new_node, EFANode * old_node) = 0;
  virtual bool containsNode(EFANode * node) const = 0;
  virtual unsigned int getNumCuts() const = 0;
  virtual unsigned int getNumCutNodes() const = 0;
  virtual std::set<EFANode *> getAllNodes() const = 0;
  virtual bool isConnected(EFAFragment * other_fragment) const = 0;
  virtual void removeInvalidEmbeddedNodes(std::map<unsigned int, EFANode *> & EmbeddedNodes) = 0;

  // common methods
  std::vector<EFANode *> getCommonNodes(EFAFragment * other) const;
};
