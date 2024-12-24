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
  using typename FunctorBase<VectorValue<T>>::DotType;

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

  virtual bool hasBlocks(SubdomainID sub_id) const override
  {
    const bool ret = _x_comp.hasBlocks(sub_id);
    if (_has_y)
      mooseAssert(ret == _y_comp.hasBlocks(sub_id), "x and y block restriction don't agree");
    if (_has_z)
      mooseAssert(ret == _z_comp.hasBlocks(sub_id), "x and z block restriction don't agree");
    return ret;
  }

  bool supportsFaceArg() const override;
  bool supportsElemSideQpArg() const override;
  bool mayRequireGhosting() const override;

private:
  ValueType evaluate(const ElemArg & elem_arg, const StateArg & state) const override;
  ValueType evaluate(const FaceArg & face, const StateArg & state) const override;
  ValueType evaluate(const ElemQpArg & elem_qp, const StateArg & state) const override;
  ValueType evaluate(const ElemSideQpArg & elem_side_qp, const StateArg & state) const override;
  ValueType evaluate(const ElemPointArg & elem_point_arg, const StateArg & state) const override;
  ValueType evaluate(const NodeArg & node_arg, const StateArg & state) const override;

  using FunctorBase<VectorValue<T>>::evaluateGradient;
  GradientType evaluateGradient(const ElemArg & elem_arg, const StateArg & state) const override;

  using FunctorBase<VectorValue<T>>::evaluateDot;
  DotType evaluateDot(const FaceArg & face_arg, const StateArg & state) const override;
  DotType evaluateDot(const ElemArg & elem_arg, const StateArg & state) const override;

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

  /// Whether the user supplied a y-functor
  const bool _has_y;

  /// Whether the user supplied a z-functor
  const bool _has_z;
};

template <typename T>
VectorCompositeFunctor<T>::VectorCompositeFunctor(const MooseFunctorName & name,
                                                  const FunctorBase<T> & x_comp,
                                                  const FunctorBase<T> & y_comp,
                                                  const FunctorBase<T> & z_comp)
  : FunctorBase<VectorValue<T>>(name),
    _x_comp(x_comp),
    _y_comp(y_comp),
    _z_comp(z_comp),
    _has_y(true),
    _has_z(true)
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
    _z_comp(*_z_constant),
    _has_y(true),
    _has_z(false)
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
    _z_comp(*_z_constant),
    _has_y(false),
    _has_z(false)
{
}

template <typename T>
bool
VectorCompositeFunctor<T>::supportsFaceArg() const
{
  if (!_x_comp.supportsFaceArg())
    return false;
  if (_has_y && !_y_comp.supportsFaceArg())
    return false;
  if (_has_z && !_z_comp.supportsFaceArg())
    return false;
  return true;
}

template <typename T>
bool
VectorCompositeFunctor<T>::supportsElemSideQpArg() const
{
  if (!_x_comp.supportsElemSideQpArg())
    return false;
  if (_has_y && !_y_comp.supportsElemSideQpArg())
    return false;
  if (_has_z && !_z_comp.supportsElemSideQpArg())
    return false;
  return true;
}

template <typename T>
bool
VectorCompositeFunctor<T>::mayRequireGhosting() const
{
  if (!_x_comp.mayRequireGhosting() && (!_has_y || !_y_comp.mayRequireGhosting()) &&
      (!_has_z || !_z_comp.mayRequireGhosting()))
    return false;
  else
    return true;
}

template <typename T>
typename VectorCompositeFunctor<T>::ValueType
VectorCompositeFunctor<T>::evaluate(const ElemArg & elem_arg, const StateArg & state) const
{
  return {_x_comp(elem_arg, state), _y_comp(elem_arg, state), _z_comp(elem_arg, state)};
}

template <typename T>
typename VectorCompositeFunctor<T>::ValueType
VectorCompositeFunctor<T>::evaluate(const FaceArg & face, const StateArg & state) const
{
  return {_x_comp(face, state), _y_comp(face, state), _z_comp(face, state)};
}

template <typename T>
typename VectorCompositeFunctor<T>::ValueType
VectorCompositeFunctor<T>::evaluate(const ElemQpArg & elem_qp, const StateArg & state) const
{
  return {_x_comp(elem_qp, state), _y_comp(elem_qp, state), _z_comp(elem_qp, state)};
}

template <typename T>
typename VectorCompositeFunctor<T>::ValueType
VectorCompositeFunctor<T>::evaluate(const ElemSideQpArg & elem_side_qp,
                                    const StateArg & state) const
{
  return {_x_comp(elem_side_qp, state), _y_comp(elem_side_qp, state), _z_comp(elem_side_qp, state)};
}

template <typename T>
typename VectorCompositeFunctor<T>::ValueType
VectorCompositeFunctor<T>::evaluate(const ElemPointArg & elem_point_arg,
                                    const StateArg & state) const
{
  return {_x_comp(elem_point_arg, state),
          _y_comp(elem_point_arg, state),
          _z_comp(elem_point_arg, state)};
}

template <typename T>
typename VectorCompositeFunctor<T>::ValueType
VectorCompositeFunctor<T>::evaluate(const NodeArg & node_arg, const StateArg & state) const
{
  return {_x_comp(node_arg, state), _y_comp(node_arg, state), _z_comp(node_arg, state)};
}

template <typename T>
typename VectorCompositeFunctor<T>::GradientType
VectorCompositeFunctor<T>::evaluateGradient(const ElemArg & elem_arg, const StateArg & state) const
{
  return {_x_comp.gradient(elem_arg, state),
          _y_comp.gradient(elem_arg, state),
          _z_comp.gradient(elem_arg, state)};
}

template <typename T>
typename VectorCompositeFunctor<T>::DotType
VectorCompositeFunctor<T>::evaluateDot(const FaceArg & face_arg, const StateArg & state) const
{
  return {_x_comp.dot(face_arg, state), _y_comp.dot(face_arg, state), _z_comp.dot(face_arg, state)};
}

template <typename T>
typename VectorCompositeFunctor<T>::DotType
VectorCompositeFunctor<T>::evaluateDot(const ElemArg & elem_arg, const StateArg & state) const
{
  return {_x_comp.dot(elem_arg, state), _y_comp.dot(elem_arg, state), _z_comp.dot(elem_arg, state)};
}
}
