//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DualRealOps.h"
#include "SymmetricRankTwoTensorImplementation.h"

template class SymmetricRankTwoTensorTempl<Real>;
template class SymmetricRankTwoTensorTempl<DualReal>;

namespace MathUtils
{
template <>
void
mooseSetToZero<SymmetricRankTwoTensor>(SymmetricRankTwoTensor & v)
{
  v.zero();
}

template <>
void
mooseSetToZero<ADSymmetricRankTwoTensor>(ADSymmetricRankTwoTensor & v)
{
  v.zero();
}
}

template SymmetricRankTwoTensor
SymmetricRankTwoTensor::operator+(const SymmetricRankTwoTensor & a) const;
template ADSymmetricRankTwoTensor
ADSymmetricRankTwoTensor::operator+(const SymmetricRankTwoTensor & a) const;
template ADSymmetricRankTwoTensor
SymmetricRankTwoTensor::operator+(const ADSymmetricRankTwoTensor & a) const;
template ADSymmetricRankTwoTensor
ADSymmetricRankTwoTensor::operator+(const ADSymmetricRankTwoTensor & a) const;

template SymmetricRankTwoTensor
SymmetricRankTwoTensor::operator-(const SymmetricRankTwoTensor & a) const;
template ADSymmetricRankTwoTensor
ADSymmetricRankTwoTensor::operator-(const SymmetricRankTwoTensor & a) const;
template ADSymmetricRankTwoTensor
SymmetricRankTwoTensor::operator-(const ADSymmetricRankTwoTensor & a) const;
template ADSymmetricRankTwoTensor
ADSymmetricRankTwoTensor::operator-(const ADSymmetricRankTwoTensor & a) const;
