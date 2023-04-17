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
  // Cache any parents of local elements that have just been refined
  // away.
  //
  // The parent itself might *not* be local, because only some of its
  // new children are.  Make sure to cache every such case exactly once.
  if (elem->refinement_flag() == Elem::JUST_REFINED)
  {
    const Elem * const parent = elem->parent();
    const unsigned int child_num = parent->which_child_am_i(elem);

    bool im_the_lowest_local_child = true;
    for (const auto c : make_range(child_num))
      if (parent->child_ptr(c) && parent->child_ptr(c) != remote_elem &&
          parent->child_ptr(c)->processor_id() == elem->processor_id())
        im_the_lowest_local_child = false;

    if (im_the_lowest_local_child)
      _refined_elements.push_back(parent);
  }

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
