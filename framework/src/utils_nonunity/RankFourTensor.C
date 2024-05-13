//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankFourTensorImplementation.h"

template class RankFourTensorTempl<Real>;
template class RankFourTensorTempl<ADReal>;

namespace MathUtils
{
template <>
void
mooseSetToZero<RankFourTensor>(RankFourTensor & v)
{
  v.zero();
}
template <>
void
mooseSetToZero<ADRankFourTensor>(ADRankFourTensor & v)
{
  v.zero();
}
}

#define RankTwoTensorMultInstantiate(TemplateClass)                                                \
  template RankTwoTensor RankFourTensor::operator*(const TemplateClass<Real> & a) const;           \
  template ADRankTwoTensor ADRankFourTensor::operator*(const TemplateClass<Real> & a) const;       \
  template ADRankTwoTensor RankFourTensor::operator*(const TemplateClass<ADReal> & a) const;       \
  template ADRankTwoTensor ADRankFourTensor::operator*(const TemplateClass<ADReal> & a) const

RankTwoTensorMultInstantiate(RankTwoTensorTempl);
RankTwoTensorMultInstantiate(TensorValue);
RankTwoTensorMultInstantiate(TypeTensor);

template RankFourTensor RankFourTensor::operator+(const RankFourTensor & a) const;
template ADRankFourTensor ADRankFourTensor::operator+(const RankFourTensor & a) const;
template ADRankFourTensor RankFourTensor::operator+(const ADRankFourTensor & a) const;
template ADRankFourTensor ADRankFourTensor::operator+(const ADRankFourTensor & a) const;

template RankFourTensor RankFourTensor::operator-(const RankFourTensor & a) const;
template ADRankFourTensor ADRankFourTensor::operator-(const RankFourTensor & a) const;
template ADRankFourTensor RankFourTensor::operator-(const ADRankFourTensor & a) const;
template ADRankFourTensor ADRankFourTensor::operator-(const ADRankFourTensor & a) const;

template RankFourTensor RankFourTensor::operator*(const RankFourTensor & a) const;
template ADRankFourTensor ADRankFourTensor::operator*(const RankFourTensor & a) const;
template ADRankFourTensor RankFourTensor::operator*(const ADRankFourTensor & a) const;
template ADRankFourTensor ADRankFourTensor::operator*(const ADRankFourTensor & a) const;
