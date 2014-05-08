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

DiracKernelInfo::DiracKernelInfo()
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
