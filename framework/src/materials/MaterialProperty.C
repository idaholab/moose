//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialProperty.h"

namespace moose
{
namespace internal
{
template <>
void
rawValueEqualityHelper(ADRealEigenVector & out, const RealEigenVector & in)
{
  out.resize(in.size());
  for (int i = 0; i < in.size(); ++i)
    out(i) = MetaPhysicL::raw_value(in(i));
}
template <>
void
rawValueEqualityHelper(RealEigenVector & out, const ADRealEigenVector & in)
{
  out.resize(in.size());
  for (int i = 0; i < in.size(); ++i)
    out(i) = MetaPhysicL::raw_value(in(i));
}

template <>
void
rawValueEqualityHelper(ADRealEigenMatrix & out, const RealEigenMatrix & in)
{
  out.resize(in.rows(), in.cols());
  for (int i = 0; i < in.rows(); ++i)
    for (int j = 0; j < in.cols(); ++j)
      out(i, j) = MetaPhysicL::raw_value(in(i, j));
}
template <>
void
rawValueEqualityHelper(RealEigenMatrix & out, const ADRealEigenMatrix & in)
{
  out.resize(in.rows(), in.cols());
  for (int i = 0; i < in.rows(); ++i)
    for (int j = 0; j < in.cols(); ++j)
      out(i, j) = MetaPhysicL::raw_value(in(i, j));
}
}
}
