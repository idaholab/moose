/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef EFAFRAGMENT_H
#define EFAFRAGMENT_H

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
  virtual std::set<EFANode *> getAllNodes() const = 0;
  virtual bool isConnected(EFAFragment * other_fragment) const = 0;
  virtual void removeInvalidEmbeddedNodes(std::map<unsigned int, EFANode *> & EmbeddedNodes) = 0;

  // common methods
  std::vector<EFANode *> getCommonNodes(EFAFragment * other) const;
};

#endif
