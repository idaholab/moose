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
#include "CacheChangedListsThread.h"

#include "libmesh/elem.h"

CacheChangedListsThread::CacheChangedListsThread(MooseMesh & mesh) :
    ThreadedElementLoopBase<ConstElemRange>(mesh)
{
}

// Splitting Constructor
CacheChangedListsThread::CacheChangedListsThread(CacheChangedListsThread & x, Threads::split split) :
    ThreadedElementLoopBase<ConstElemRange>(x, split)
{
}

CacheChangedListsThread::~CacheChangedListsThread()
{
}

void
CacheChangedListsThread::onElement(const Elem *elem)
{
  if (elem->refinement_flag() == Elem::INACTIVE && elem->has_children() && elem->child(0)->refinement_flag() == Elem::JUST_REFINED)
    _refined_elements.push_back(elem);

  if (elem->refinement_flag() == Elem::JUST_COARSENED)
  {
    if (elem->has_children())
    {
      _coarsened_elements.push_back(elem);

      std::vector<const Elem *> & children = _coarsened_element_children[elem];

      for(unsigned int child=0; child < elem->n_children(); child++)
        children.push_back(elem->child(child));
    }
  }
}

void
CacheChangedListsThread::join(const CacheChangedListsThread & y)
{
  _refined_elements.insert(_refined_elements.end(), y._refined_elements.begin(), y._refined_elements.end());
  _coarsened_elements.insert(_coarsened_elements.end(), y._coarsened_elements.begin(), y._coarsened_elements.end());
  _coarsened_element_children.insert(y._coarsened_element_children.begin(), y._coarsened_element_children.end());
}
