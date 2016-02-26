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

#include "DiracKernelInfo.h"
#include "MooseMesh.h"

// LibMesh
#include "libmesh/point_locator_base.h"
#include "libmesh/elem.h"

DiracKernelInfo::DiracKernelInfo() :
    _point_locator()
{
}

DiracKernelInfo::~DiracKernelInfo()
{
}

void
DiracKernelInfo::addPoint(const Elem * elem, Point p)
{
  _elements.insert(elem);

  if (!hasPoint(elem, p))
  {
    std::vector<Point> & point_list = _points[elem];
    point_list.push_back(p);
  }
}

void
DiracKernelInfo::clearPoints()
{
  _elements.clear();
  _points.clear();
}



bool
DiracKernelInfo::hasPoint(const Elem * elem, Point p)
{
  std::vector<Point> & point_list = _points[elem];

  std::vector<Point>::iterator
    it = point_list.begin(),
    end = point_list.end();

  for (; it != end; ++it)
  {
    Real delta = (*it - p).norm_sq();

    if (delta < TOLERANCE*TOLERANCE)
      return true;
  }

  // If we haven't found it, we don't have it.
  return false;
}



void
DiracKernelInfo::updatePointLocator(const MooseMesh& mesh)
{
  // Note: we could update the PointLocator *every* time we call this
  // function, but that may introduce an unacceptable overhead in
  // problems which don't need a PointLocator at all.  This issue will
  // most likely become a moot point when we eventually add a shared
  // "CachingPointLocator" to MOOSE.
  // _point_locator = PointLocatorBase::build(TREE_LOCAL_ELEMENTS, mesh);

  // Construct the PointLocator object if *any* processors have Dirac
  // points.  Note: building a PointLocator object is a parallel_only()
  // function, so this is an all-or-nothing thing.
  unsigned pl_needs_rebuild = _elements.size();
  mesh.comm().max(pl_needs_rebuild);

  if (pl_needs_rebuild)
  {
    // PointLocatorBase::build() is a parallel_only function!  So we
    // can't skip building it just becuase our local _elements is
    // empty, it might be non-empty on some other processor!
    _point_locator = PointLocatorBase::build(TREE_LOCAL_ELEMENTS, mesh);
  }
  else
  {
    // There are no elements with Dirac points, but we have been
    // requested to update the PointLocator so we have to assume the
    // old one is invalid.  Therefore we reset it to NULL... however
    // adding this line causes the code to hang because it triggers
    // the PointLocator to be rebuilt in a non-parallel-only segment
    // of the code later... so it's commented out for now even though
    // it's probably the right behavior.
    // _point_locator.reset(NULL);
  }
}



const Elem *
DiracKernelInfo::findPoint(Point p, const MooseMesh& mesh)
{
  // If the PointLocator has never been created, do so now.  NOTE - WE
  // CAN'T DO THIS if findPoint() is only called on some processors,
  // PointLocatorBase::build() is a 'parallel_only' method!
  if (_point_locator.get() == NULL)
    _point_locator = PointLocatorBase::build(TREE_LOCAL_ELEMENTS, mesh);

  // Check that the PointLocator is ready to start locating points.
  // So far I do not have any tests that trip this...
  if (_point_locator->initialized() == false)
    mooseError("Error, PointLocator is not initialized!");

  const Elem * elem = (*_point_locator)(p);

  // Note: The PointLocator object returns NULL when the Point is not
  // found within the Mesh.  This is not considered to be an error as
  // far as the DiracKernels are concerned: sometimes the Mesh moves
  // out from the Dirac point entirely and in that case the Point just
  // gets "deactivated".

  return elem;
}
