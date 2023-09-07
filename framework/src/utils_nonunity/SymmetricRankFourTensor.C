//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DualRealOps.h"
#include "SymmetricRankFourTensorImplementation.h"

template class SymmetricRankFourTensorTempl<Real>;
template class SymmetricRankFourTensorTempl<ADReal>;

namespace MathUtils
{
template <>
void
mooseSetToZero<SymmetricRankFourTensor>(SymmetricRankFourTensor & v)
{
  v.zero();
}
template <>
void
mooseSetToZero<ADSymmetricRankFourTensor>(ADSymmetricRankFourTensor & v)
{
  v.zero();
}
}

#define SymmetricRankFourTensorMultInstantiate(TemplateClass, opname)                              \
  template TemplateClass<Real> SymmetricRankFourTensor::operator opname(                           \
      const TemplateClass<Real> & a) const;                                                        \
  template TemplateClass<ADReal> ADSymmetricRankFourTensor::operator opname(                       \
      const TemplateClass<Real> & a) const;                                                        \
  template TemplateClass<ADReal> SymmetricRankFourTensor::operator opname(                         \
      const TemplateClass<ADReal> & a) const;                                                      \
  template TemplateClass<ADReal> ADSymmetricRankFourTensor::operator opname(                       \
      const TemplateClass<ADReal> & a) const

SymmetricRankFourTensorMultInstantiate(SymmetricRankTwoTensorTempl, *);
SymmetricRankFourTensorMultInstantiate(SymmetricRankFourTensorTempl, *);
SymmetricRankFourTensorMultInstantiate(SymmetricRankFourTensorTempl, +);
SymmetricRankFourTensorMultInstantiate(SymmetricRankFourTensorTempl, -);
