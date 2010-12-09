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
