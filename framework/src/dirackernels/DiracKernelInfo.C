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

DiracKernelInfo::DiracKernelInfo(MooseSystem & moose_system)
  :_moose_system(moose_system)
{
}

DiracKernelInfo::~DiracKernelInfo()
{
}

void
DiracKernelInfo::addPoint(const Elem * elem, Point p)
{
  _elements.insert(elem);
  _points[elem].insert(p);
}

void
DiracKernelInfo::clearPoints()
{
  _elements.clear();
  _points.clear();
}
