//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DualRealOps.h"
#include "RankFourTensorImplementation.h"

template class RankFourTensorTempl<Real>;
template class RankFourTensorTempl<DualReal>;

namespace MathUtils
{
template <>
void
mooseSetToZero<RankFourTensorTempl<Real>>(RankFourTensorTempl<Real> & v)
{
  v.zero();
}
template <>
void
mooseSetToZero<RankFourTensorTempl<DualReal>>(RankFourTensorTempl<DualReal> & v)
{
  v.zero();
}
}

#define RankTwoTensorMultInstantiate(TemplateClass)                                                \
  template RankTwoTensorTempl<Real> RankFourTensorTempl<Real>::operator*(                          \
      const TemplateClass<Real> & a) const;                                                        \
  template RankTwoTensorTempl<DualReal> RankFourTensorTempl<DualReal>::operator*(                  \
      const TemplateClass<Real> & a) const;                                                        \
  template RankTwoTensorTempl<DualReal> RankFourTensorTempl<Real>::operator*(                      \
      const TemplateClass<DualReal> & a) const;                                                    \
  template RankTwoTensorTempl<DualReal> RankFourTensorTempl<DualReal>::operator*(                  \
      const TemplateClass<DualReal> & a) const

RankTwoTensorMultInstantiate(RankTwoTensorTempl);
RankTwoTensorMultInstantiate(TensorValue);
RankTwoTensorMultInstantiate(TypeTensor);

template RankFourTensorTempl<Real> RankFourTensorTempl<Real>::
operator+(const RankFourTensorTempl<Real> & a) const;
template RankFourTensorTempl<DualReal> RankFourTensorTempl<DualReal>::
operator+(const RankFourTensorTempl<Real> & a) const;
template RankFourTensorTempl<DualReal> RankFourTensorTempl<Real>::
operator+(const RankFourTensorTempl<DualReal> & a) const;
template RankFourTensorTempl<DualReal> RankFourTensorTempl<DualReal>::
operator+(const RankFourTensorTempl<DualReal> & a) const;

template RankFourTensorTempl<Real> RankFourTensorTempl<Real>::
operator-(const RankFourTensorTempl<Real> & a) const;
template RankFourTensorTempl<DualReal> RankFourTensorTempl<DualReal>::
operator-(const RankFourTensorTempl<Real> & a) const;
template RankFourTensorTempl<DualReal> RankFourTensorTempl<Real>::
operator-(const RankFourTensorTempl<DualReal> & a) const;
template RankFourTensorTempl<DualReal> RankFourTensorTempl<DualReal>::
operator-(const RankFourTensorTempl<DualReal> & a) const;

template RankFourTensorTempl<Real> RankFourTensorTempl<Real>::
operator*(const RankFourTensorTempl<Real> & a) const;
template RankFourTensorTempl<DualReal> RankFourTensorTempl<DualReal>::
operator*(const RankFourTensorTempl<Real> & a) const;
template RankFourTensorTempl<DualReal> RankFourTensorTempl<Real>::
operator*(const RankFourTensorTempl<DualReal> & a) const;
template RankFourTensorTempl<DualReal> RankFourTensorTempl<DualReal>::
operator*(const RankFourTensorTempl<DualReal> & a) const;
