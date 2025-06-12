//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseFunctor.h"
#include "InputParameters.h"
#include "BlockRestrictable.h"

/**
 * Base class for creating a user object with the SpatialUserObject and Moose::Functor APIs
 */
template <typename UserObjectType>
class SpatialUserObjectFunctor : public UserObjectType, public Moose::FunctorBase<Real>
{
public:
  static InputParameters validParams();

  SpatialUserObjectFunctor(const InputParameters & params);
  virtual bool hasBlocks(SubdomainID sub) const override;

protected:
  using ElemArg = Moose::ElemArg;
  using ElemQpArg = Moose::ElemQpArg;
  using ElemSideQpArg = Moose::ElemSideQpArg;
  using FaceArg = Moose::FaceArg;
  using ElemPointArg = Moose::ElemPointArg;
  using NodeArg = Moose::NodeArg;

  virtual Real evaluate(const ElemArg & elem, const Moose::StateArg & state) const override final;
  virtual Real evaluate(const FaceArg & face, const Moose::StateArg & state) const override final;
  virtual Real evaluate(const ElemQpArg & qp, const Moose::StateArg & state) const override final;
  virtual Real evaluate(const ElemSideQpArg & elem_side_qp,
                        const Moose::StateArg & state) const override final;
  virtual Real evaluate(const ElemPointArg & elem_point,
                        const Moose::StateArg & state) const override final;
  virtual Real evaluate(const NodeArg & node, const Moose::StateArg & state) const override final;

  virtual bool supportsFaceArg() const override final { return true; }
  virtual bool supportsElemSideQpArg() const override final { return true; }

private:
  /*
   * Helper template implementing functor evaluations
   */
  template <typename SpatialArg>
  Real evaluateTemplate(const SpatialArg & position, const Moose::StateArg & state) const;
};

template <typename UserObjectType>
InputParameters
SpatialUserObjectFunctor<UserObjectType>::validParams()
{
  auto params = UserObjectType::validParams();
  // Spatial UOs are used in the transfers
  ExecFlagEnum & exec_enum = params.template set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_TRANSFER);
  return params;
}

template <typename UserObjectType>
SpatialUserObjectFunctor<UserObjectType>::SpatialUserObjectFunctor(const InputParameters & params)
  : UserObjectType(params), Moose::FunctorBase<Real>(this->name())
{
}

template <typename UserObjectType>
template <typename SpatialArg>
Real
SpatialUserObjectFunctor<UserObjectType>::evaluateTemplate(
    const SpatialArg & position, const Moose::StateArg & libmesh_dbg_var(state)) const
{
  mooseAssert(state.state == 0, "We do not currently support evaluating at old states");
  return this->spatialValue(position.getPoint());
}

template <typename UserObjectType>
Real
SpatialUserObjectFunctor<UserObjectType>::evaluate(const ElemArg & elem,
                                                   const Moose::StateArg & state) const
{
  return evaluateTemplate(elem, state);
}

template <typename UserObjectType>
Real
SpatialUserObjectFunctor<UserObjectType>::evaluate(const FaceArg & face,
                                                   const Moose::StateArg & state) const
{
  return evaluateTemplate(face, state);
}

template <typename UserObjectType>
Real
SpatialUserObjectFunctor<UserObjectType>::evaluate(const ElemQpArg & qp,
                                                   const Moose::StateArg & state) const
{
  return evaluateTemplate(qp, state);
}

template <typename UserObjectType>
Real
SpatialUserObjectFunctor<UserObjectType>::evaluate(const ElemSideQpArg & elem_side_qp,
                                                   const Moose::StateArg & state) const
{
  return evaluateTemplate(elem_side_qp, state);
}

template <typename UserObjectType>
Real
SpatialUserObjectFunctor<UserObjectType>::evaluate(const ElemPointArg & elem_point,
                                                   const Moose::StateArg & state) const
{
  return evaluateTemplate(elem_point, state);
}

template <typename UserObjectType>
Real
SpatialUserObjectFunctor<UserObjectType>::evaluate(const NodeArg & node,
                                                   const Moose::StateArg & state) const
{
  return evaluateTemplate(node, state);
}

template <typename UserObjectType>
bool
SpatialUserObjectFunctor<UserObjectType>::hasBlocks(const SubdomainID sub_id) const
{
  if constexpr (std::is_base_of<BlockRestrictable, UserObjectType>::value)
    return UserObjectType::hasBlocks(sub_id);
  else
    return Moose::FunctorBase<Real>::hasBlocks(sub_id);
}
