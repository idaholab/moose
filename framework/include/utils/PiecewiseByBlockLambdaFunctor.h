//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseError.h"
#include "MooseFunctor.h"
#include "Moose.h"
#include "Limiter.h"
#include "MathFVUtils.h"
#include "GreenGaussGradient.h"

#include "libmesh/elem.h"
#include "libmesh/remote_elem.h"
#include "libmesh/tensor_tools.h"

#include <unordered_map>
#include <functional>

/**
 * A material property that is evaluated on-the-fly via calls to various overloads of \p operator()
 */
template <typename T>
class PiecewiseByBlockLambdaFunctor : public Moose::FunctorBase<T>
{
public:
  template <typename PolymorphicLambda>
  PiecewiseByBlockLambdaFunctor(const std::string & name,
                                PolymorphicLambda my_lammy,
                                const std::set<ExecFlagType> & clearance_schedule,
                                const MooseMesh & mesh,
                                const std::set<SubdomainID> & block_ids);

  /**
   * Set the functor that will be used in calls to \p evaluate overloads
   * @param mesh The mesh that the functor is defined on
   * @param block_ids The block/subdomain IDs that the user-provided functor is valid for
   * @param my_lammy The functor that defines how this object is evaluated
   */
  template <typename PolymorphicLambda>
  void setFunctor(const MooseMesh & mesh,
                  const std::set<SubdomainID> & block_ids,
                  PolymorphicLambda my_lammy);

  virtual ~PiecewiseByBlockLambdaFunctor() = default;

  bool isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                  const Elem * elem,
                                  const Moose::StateArg & time) const override;

  bool hasBlocks(SubdomainID id) const override;

  using typename Moose::FunctorBase<T>::FunctorType;
  using typename Moose::FunctorBase<T>::ValueType;
  using typename Moose::FunctorBase<T>::DotType;
  using typename Moose::FunctorBase<T>::GradientType;
  using typename Moose::FunctorBase<T>::FunctorReturnType;

protected:
  using ElemFn = std::function<T(const Moose::ElemArg &, const Moose::StateArg &)>;
  using FaceFn = std::function<T(const Moose::FaceArg &, const Moose::StateArg &)>;
  using ElemQpFn = std::function<T(const Moose::ElemQpArg &, const Moose::StateArg &)>;
  using ElemSideQpFn = std::function<T(const Moose::ElemSideQpArg &, const Moose::StateArg &)>;
  using ElemPointFn = std::function<T(const Moose::ElemPointArg &, const Moose::StateArg &)>;

  ValueType evaluate(const Moose::ElemArg & elem_arg, const Moose::StateArg & time) const override;
  ValueType evaluate(const Moose::FaceArg & face, const Moose::StateArg & time) const override;
  ValueType evaluate(const Moose::ElemQpArg & elem_qp, const Moose::StateArg & time) const override;
  ValueType evaluate(const Moose::ElemSideQpArg & elem_side_qp,
                     const Moose::StateArg & time) const override;
  ValueType evaluate(const Moose::ElemPointArg & elem_point,
                     const Moose::StateArg & time) const override;

  using Moose::FunctorBase<T>::evaluateGradient;
  GradientType evaluateGradient(const Moose::ElemArg & elem_arg,
                                const Moose::StateArg &) const override;
  GradientType evaluateGradient(const Moose::FaceArg & face_arg,
                                const Moose::StateArg &) const override;

