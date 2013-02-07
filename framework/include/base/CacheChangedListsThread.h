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

#ifndef CACHECHANGEDLISTSTHREAD_H
#define CACHECHANGEDLISTSTHREAD_H

#include "ThreadedElementLoop.h"

// libMesh includes
#include "libmesh/stored_range.h"

class CacheChangedListsThread : public ThreadedElementLoopBase<ConstElemRange>
{
public:
  CacheChangedListsThread(MooseMesh & mesh);
  CacheChangedListsThread(CacheChangedListsThread & x, Threads::split split);
  virtual ~CacheChangedListsThread();

  virtual void onElement(const Elem *elem);

  void join(const CacheChangedListsThread & y);

  /// The elements that were just refined.
  std::vector<const Elem *> _refined_elements;

  /// The elements that were just coarsened.
  std::vector<const Elem *> _coarsened_elements;

  /// Map of Parent elements to children elements for elements that were just coarsened.
  std::map<const Elem *, std::vector<const Elem *> > _coarsened_element_children;
};

#endif //CACHECHANGEDLISTSTHREAD_H
