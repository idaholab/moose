//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankThreeTensorImplementation.h"

template class RankThreeTensorTempl<Real>;
template class RankThreeTensorTempl<ADReal>;

namespace MathUtils
{
template <>
void
mooseSetToZero<RankThreeTensor>(RankThreeTensor & v)
{
  v.zero();
}

template <>
void
mooseSetToZero<ADRankThreeTensor>(ADRankThreeTensor & v)
{
  v.zero();
}
}
