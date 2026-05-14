//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  /// Identify embedded nodes that should be dropped, appending them to
  /// invalid_emb_out.  2D fragments may also perform their (local) cleanup
  /// in-place; 3D fragments only collect, leaving the actual removal/free to a
  /// global sweep in the EFA driver.
  virtual void removeInvalidEmbeddedNodes(std::map<unsigned int, EFANode *> & EmbeddedNodes,
                                          std::vector<EFANode *> & invalid_emb_out) = 0;

  // common methods
  std::vector<EFANode *> getCommonNodes(EFAFragment * other) const;
};