private:
  /**
   * Provide a useful error message about lack of functor material property on the provided
   * subdomain \p sub_id
   */
  void subdomainErrorMessage(SubdomainID sub_id) const;

  /// Functors that return element average values (or cell centroid values or whatever the
  /// implementer wants to return for a given element argument)
  std::unordered_map<SubdomainID, ElemFn> _elem_functor;

  /// Functors that return the property value on the requested side of the face (e.g. the
  /// infinitesimal + or - side of the face)
  std::unordered_map<SubdomainID, FaceFn> _face_functor;

  /// Functors that will evaluate elements at quadrature points
  std::unordered_map<SubdomainID, ElemQpFn> _elem_qp_functor;

  /// Functors that will evaluate elements at side quadrature points
  std::unordered_map<SubdomainID, ElemSideQpFn> _elem_side_qp_functor;

  /// Functors that return evaluations at an arbitrary physical point in an element
  std::unordered_map<SubdomainID, ElemPointFn> _elem_point_functor;

  /// The mesh that this functor operates on
  const MooseMesh & _mesh;
};

template <typename T>
template <typename PolymorphicLambda>
PiecewiseByBlockLambdaFunctor<T>::PiecewiseByBlockLambdaFunctor(
    const std::string & name,
    PolymorphicLambda my_lammy,
    const std::set<ExecFlagType> & clearance_schedule,
    const MooseMesh & mesh,
    const std::set<SubdomainID> & block_ids)
  : Moose::FunctorBase<T>(name, clearance_schedule), _mesh(mesh)
{
  setFunctor(mesh, block_ids, my_lammy);
}

template <typename T>
template <typename PolymorphicLambda>
void
PiecewiseByBlockLambdaFunctor<T>::setFunctor(const MooseMesh & mesh,
                                             const std::set<SubdomainID> & block_ids,
                                             PolymorphicLambda my_lammy)
{
  mooseAssert(&mesh == &_mesh,
              "We should always be setting this functor with the same mesh. We may relax this "
              "assertion later");

  auto add_lammy = [this, my_lammy](const SubdomainID block_id)
  {
    auto pr = _elem_functor.emplace(block_id, my_lammy);
    if (!pr.second)
      mooseError("No insertion for the functor material property '",
                 this->functorName(),
                 "' for block id ",
                 block_id,
                 ". Another material must already declare this property on that block.");
    _face_functor.emplace(block_id, my_lammy);
    _elem_qp_functor.emplace(block_id, my_lammy);
    _elem_side_qp_functor.emplace(block_id, my_lammy);
    _elem_point_functor.emplace(block_id, my_lammy);
  };

  for (const auto block_id : block_ids)
  {
    if (block_id == Moose::ANY_BLOCK_ID)
    {
      const auto & inner_block_ids = mesh.meshSubdomains();
      for (const auto inner_block_id : inner_block_ids)
        add_lammy(inner_block_id);
    }
    else
      add_lammy(block_id);
  }
}

template <typename T>
bool
PiecewiseByBlockLambdaFunctor<T>::isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                                             const Elem *,
                                                             const Moose::StateArg &) const
{
  if (!fi.neighborPtr())
    return true;

  const bool defined_on_elem = _elem_functor.count(fi.elem().subdomain_id());
  const bool defined_on_neighbor = _elem_functor.count(fi.neighbor().subdomain_id());
  const bool extrapolated = (defined_on_elem + defined_on_neighbor) == 1;

  mooseAssert(defined_on_elem || defined_on_neighbor,
              "This shouldn't be called if we aren't defined on either side.");
  return extrapolated;
}

template <typename T>
bool
PiecewiseByBlockLambdaFunctor<T>::hasBlocks(const SubdomainID id) const
{
  // If any of the maps has a functor for that block, it has the block
  const bool has_blocks = _elem_functor.count(id);
  mooseAssert(has_blocks == _face_functor.count(id),
              "All functor sets should agree on whether we have this sub id");
  mooseAssert(has_blocks == _elem_qp_functor.count(id),
              "All functor sets should agree on whether we have this sub id");
  mooseAssert(has_blocks == _elem_side_qp_functor.count(id),
              "All functor sets should agree on whether we have this sub id");
  return has_blocks;
}

template <typename T>
void
PiecewiseByBlockLambdaFunctor<T>::subdomainErrorMessage(const SubdomainID sub_id) const
{
  mooseError("The provided subdomain ID ",
             std::to_string(sub_id),
             " doesn't exist in the map for lambda functor '",
             this->functorName(),
             "'! This is likely because you did not provide a functor material "
             "definition on that subdomain");
}

