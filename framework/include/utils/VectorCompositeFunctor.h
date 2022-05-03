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

/**
 * A functor that returns a vector composed of its component functor evaluations
 */
template <typename T>
class VectorCompositeFunctor : public Moose::FunctorBase<VectorValue<T>>
{
public:
  template <typename U>
  using FunctorBase = Moose::FunctorBase<U>;

  using typename FunctorBase<VectorValue<T>>::ValueType;
  using typename FunctorBase<VectorValue<T>>::GradientType;
  using ElemArg = Moose::ElemArg;
  using ElemFromFaceArg = Moose::ElemFromFaceArg;
  using FaceArg = Moose::FaceArg;
  using SingleSidedFaceArg = Moose::SingleSidedFaceArg;
  using ElemQpArg = Moose::ElemQpArg;
  using ElemSideQpArg = Moose::ElemSideQpArg;

  VectorCompositeFunctor(const MooseFunctorName & name,
                         const FunctorBase<T> & x_comp,
                         const FunctorBase<T> & y_comp,
                         const FunctorBase<T> & z_comp)
    : Moose::FunctorBase<VectorValue<T>>(name), _x_comp(x_comp), _y_comp(y_comp), _z_comp(z_comp)
  {
  }

  VectorCompositeFunctor(const MooseFunctorName & name,
                         const FunctorBase<T> & x_comp,
                         const FunctorBase<T> & y_comp)
    : Moose::FunctorBase<VectorValue<T>>(name),
      _z_constant(std::make_unique<Moose::ConstantFunctor>(T(0))),
      _x_comp(x_comp),
      _y_comp(y_comp),
      _z_comp(*_z_constant)
  {
  }

  VectorCompositeFunctor(const MooseFunctorName & name, const FunctorBase<T> & x_comp)
    : Moose::FunctorBase<VectorValue<T>>(name),
      _y_constant(std::make_unique<Moose::ConstantFunctor>(T(0))),
      _z_constant(std::make_unique<Moose::ConstantFunctor>(T(0))),
      _x_comp(x_comp),
      _y_comp(*_y_constant),
      _z_comp(*_z_constant)
  {
  }

private:
  ValueType evaluate(const ElemArg & elem_arg, unsigned int state) const override final
  {
    return {_x_comp(elem_arg, state), _y_comp(elem_arg, state), _z_comp(elem_arg, state)};
  }

  ValueType evaluate(const ElemFromFaceArg & elem_from_face, unsigned int state) const override
  {
    return {_x_comp(elem_from_face, state),
            _y_comp(elem_from_face, state),
            _z_comp(elem_from_face, state)};
  }

  ValueType evaluate(const FaceArg & face, unsigned int state) const override final
  {
    return {_x_comp(face, state), _y_comp(face, state), _z_comp(face, state)};
  }

  ValueType evaluate(const SingleSidedFaceArg & ssf, unsigned int state) const override
  {
    return {_x_comp(ssf, state), _y_comp(ssf, state), _z_comp(ssf, state)};
  }

  ValueType evaluate(const ElemQpArg & elem_qp, unsigned int state) const override
  {
    return {_x_comp(elem_qp, state), _y_comp(elem_qp, state), _z_comp(elem_qp, state)};
  }

  ValueType evaluate(const ElemSideQpArg & elem_side_qp, unsigned int state) const override
  {
    return {
        _x_comp(elem_side_qp, state), _y_comp(elem_side_qp, state), _z_comp(elem_side_qp, state)};
  }

  using Moose::FunctorBase<VectorValue<T>>::evaluateGradient;
  GradientType evaluateGradient(const ElemFromFaceArg & elem_from_face,
                                unsigned int state) const override
  {
    return {_x_comp.gradient(elem_from_face, state),
            _y_comp.gradient(elem_from_face, state),
            _z_comp.gradient(elem_from_face, state)};
  }

  /// Possible holder of constant-0 y-component functor. This will be allocated if the user only
  /// supplies one component functor during construction
  std::unique_ptr<Moose::ConstantFunctor<T>> _y_constant;

  /// Possible holder of constant-0 z-component functor. This will be allocated if the user only
  /// supplies two component functors during construction
  std::unique_ptr<Moose::ConstantFunctor<T>> _z_constant;

  /// The x-component functor
  const Moose::FunctorBase<T> & _x_comp;
  /// The y-component functor
  const Moose::FunctorBase<T> & _y_comp;
  /// The z-component functor
  const Moose::FunctorBase<T> & _z_comp;
};
