//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MooseError.h"
#include "MooseFunctor.h"
#include "Moose.h"

#include <unordered_map>
#include <functional>

/**
 * A functor that compute the value of a functor at a fixed node, for every call
 */
template <typename T>
class FixedElemValueFunctor : public Moose::FunctorBase<T>
{
public:
  FixedElemValueFunctor(const std::string & name,
                        const Moose::FunctorEnvelope<T> & functor,
                        const std::set<ExecFlagType> & clearance_schedule,
                        const MooseMesh & mesh,
                        const unsigned int elem_id);

  virtual ~FixedElemValueFunctor() = default;

  bool isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                  const Elem * elem,
                                  const Moose::StateArg & time) const override;

  bool hasBlocks(SubdomainID /* id */) const override { return true; }
  bool hasFaceSide(const FaceInfo &, const bool) const override { return true; }

  using typename Moose::FunctorBase<T>::FunctorType;
  using typename Moose::FunctorBase<T>::ValueType;
  using typename Moose::FunctorBase<T>::FunctorReturnType;

protected:
  ValueType evaluate(const Moose::ElemArg & elem_arg, const Moose::StateArg & time) const override;
  ValueType evaluate(const Moose::FaceArg & face, const Moose::StateArg & time) const override;
  ValueType evaluate(const Moose::ElemQpArg & elem_qp, const Moose::StateArg & time) const override;
  ValueType evaluate(const Moose::ElemSideQpArg & elem_side_qp,
                     const Moose::StateArg & time) const override;
  ValueType evaluate(const Moose::ElemPointArg & elem_point,
                     const Moose::StateArg & time) const override;
  ValueType evaluate(const Moose::NodeArg & elem_point,
                     const Moose::StateArg & time) const override;

protected:
  /// Functor to evaluate to compute the elem value
  const Moose::Functor<T> & _functor;

  /// Elem argument to compute the functor
  ElemArg _elem_arg;
};

template <typename T>
FixedElemValueFunctor<T>::FixedElemValueFunctor(const std::string & name,
                                                const Moose::FunctorEnvelope<T> & functor,
                                                const std::set<ExecFlagType> & clearance_schedule,
                                                const MooseMesh & mesh,
                                                const unsigned int elem_id)
  : Moose::FunctorBase<T>(name, clearance_schedule), _functor(functor)
{
  const auto elem_ptr = mesh.elemPtr(elem_id);
  _elem_arg = {elem_ptr, false};
}

template <typename T>
bool
FixedElemValueFunctor<T>::isExtrapolatedBoundaryFace(const FaceInfo &,
                                                     const Elem *,
                                                     const Moose::StateArg &) const
{
  return false;
}

template <typename T>
typename FixedElemValueFunctor<T>::ValueType
FixedElemValueFunctor<T>::evaluate(const Moose::ElemArg & /*elem_arg*/,
                                   const Moose::StateArg & time) const
{
  return _functor(_elem_arg, time);
}

template <typename T>
typename FixedElemValueFunctor<T>::ValueType
FixedElemValueFunctor<T>::evaluate(const Moose::FaceArg & /*face*/,
                                   const Moose::StateArg & time) const
{
  return _functor(_elem_arg, time);
}

template <typename T>
typename FixedElemValueFunctor<T>::ValueType
FixedElemValueFunctor<T>::evaluate(const Moose::ElemQpArg & /*elem_qp*/,
                                   const Moose::StateArg & time) const
{
  return _functor(_elem_arg, time);
}

template <typename T>
typename FixedElemValueFunctor<T>::ValueType
FixedElemValueFunctor<T>::evaluate(const Moose::ElemSideQpArg & /*elem_side_qp*/,
                                   const Moose::StateArg & time) const
{
  return _functor(_elem_arg, time);
}

template <typename T>
typename FixedElemValueFunctor<T>::ValueType
FixedElemValueFunctor<T>::evaluate(const Moose::ElemPointArg & /*elem_point_arg*/,
                                   const Moose::StateArg & time) const
{
  return _functor(_elem_arg, time);
}

template <typename T>
typename FixedElemValueFunctor<T>::ValueType
FixedElemValueFunctor<T>::evaluate(const Moose::NodeArg & /*node_arg*/,
                                   const Moose::StateArg & time) const
{
  return _functor(_elem_arg, time);
}
