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
class FixedNodeValueFunctor : public Moose::FunctorBase<T>
{
public:
  FixedNodeValueFunctor(const std::string & name,
                        const Moose::FunctorEnvelope<T> & functor,
                        const std::set<ExecFlagType> & clearance_schedule,
                        const Node * const node,
                        const SubdomainID sub_id);

  virtual ~FixedNodeValueFunctor() = default;

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
  /// Functor to evaluate to compute the node value
  const Moose::Functor<T> & _functor;

  /// Node to evaluate on. This is fixed, set at construction
  const Node * const _node;

  /// Nodal argument to compute the functor
  NodeArg _node_arg;
};

template <typename T>
FixedNodeValueFunctor<T>::FixedNodeValueFunctor(const std::string & name,
                                                const Moose::FunctorEnvelope<T> & functor,
                                                const std::set<ExecFlagType> & clearance_schedule,
                                                const Node * const node,
                                                const SubdomainID sub_id)
  : Moose::FunctorBase<T>(name, clearance_schedule), _functor(functor), _node(node)
{
  _node_arg = {_node, sub_id};
}

template <typename T>
bool
FixedNodeValueFunctor<T>::isExtrapolatedBoundaryFace(const FaceInfo &,
                                                     const Elem *,
                                                     const Moose::StateArg &) const
{
  return false;
}

template <typename T>
typename FixedNodeValueFunctor<T>::ValueType
FixedNodeValueFunctor<T>::evaluate(const Moose::ElemArg & /*elem_arg*/,
                                   const Moose::StateArg & time) const
{
  return _functor(_node_arg, time);
}

template <typename T>
typename FixedNodeValueFunctor<T>::ValueType
FixedNodeValueFunctor<T>::evaluate(const Moose::FaceArg & /*face*/,
                                   const Moose::StateArg & time) const
{
  return _functor(_node_arg, time);
}

template <typename T>
typename FixedNodeValueFunctor<T>::ValueType
FixedNodeValueFunctor<T>::evaluate(const Moose::ElemQpArg & /*elem_qp*/,
                                   const Moose::StateArg & time) const
{
  return _functor(_node_arg, time);
}

template <typename T>
typename FixedNodeValueFunctor<T>::ValueType
FixedNodeValueFunctor<T>::evaluate(const Moose::ElemSideQpArg & /*elem_side_qp*/,
                                   const Moose::StateArg & time) const
{
  return _functor(_node_arg, time);
}

template <typename T>
typename FixedNodeValueFunctor<T>::ValueType
FixedNodeValueFunctor<T>::evaluate(const Moose::ElemPointArg & /*elem_point_arg*/,
                                   const Moose::StateArg & time) const
{
  return _functor(_node_arg, time);
}

template <typename T>
typename FixedNodeValueFunctor<T>::ValueType
FixedNodeValueFunctor<T>::evaluate(const Moose::NodeArg & /*node_arg*/,
                                   const Moose::StateArg & time) const
{
  return _functor(_node_arg, time);
}
