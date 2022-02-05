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

  bool isExtrapolatedBoundaryFace(const FaceInfo & fi) const override;

  using typename Moose::FunctorBase<T>::FunctorType;
  using typename Moose::FunctorBase<T>::ValueType;
  using typename Moose::FunctorBase<T>::DotType;
  using typename Moose::FunctorBase<T>::GradientType;
  using typename Moose::FunctorBase<T>::FunctorReturnType;

protected:
  using ElemFn = std::function<T(const Moose::ElemArg &, const unsigned int &)>;
  using ElemAndFaceFn = std::function<T(const Moose::ElemFromFaceArg &, const unsigned int &)>;
  using FaceFn = std::function<T(const Moose::SingleSidedFaceArg &, const unsigned int &)>;
  using ElemQpFn = std::function<T(const Moose::ElemQpArg &, const unsigned int &)>;
  using ElemSideQpFn = std::function<T(const Moose::ElemSideQpArg &, const unsigned int &)>;

  ValueType evaluate(const Moose::ElemArg & elem_arg, unsigned int state) const override;
  ValueType evaluate(const Moose::ElemFromFaceArg & elem_from_face,
                     unsigned int state) const override;
  ValueType evaluate(const Moose::FaceArg & face, unsigned int state) const override;
  ValueType evaluate(const Moose::SingleSidedFaceArg & face, unsigned int state) const override;
  ValueType evaluate(const Moose::ElemQpArg & elem_qp, unsigned int state) const override;
  ValueType evaluate(const Moose::ElemSideQpArg & elem_side_qp, unsigned int state) const override;

  using Moose::FunctorBase<T>::evaluateGradient;
  GradientType evaluateGradient(const Moose::ElemArg & elem_arg, unsigned int) const override;

private:
  /**
   * Provide a useful error message about lack of functor material property on the provided
   * subdomain \p sub_id
   */
  void subdomainErrorMessage(SubdomainID sub_id) const;

  /// Functors that return element average values (or cell centroid values or whatever the
  /// implementer wants to return for a given element argument)
  std::unordered_map<SubdomainID, ElemFn> _elem_functor;

  /// Functors that return the value on the requested element that will perform any necessary
  /// ghosting operations if this object is not technically defined on the requested subdomain
  std::unordered_map<SubdomainID, ElemAndFaceFn> _elem_from_face_functor;

  /// Functors that return the property value on the requested side of the face (e.g. the
  /// infinitesimal + or - side of the face)
  std::unordered_map<SubdomainID, FaceFn> _face_functor;

  /// Functors that will evaluate elements at quadrature points
  std::unordered_map<SubdomainID, ElemQpFn> _elem_qp_functor;

  /// Functors that will evaluate elements at side quadrature points
  std::unordered_map<SubdomainID, ElemSideQpFn> _elem_side_qp_functor;

  /// The name of this object
  std::string _name;

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
  : Moose::FunctorBase<T>(clearance_schedule), _name(name), _mesh(mesh)
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
                 _name,
                 "' for block id ",
                 block_id,
                 ". Another material must already declare this property on that block.");
    _elem_from_face_functor.emplace(block_id, my_lammy);
    _face_functor.emplace(block_id, my_lammy);
    _elem_qp_functor.emplace(block_id, my_lammy);
    _elem_side_qp_functor.emplace(block_id, my_lammy);
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
PiecewiseByBlockLambdaFunctor<T>::isExtrapolatedBoundaryFace(const FaceInfo & fi) const
{
  if (!fi.neighborPtr())
    return true;

  return (_elem_functor.count(fi.elem().subdomain_id()) +
          _elem_functor.count(fi.neighbor().subdomain_id())) == 1;
}

template <typename T>
void
PiecewiseByBlockLambdaFunctor<T>::subdomainErrorMessage(const SubdomainID sub_id) const
{
  mooseError("The provided subdomain ID ",
             std::to_string(sub_id),
             " doesn't exist in the map for lambda functor '",
             _name,
             "'! This is likely because you did not provide a functor material "
             "definition on that subdomain");
}

