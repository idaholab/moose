//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseFunctor.h"

namespace Moose
{
/**
 * Wraps non-AD functors such that they can be used in objects that have requested the functor as AD
 */
template <typename T>
class ADWrapperFunctor : public FunctorBase<T>
{
public:
  using typename Moose::FunctorBase<T>::ValueType;
  using typename Moose::FunctorBase<T>::GradientType;
  using typename Moose::FunctorBase<T>::DotType;

  ADWrapperFunctor(const FunctorBase<typename MetaPhysicL::RawType<T>::value_type> & non_ad_functor)
    : FunctorBase<T>(non_ad_functor.functorName() + "_ad_ified", {EXEC_ALWAYS}),
      _non_ad_functor(non_ad_functor)
  {
  }

  virtual bool isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                          const Elem * const elem,
                                          const Moose::StateArg & state) const override
  {
    return _non_ad_functor.isExtrapolatedBoundaryFace(fi, elem, state);
  }
  virtual bool isConstant() const override { return _non_ad_functor.isConstant(); }
  virtual bool hasBlocks(const SubdomainID id) const override
  {
    return _non_ad_functor.hasBlocks(id);
  }
  virtual bool hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const override
  {
    return _non_ad_functor.hasFaceSide(fi, fi_elem_side);
  }

protected:
  ///@{
  /**
   * Forward calls to wrapped object
   */
  ValueType evaluate(const ElemArg & elem, const StateArg & state) const override
  {
    return _non_ad_functor(elem, state);
  }
  ValueType evaluate(const FaceArg & face, const StateArg & state) const override
  {
    return _non_ad_functor(face, state);
  }
  ValueType evaluate(const ElemQpArg & qp, const StateArg & state) const override
  {
    return _non_ad_functor(qp, state);
  }
  ValueType evaluate(const ElemSideQpArg & qp, const StateArg & state) const override
  {
    return _non_ad_functor(qp, state);
  }
  ValueType evaluate(const ElemPointArg & elem_point, const StateArg & state) const override
  {
    return _non_ad_functor(elem_point, state);
  }

  GradientType evaluateGradient(const ElemArg & elem, const StateArg & state) const override
  {
    return _non_ad_functor.gradient(elem, state);
  }
  GradientType evaluateGradient(const FaceArg & face, const StateArg & state) const override
  {
    return _non_ad_functor.gradient(face, state);
  }
  GradientType evaluateGradient(const ElemQpArg & qp, const StateArg & state) const override
  {
    return _non_ad_functor.gradient(qp, state);
  }
  GradientType evaluateGradient(const ElemSideQpArg & qp, const StateArg & state) const override
  {
    return _non_ad_functor.gradient(qp, state);
  }
  GradientType evaluateGradient(const ElemPointArg & elem_point,
                                const StateArg & state) const override
  {
    return _non_ad_functor.gradient(elem_point, state);
  }

  DotType evaluateDot(const ElemArg & elem, const StateArg & state) const override
  {
    return _non_ad_functor.dot(elem, state);
  }
  DotType evaluateDot(const FaceArg & face, const StateArg & state) const override
  {
    return _non_ad_functor.dot(face, state);
  }
  DotType evaluateDot(const ElemQpArg & qp, const StateArg & state) const override
  {
    return _non_ad_functor.dot(qp, state);
  }
  DotType evaluateDot(const ElemSideQpArg & qp, const StateArg & state) const override
  {
    return _non_ad_functor.dot(qp, state);
  }
  DotType evaluateDot(const ElemPointArg & elem_point, const StateArg & state) const override
  {
    return _non_ad_functor.dot(elem_point, state);
  }
  ///@}

private:
  /// Our wrapped AD object
  const FunctorBase<typename MetaPhysicL::RawType<T>::value_type> & _non_ad_functor;
};
}
