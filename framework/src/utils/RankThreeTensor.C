//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DualRealOps.h"
#include "RankThreeTensorImplementation.h"

template class RankThreeTensorTempl<Real>;
template class RankThreeTensorTempl<DualReal>;

namespace MathUtils
{
template <>
void
mooseSetToZero<RankThreeTensorTempl<Real>>(RankThreeTensorTempl<Real> & v)
{
  v.zero();
}

template <>
void
mooseSetToZero<RankThreeTensorTempl<DualReal>>(RankThreeTensorTempl<DualReal> & v)
{
  v.zero();
}
}
