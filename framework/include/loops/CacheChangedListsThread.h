//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedElementLoopBase.h"

#include "libmesh/stored_range.h"

class CacheChangedListsThread : public ThreadedElementLoopBase<ConstElemRange>
{
public:
  CacheChangedListsThread(MooseMesh & mesh);
  CacheChangedListsThread(CacheChangedListsThread & x, Threads::split split);
  virtual ~CacheChangedListsThread();

  virtual void onElement(const Elem * elem) override;

  void join(const CacheChangedListsThread & y);

  /// The elements that were just refined.
  std::vector<const Elem *> _refined_elements;

  /// The elements that were just coarsened.
  std::vector<const Elem *> _coarsened_elements;

  /// Map of Parent elements to children elements for elements that were just coarsened.
  std::map<const Elem *, std::vector<const Elem *>> _coarsened_element_children;
};
