//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GreenGaussGradient.h"
#include "ArrayComponentFunctor.h"

#include <vector>
#include <array>

namespace Moose
{
namespace FV
{
template <typename T>
typename Moose::FunctorBase<std::vector<T>>::GradientType
greenGaussGradient(const ElemArg &,
                   const Moose::FunctorBase<std::vector<T>> &,
                   const bool,
                   const MooseMesh &)
{
  mooseError("It doesn't make any sense to call this function. There is no size to a vector "
             "functor. I suppose we could call the value type overload and get the size from the "
             "returned vector size, but that's not very efficient. If you want us to do that "
             "though, please contact a MOOSE developer");
}

template <typename T, std::size_t N>
typename Moose::FunctorBase<std::array<T, N>>::GradientType
greenGaussGradient(const ElemArg & elem_arg,
                   const Moose::FunctorBase<std::array<T, N>> & functor,
                   const bool two_term_boundary_expansion,
                   const MooseMesh & mesh)
{
  typedef typename Moose::FunctorBase<std::array<T, N>>::GradientType GradientType;
  GradientType ret;
  for (const auto i : make_range(N))
  {
    // Note that this can be very inefficient. Within the scalar greenGaussGradient routine we're
    // going to do value type evaluations of the array functor from scalar_functor and we will be
    // discarding all the value type evaluations other than the one corresponding to i
    ArrayComponentFunctor<T, FunctorBase<std::array<T, N>>> scalar_functor(functor, i);
    ret[i] = greenGaussGradient(elem_arg, scalar_functor, two_term_boundary_expansion, mesh);
  }

  return ret;
}
}
}
