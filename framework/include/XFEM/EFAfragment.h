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

class EFAfragment
{
public:

  EFAfragment();
  virtual ~EFAfragment();

  virtual void switchNode(EFAnode *new_node, EFAnode *old_node) = 0;
  virtual bool containsNode(EFAnode *node) const = 0;
  virtual unsigned int get_num_cuts() const = 0;
  virtual std::set<EFAnode*> get_all_nodes() const = 0;
  virtual bool isConnected(EFAfragment *other_fragment) const = 0;

  // common methods
  std::vector<EFAnode*> get_common_nodes(EFAfragment* other) const;
};

#endif
