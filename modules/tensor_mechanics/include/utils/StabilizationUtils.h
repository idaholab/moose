//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseTypes.h"

namespace StabilizationUtils
{
template <typename Functor>
auto
elementAverage(const Functor & f, const MooseArray<Real> & JxW, const MooseArray<Real> & coord)
{
  using ret_type = decltype(f(std::declval<unsigned int>()));
  ret_type avg;
  MathUtils::mooseSetToZero(avg);
  Real v = 0;
  for (auto qp : make_range(JxW.size()))
  {
    avg += f(qp) * JxW[qp] * coord[qp];
    v += JxW[qp] * coord[qp];
  }
  avg /= v;
  return avg;
}
}
