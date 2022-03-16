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

/**
 * This is essentially a forwarding functor that forwards the spatial and temporal evaluation
 * arguments to the parent array functor and then returns the result indexed at a given component.
 */
template <typename T, typename ArrayTypeFunctor>
class ArrayComponentFunctor : public Moose::FunctorBase<T>
{
public:
  using typename Moose::FunctorBase<T>::ValueType;
  using typename Moose::FunctorBase<T>::GradientType;
  using typename Moose::FunctorBase<T>::DotType;

  ArrayComponentFunctor(const ArrayTypeFunctor & array, const unsigned int component)
    : _array(array), _component(component)
  {
  }

  bool isExtrapolatedBoundaryFace(const FaceInfo & fi) const override
  {
    return _array.isExtrapolatedBoundaryFace(fi);
  }

private:
  /// The parent array functor
  const ArrayTypeFunctor & _array;

  /// The component at which we'll index the parent array functor evaluation result
  const unsigned int _component;

  ValueType evaluate(const Moose::ElemArg & elem, const unsigned int state) const override final
  {
    return _array(elem, state)[_component];
  }

  ValueType evaluate(const Moose::ElemFromFaceArg & elem_from_face,
                     const unsigned int state) const override final
  {
    return _array(elem_from_face, state)[_component];
  }

  ValueType evaluate(const Moose::FaceArg & face, const unsigned int state) const override final
  {
    return _array(face, state)[_component];
  }

  ValueType evaluate(const Moose::SingleSidedFaceArg & face,
                     const unsigned int state) const override final
  {
    return _array(face, state)[_component];
  }

  ValueType evaluate(const Moose::ElemQpArg & elem_qp,
                     const unsigned int state) const override final
  {
    return _array(elem_qp, state)[_component];
  }

  ValueType evaluate(const Moose::ElemSideQpArg & elem_side_qp,
                     const unsigned int state) const override final
  {
    return _array(elem_side_qp, state)[_component];
  }
};