template <typename T>
typename PiecewiseByBlockLambdaFunctor<T>::ValueType
PiecewiseByBlockLambdaFunctor<T>::evaluate(const Moose::ElemArg & elem_arg,
                                           const Moose::StateArg & time) const
{
  const Elem * const elem = elem_arg.elem;
  mooseAssert(elem && elem != libMesh::remote_elem,
              "The element must be non-null and non-remote in functor material properties");
  auto it = _elem_functor.find(elem->subdomain_id());
  if (it == _elem_functor.end())
    subdomainErrorMessage(elem->subdomain_id());

  return it->second(elem_arg, time);
}

template <typename T>
typename PiecewiseByBlockLambdaFunctor<T>::ValueType
PiecewiseByBlockLambdaFunctor<T>::evaluate(const Moose::FaceArg & face,
                                           const Moose::StateArg & time) const
{
  using namespace Moose::FV;

  if (face.face_side)
  {
    const auto sub_id = face.face_side->subdomain_id();
    auto it = _face_functor.find(sub_id);
    if (it == _face_functor.end())
      subdomainErrorMessage(sub_id);

    return it->second(face, time);
  }

  mooseAssert(this->isInternalFace(*face.fi),
              "If we did not have a face side, then we must be an internal face");
  return interpolate(*this, face, time);
}

template <typename T>
typename PiecewiseByBlockLambdaFunctor<T>::ValueType
PiecewiseByBlockLambdaFunctor<T>::evaluate(const Moose::ElemQpArg & elem_qp,
                                           const Moose::StateArg & time) const
{
  const auto sub_id = std::get<0>(elem_qp)->subdomain_id();
  auto it = _elem_qp_functor.find(sub_id);
  if (it == _elem_qp_functor.end())
    subdomainErrorMessage(sub_id);

  return it->second(elem_qp, time);
}

template <typename T>
typename PiecewiseByBlockLambdaFunctor<T>::ValueType
PiecewiseByBlockLambdaFunctor<T>::evaluate(const Moose::ElemSideQpArg & elem_side_qp,
                                           const Moose::StateArg & time) const
{
  const auto sub_id = std::get<0>(elem_side_qp)->subdomain_id();
  auto it = _elem_side_qp_functor.find(sub_id);
  if (it == _elem_side_qp_functor.end())
    subdomainErrorMessage(sub_id);

  return it->second(elem_side_qp, time);
}

template <typename T>
typename PiecewiseByBlockLambdaFunctor<T>::ValueType
PiecewiseByBlockLambdaFunctor<T>::evaluate(const Moose::ElemPointArg & elem_point_arg,
                                           const Moose::StateArg & time) const
{
  const Elem * const elem = elem_point_arg.elem;
  mooseAssert(elem && elem != libMesh::remote_elem,
              "The element must be non-null and non-remote in functor material properties");
  auto it = _elem_point_functor.find(elem->subdomain_id());
  if (it == _elem_point_functor.end())
    subdomainErrorMessage(elem->subdomain_id());

  return it->second(elem_point_arg, time);
}

template <typename T>
typename PiecewiseByBlockLambdaFunctor<T>::GradientType
PiecewiseByBlockLambdaFunctor<T>::evaluateGradient(const Moose::ElemArg & elem_arg,
                                                   const Moose::StateArg & time) const
{
  return Moose::FV::greenGaussGradient(elem_arg, time, *this, true, _mesh);
}

template <typename T>
typename PiecewiseByBlockLambdaFunctor<T>::GradientType
PiecewiseByBlockLambdaFunctor<T>::evaluateGradient(const Moose::FaceArg & face_arg,
                                                   const Moose::StateArg & time) const
{
  return Moose::FV::greenGaussGradient(face_arg, time, *this, true, _mesh);
}
