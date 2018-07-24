//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CacheChangedListsThread.h"

#include "libmesh/elem.h"

CacheChangedListsThread::CacheChangedListsThread(MooseMesh & mesh)
  : ThreadedElementLoopBase<ConstElemRange>(mesh)
{
}

// Splitting Constructor
CacheChangedListsThread::CacheChangedListsThread(CacheChangedListsThread & x, Threads::split split)
  : ThreadedElementLoopBase<ConstElemRange>(x, split)
{
}

CacheChangedListsThread::~CacheChangedListsThread() {}

void
CacheChangedListsThread::onElement(const Elem * elem)
{
  if (elem->refinement_flag() == Elem::INACTIVE && elem->has_children() &&
      elem->child_ptr(0)->refinement_flag() == Elem::JUST_REFINED)
    _refined_elements.push_back(elem);

  if (elem->refinement_flag() == Elem::JUST_COARSENED)
  {
    if (elem->has_children())
    {
      _coarsened_elements.push_back(elem);

      std::vector<const Elem *> & children = _coarsened_element_children[elem];

      for (unsigned int child = 0; child < elem->n_children(); child++)
        children.push_back(elem->child_ptr(child));
    }
  }
}

void
CacheChangedListsThread::join(const CacheChangedListsThread & y)
{
  _refined_elements.insert(
      _refined_elements.end(), y._refined_elements.begin(), y._refined_elements.end());
  _coarsened_elements.insert(
      _coarsened_elements.end(), y._coarsened_elements.begin(), y._coarsened_elements.end());
  _coarsened_element_children.insert(y._coarsened_element_children.begin(),
                                     y._coarsened_element_children.end());
}
