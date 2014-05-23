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

DiracKernelInfo::DiracKernelInfo() :
    _point_locator(NULL)
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
    Real delta = (*it - p).size_sq();

    if (delta < TOLERANCE*TOLERANCE)
      return true;
  }

  // If we haven't found it, we don't have it.
  return false;
}



void
DiracKernelInfo::updatePointLocator(const MooseMesh& mesh)
{
  // Construct the PointLocator object, but only if we have Dirac points
  if (!_elements.empty())
    _point_locator = PointLocatorBase::build(TREE, mesh);
}



const Elem *
DiracKernelInfo::findPoint(Point p, const MooseMesh& mesh)
{
  // If the PointLocator has never been created, do so now.
  if (_point_locator.get() == NULL)
    _point_locator = PointLocatorBase::build(TREE, mesh);

  const Elem * elem = (*_point_locator)(p);

  // Note: The PointLocator object returns NULL when the Point is not
  // found within the Mesh.  This is not considered to be an error as
  // far as the DiracKernels are concerned: sometimes the Mesh moves
  // out from the Dirac point entirely and in that case the Point just
  // gets "deactivated".

  return elem;
}
