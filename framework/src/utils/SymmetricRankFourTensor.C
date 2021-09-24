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
template class SymmetricRankFourTensorTempl<DualReal>;

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

#define SymmetricRankTwoTensorMultInstantiate(TemplateClass)                                       \
  template SymmetricRankTwoTensor SymmetricRankFourTensor::operator*(                              \
      const TemplateClass<Real> & a) const;                                                        \
  template ADSymmetricRankTwoTensor ADSymmetricRankFourTensor::operator*(                          \
      const TemplateClass<Real> & a) const;                                                        \
  template ADSymmetricRankTwoTensor SymmetricRankFourTensor::operator*(                            \
      const TemplateClass<DualReal> & a) const;                                                    \
  template ADSymmetricRankTwoTensor ADSymmetricRankFourTensor::operator*(                          \
      const TemplateClass<DualReal> & a) const

SymmetricRankTwoTensorMultInstantiate(SymmetricRankTwoTensorTempl);

template SymmetricRankFourTensor
SymmetricRankFourTensor::operator+(const SymmetricRankFourTensor & a) const;
template ADSymmetricRankFourTensor
ADSymmetricRankFourTensor::operator+(const SymmetricRankFourTensor & a) const;
template ADSymmetricRankFourTensor
SymmetricRankFourTensor::operator+(const ADSymmetricRankFourTensor & a) const;
template ADSymmetricRankFourTensor
ADSymmetricRankFourTensor::operator+(const ADSymmetricRankFourTensor & a) const;

template SymmetricRankFourTensor
SymmetricRankFourTensor::operator-(const SymmetricRankFourTensor & a) const;
template ADSymmetricRankFourTensor
ADSymmetricRankFourTensor::operator-(const SymmetricRankFourTensor & a) const;
template ADSymmetricRankFourTensor
SymmetricRankFourTensor::operator-(const ADSymmetricRankFourTensor & a) const;
template ADSymmetricRankFourTensor
ADSymmetricRankFourTensor::operator-(const ADSymmetricRankFourTensor & a) const;

template SymmetricRankFourTensor
SymmetricRankFourTensor::operator*(const SymmetricRankFourTensor & a) const;
template ADSymmetricRankFourTensor
ADSymmetricRankFourTensor::operator*(const SymmetricRankFourTensor & a) const;
template ADSymmetricRankFourTensor
SymmetricRankFourTensor::operator*(const ADSymmetricRankFourTensor & a) const;
template ADSymmetricRankFourTensor
ADSymmetricRankFourTensor::operator*(const ADSymmetricRankFourTensor & a) const;
