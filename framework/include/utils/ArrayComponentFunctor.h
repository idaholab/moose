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
 * This is essentially a forwarding functor that forwards the spatial and temporal evaluation
 * arguments to the parent array functor and then returns the result indexed at a given component.
 */
template <typename T, typename ArrayTypeFunctor>
class ArrayComponentFunctor : public FunctorBase<T>
{
public:
  using typename FunctorBase<T>::ValueType;
  using typename FunctorBase<T>::GradientType;
  using typename FunctorBase<T>::DotType;

  ArrayComponentFunctor(const ArrayTypeFunctor & array, const unsigned int component)
    : FunctorBase<T>(array.functorName() + "_" + std::to_string(component)),
      _array(array),
      _component(component)
  {
  }

  bool isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                  const Elem * elem,
                                  const Moose::StateArg & state) const override;
  bool hasBlocks(SubdomainID sub_id) const override { return _array.hasBlocks(sub_id); }

private:
  /// The parent array functor
  const ArrayTypeFunctor & _array;

  /// The component at which we'll index the parent array functor evaluation result
  const unsigned int _component;

  ValueType evaluate(const ElemArg & elem, const StateArg & state) const override final
  {
    return _array(elem, state)[_component];
  }

  ValueType evaluate(const FaceArg & face, const StateArg & state) const override final
  {
    return _array(face, state)[_component];
  }

  ValueType evaluate(const ElemQpArg & elem_qp, const StateArg & state) const override final
  {
    return _array(elem_qp, state)[_component];
  }

  ValueType evaluate(const ElemSideQpArg & elem_side_qp,
                     const StateArg & state) const override final
  {
    return _array(elem_side_qp, state)[_component];
  }

  ValueType evaluate(const ElemPointArg & elem_point, const StateArg & state) const override final
  {
    return _array(elem_point, state)[_component];
  }

  using FunctorBase<T>::evaluateGradient;
  GradientType evaluateGradient(const ElemArg & elem, const StateArg & state) const override final
  {
    return _array.gradient(elem, state)[_component];
  }
};

template <typename T, typename ArrayTypeFunctor>
bool
ArrayComponentFunctor<T, ArrayTypeFunctor>::isExtrapolatedBoundaryFace(
    const FaceInfo & fi, const Elem * const elem, const Moose::StateArg & state) const
{
  return _array.isExtrapolatedBoundaryFace(fi, elem, state);
}
}
