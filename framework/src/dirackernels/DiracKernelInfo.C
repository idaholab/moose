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
  _points[elem].insert(p);
}

void
DiracKernelInfo::clearPoints()
{
  _elements.clear();
  _points.clear();
}