template <typename T>
typename PiecewiseByBlockLambdaFunctor<T>::ValueType
PiecewiseByBlockLambdaFunctor<T>::evaluate(const Moose::ElemArg & elem_arg,
                                           unsigned int state) const
{
  const Elem * const elem = elem_arg.elem;
  mooseAssert(elem && elem != libMesh::remote_elem,
              "The element must be non-null and non-remote in functor material properties");
  auto it = _elem_functor.find(elem->subdomain_id());
  if (it == _elem_functor.end())
    subdomainErrorMessage(elem->subdomain_id());

  return it->second(elem_arg, state);
}

template <typename T>
typename PiecewiseByBlockLambdaFunctor<T>::ValueType
PiecewiseByBlockLambdaFunctor<T>::evaluate(const Moose::ElemFromFaceArg & elem_from_face,
                                           unsigned int state) const
{
  mooseAssert((elem_from_face.elem && elem_from_face.elem != libMesh::remote_elem) ||
                  elem_from_face.fi,
              "The element must be non-null and non-remote or the face must be non-null in functor "
              "material properties");
  auto it = _elem_from_face_functor.find(elem_from_face.sub_id);
  if (it == _elem_from_face_functor.end())
    subdomainErrorMessage(elem_from_face.sub_id);

  return it->second(elem_from_face, state);
}

template <typename T>
typename PiecewiseByBlockLambdaFunctor<T>::ValueType
PiecewiseByBlockLambdaFunctor<T>::evaluate(const Moose::SingleSidedFaceArg & face,
                                           unsigned int state) const
{
  auto it = _face_functor.find(face.sub_id);
  if (it == _face_functor.end())
    subdomainErrorMessage(face.sub_id);

  return it->second(face, state);
}

template <typename T>
typename PiecewiseByBlockLambdaFunctor<T>::ValueType
PiecewiseByBlockLambdaFunctor<T>::evaluate(const Moose::FaceArg & face, unsigned int state) const
{
  using namespace Moose::FV;

  mooseAssert(face.fi,
              "We must have a non-null face_info in order to prepare our ElemFromFace tuples");
  static const typename libMesh::TensorTools::IncrementRank<T>::type example_gradient(0);

  switch (face.limiter_type)
  {
    case LimiterType::CentralDifference:
    case LimiterType::Upwind:
    {
      const auto elem_from_face = face.elemFromFace();
      const auto neighbor_from_face = face.neighborFromFace();
      const auto & upwind_elem = face.elem_is_upwind ? elem_from_face : neighbor_from_face;
      const auto & downwind_elem = face.elem_is_upwind ? neighbor_from_face : elem_from_face;
      auto limiter_obj =
          Limiter<typename LimiterValueType<T>::value_type>::build(face.limiter_type);
      return interpolate(*limiter_obj,
                         evaluate(upwind_elem, state),
                         evaluate(downwind_elem, state),
                         &example_gradient,
                         *face.fi,
                         face.elem_is_upwind);
    }

    default:
      mooseError("Unsupported limiter type in PiecewiseByBlockLambdaFunctor::evaluate(FaceArg)");
  }
}

template <typename T>
typename PiecewiseByBlockLambdaFunctor<T>::ValueType
PiecewiseByBlockLambdaFunctor<T>::evaluate(const Moose::ElemQpArg & elem_qp,
                                           unsigned int state) const
{
  const auto sub_id = std::get<0>(elem_qp)->subdomain_id();
  auto it = _elem_qp_functor.find(sub_id);
  if (it == _elem_qp_functor.end())
    subdomainErrorMessage(sub_id);

  return it->second(elem_qp, state);
}

template <typename T>
typename PiecewiseByBlockLambdaFunctor<T>::ValueType
PiecewiseByBlockLambdaFunctor<T>::evaluate(const Moose::ElemSideQpArg & elem_side_qp,
                                           unsigned int state) const
{
  const auto sub_id = std::get<0>(elem_side_qp)->subdomain_id();
  auto it = _elem_side_qp_functor.find(sub_id);
  if (it == _elem_side_qp_functor.end())
    subdomainErrorMessage(sub_id);

  return it->second(elem_side_qp, state);
}

template <typename T>
typename PiecewiseByBlockLambdaFunctor<T>::GradientType
PiecewiseByBlockLambdaFunctor<T>::evaluateGradient(const Moose::ElemArg & elem_arg,
                                                   unsigned int) const
{
  return Moose::FV::greenGaussGradient(elem_arg, *this, true, _mesh);
}
