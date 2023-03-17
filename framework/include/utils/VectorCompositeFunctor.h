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
#include "libmesh/vector_value.h"

using libMesh::VectorValue;

namespace Moose
{
/**
 * A functor that returns a vector composed of its component functor evaluations
 */
template <typename T>
class VectorCompositeFunctor : public FunctorBase<VectorValue<T>>
{
public:
  template <typename U>
  using FunctorBase = FunctorBase<U>;

  using typename FunctorBase<VectorValue<T>>::ValueType;
  using typename FunctorBase<VectorValue<T>>::GradientType;

  /**
   * From xyz component constructor
   */
  VectorCompositeFunctor(const MooseFunctorName & name,
                         const FunctorBase<T> & x_comp,
                         const FunctorBase<T> & y_comp,
                         const FunctorBase<T> & z_comp);

  /**
   * From xy component constructor
   */
  VectorCompositeFunctor(const MooseFunctorName & name,
                         const FunctorBase<T> & x_comp,
                         const FunctorBase<T> & y_comp);

  /**
   * From x component constructor
   */
  VectorCompositeFunctor(const MooseFunctorName & name, const FunctorBase<T> & x_comp);

private:
  ValueType evaluate(const ElemArg & elem_arg, const TimeArg & time) const override;
  ValueType evaluate(const FaceArg & face, const TimeArg & time) const override;
  ValueType evaluate(const ElemQpArg & elem_qp, const TimeArg & time) const override;
  ValueType evaluate(const ElemSideQpArg & elem_side_qp, const TimeArg & time) const override;
  ValueType evaluate(const ElemPointArg & elem_point_arg, const TimeArg & time) const override;

  using FunctorBase<VectorValue<T>>::evaluateGradient;
  GradientType evaluateGradient(const ElemArg & elem_arg, const TimeArg & time) const override;

  /// Possible holder of constant-0 y-component functor. This will be allocated if the user only
  /// supplies one component functor during construction
  std::unique_ptr<ConstantFunctor<T>> _y_constant;

  /// Possible holder of constant-0 z-component functor. This will be allocated if the user only
  /// supplies two component functors during construction
  std::unique_ptr<ConstantFunctor<T>> _z_constant;

  /// The x-component functor
  const FunctorBase<T> & _x_comp;
  /// The y-component functor
  const FunctorBase<T> & _y_comp;
  /// The z-component functor
  const FunctorBase<T> & _z_comp;
};

template <typename T>
VectorCompositeFunctor<T>::VectorCompositeFunctor(const MooseFunctorName & name,
                                                  const FunctorBase<T> & x_comp,
                                                  const FunctorBase<T> & y_comp,
                                                  const FunctorBase<T> & z_comp)
  : FunctorBase<VectorValue<T>>(name), _x_comp(x_comp), _y_comp(y_comp), _z_comp(z_comp)
{
}

template <typename T>
VectorCompositeFunctor<T>::VectorCompositeFunctor(const MooseFunctorName & name,
                                                  const FunctorBase<T> & x_comp,
                                                  const FunctorBase<T> & y_comp)
  : FunctorBase<VectorValue<T>>(name),
    _z_constant(std::make_unique<ConstantFunctor>(T(0))),
    _x_comp(x_comp),
    _y_comp(y_comp),
    _z_comp(*_z_constant)
{
}

template <typename T>
VectorCompositeFunctor<T>::VectorCompositeFunctor(const MooseFunctorName & name,
                                                  const FunctorBase<T> & x_comp)
  : FunctorBase<VectorValue<T>>(name),
    _y_constant(std::make_unique<ConstantFunctor>(T(0))),
    _z_constant(std::make_unique<ConstantFunctor>(T(0))),
    _x_comp(x_comp),
    _y_comp(*_y_constant),
    _z_comp(*_z_constant)
{
}

template <typename T>
typename VectorCompositeFunctor<T>::ValueType
VectorCompositeFunctor<T>::evaluate(const ElemArg & elem_arg, const TimeArg & time) const
{
  return {_x_comp(elem_arg, time), _y_comp(elem_arg, time), _z_comp(elem_arg, time)};
}

template <typename T>
typename VectorCompositeFunctor<T>::ValueType
VectorCompositeFunctor<T>::evaluate(const FaceArg & face, const TimeArg & time) const
{
  return {_x_comp(face, time), _y_comp(face, time), _z_comp(face, time)};
}

template <typename T>
typename VectorCompositeFunctor<T>::ValueType
VectorCompositeFunctor<T>::evaluate(const ElemQpArg & elem_qp, const TimeArg & time) const
{
  return {_x_comp(elem_qp, time), _y_comp(elem_qp, time), _z_comp(elem_qp, time)};
}

template <typename T>
typename VectorCompositeFunctor<T>::ValueType
VectorCompositeFunctor<T>::evaluate(const ElemSideQpArg & elem_side_qp, const TimeArg & time) const
{
  return {_x_comp(elem_side_qp, time), _y_comp(elem_side_qp, time), _z_comp(elem_side_qp, time)};
}

template <typename T>
typename VectorCompositeFunctor<T>::ValueType
VectorCompositeFunctor<T>::evaluate(const ElemPointArg & elem_point_arg, const TimeArg & time) const
{
  return {
      _x_comp(elem_point_arg, time), _y_comp(elem_point_arg, time), _z_comp(elem_point_arg, time)};
}

template <typename T>
typename VectorCompositeFunctor<T>::GradientType
VectorCompositeFunctor<T>::evaluateGradient(const ElemArg & elem_arg, const TimeArg & time) const
{
  return {_x_comp.gradient(elem_arg, time),
          _y_comp.gradient(elem_arg, time),
          _z_comp.gradient(elem_arg, time)};
}
}
