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

#include "libmesh/elem.h"
#include "libmesh/remote_elem.h"
#include "libmesh/tensor_tools.h"

#include <unordered_map>
#include <functional>

/**
 * A material property that is evaluated on-the-fly via calls to various overloads of \p operator()
 */
template <typename T>
class FunctorMaterialPropertyImpl : public Moose::Functor<T>
{
public:
  FunctorMaterialPropertyImpl(const std::string & name) : _name(name) {}

  virtual ~FunctorMaterialPropertyImpl() = default;

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

  using typename Moose::Functor<T>::FaceArg;
  using typename Moose::Functor<T>::SingleSidedFaceArg;
  using typename Moose::Functor<T>::ElemFromFaceArg;
  using typename Moose::Functor<T>::ElemQpArg;
  using typename Moose::Functor<T>::ElemSideQpArg;
  using typename Moose::Functor<T>::FunctorType;
  using typename Moose::Functor<T>::ValueType;
  using typename Moose::Functor<T>::DotType;
  using typename Moose::Functor<T>::GradientType;
  using typename Moose::Functor<T>::FunctorReturnType;

protected:
  using ElemFn = std::function<T(const Elem * const &, const unsigned int &)>;
  using ElemAndFaceFn = std::function<T(const ElemFromFaceArg &, const unsigned int &)>;
  using FaceFn = std::function<T(const SingleSidedFaceArg &, const unsigned int &)>;
  using ElemQpFn = std::function<T(const ElemQpArg &, const unsigned int &)>;
  using ElemSideQpFn = std::function<T(const ElemSideQpArg &, const unsigned int &)>;
  using TQpFn = std::function<T(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> &,
                                const unsigned int &)>;

  ValueType evaluate(const Elem * const & elem, unsigned int state) const override;
  ValueType evaluate(const ElemFromFaceArg & elem_from_face, unsigned int state) const override;
  ValueType evaluate(const FaceArg & face, unsigned int state) const override;
  ValueType evaluate(const SingleSidedFaceArg & face, unsigned int state) const override;
  ValueType evaluate(const ElemQpArg & elem_qp, unsigned int state) const override;
  ValueType evaluate(const ElemSideQpArg & elem_side_qp, unsigned int state) const override;
  ValueType evaluate(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp,
                     unsigned int state) const override;

  GradientType evaluateGradient(const Elem * const & elem, unsigned int state) const override;
  GradientType evaluateGradient(const ElemFromFaceArg & elem_from_face,
                                unsigned int state) const override;
  GradientType evaluateGradient(const FaceArg & face, unsigned int state) const override;
  GradientType evaluateGradient(const SingleSidedFaceArg & face, unsigned int state) const override;
  GradientType evaluateGradient(const ElemQpArg & elem_qp, unsigned int state) const override;
  GradientType evaluateGradient(const ElemSideQpArg & elem_side_qp,
                                unsigned int state) const override;
  GradientType
  evaluateGradient(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp,
                   unsigned int state) const override;

  DotType evaluateDot(const Elem * const & elem, unsigned int state) const override;
  DotType evaluateDot(const ElemFromFaceArg & elem_from_face, unsigned int state) const override;
  DotType evaluateDot(const FaceArg & face, unsigned int state) const override;
  DotType evaluateDot(const SingleSidedFaceArg & face, unsigned int state) const override;
  DotType evaluateDot(const ElemQpArg & elem_qp, unsigned int state) const override;
  DotType evaluateDot(const ElemSideQpArg & elem_side_qp, unsigned int state) const override;
  DotType evaluateDot(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp,
                      unsigned int state) const override;

private:
  /**
   * Provide a useful error message about lack of functor material property on the provided
   * subdomain \p sub_id
   */
  std::string subdomainErrorMessage(SubdomainID sub_id) const;

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

  /// Functors that will index elemental, neighbor, or lower-dimensional data at a provided
  /// quadrature point index
  std::unordered_map<SubdomainID, TQpFn> _tqp_functor;

  /// The name of this object
  std::string _name;

  template <typename>
  friend class FunctorMaterialProperty;
};

template <typename T>
template <typename PolymorphicLambda>
void
FunctorMaterialPropertyImpl<T>::setFunctor(const MooseMesh & mesh,
                                           const std::set<SubdomainID> & block_ids,
                                           PolymorphicLambda my_lammy)
{
  auto add_lammy = [this, my_lammy](const SubdomainID block_id) {
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
    _tqp_functor.emplace(block_id, my_lammy);
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
std::string
FunctorMaterialPropertyImpl<T>::subdomainErrorMessage(const SubdomainID sub_id) const
{
  return "The provided subdomain ID " + std::to_string(sub_id) +
         " doesn't exist in the map for material property " + _name +
         "! This is likely because you did not provide a functor material "
         "definition on that subdomain";
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::ValueType
FunctorMaterialPropertyImpl<T>::evaluate(const Elem * const & elem, unsigned int state) const
{
  mooseAssert(elem && elem != libMesh::remote_elem,
              "The element must be non-null and non-remote in functor material properties");
  auto it = _elem_functor.find(elem->subdomain_id());
  mooseAssert(it != _elem_functor.end(), subdomainErrorMessage(elem->subdomain_id()));
  return it->second(elem, state);
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::ValueType
FunctorMaterialPropertyImpl<T>::evaluate(const ElemFromFaceArg & elem_from_face,
                                         unsigned int state) const
{
  mooseAssert(
      (std::get<0>(elem_from_face) && std::get<0>(elem_from_face) != libMesh::remote_elem) ||
          std::get<1>(elem_from_face),
      "The element must be non-null and non-remote or the face must be non-null in functor "
      "material properties");
  auto it = _elem_from_face_functor.find(std::get<2>(elem_from_face));
  mooseAssert(it != _elem_from_face_functor.end(),
              subdomainErrorMessage(std::get<2>(elem_from_face)));
  return it->second(elem_from_face, state);
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::ValueType
FunctorMaterialPropertyImpl<T>::evaluate(const SingleSidedFaceArg & face, unsigned int state) const
{
  const auto sub_id = std::get<3>(face);
  auto it = _face_functor.find(sub_id);
  mooseAssert(it != _face_functor.end(), subdomainErrorMessage(sub_id));
  return it->second(face, state);
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::ValueType
FunctorMaterialPropertyImpl<T>::evaluate(const FaceArg & face, unsigned int state) const
{
  using namespace Moose::FV;

  const auto elem_sub_id = std::get<3>(face).first;
  const auto neighbor_sub_id = std::get<3>(face).second;
  const auto limiter = std::get<1>(face);
  const auto * const face_info = std::get<0>(face);
  mooseAssert(face_info,
              "We must have a non-null face_info in order to prepare our ElemFromFace tuples");
  const bool fi_elem_is_upwind = std::get<2>(face);
  static const typename libMesh::TensorTools::IncrementRank<T>::type example_gradient(0);

  switch (limiter)
  {
    case LimiterType::CentralDifference:
    case LimiterType::Upwind:
    {
      const auto elem_from_face = std::make_tuple(&face_info->elem(), face_info, elem_sub_id);
      const auto neighbor_from_face =
          std::make_tuple(face_info->neighborPtr(), face_info, neighbor_sub_id);
      const auto & upwind_elem = fi_elem_is_upwind ? elem_from_face : neighbor_from_face;
      const auto & downwind_elem = fi_elem_is_upwind ? neighbor_from_face : elem_from_face;
      auto limiter_obj = Limiter<typename LimiterValueType<T>::value_type>::build(limiter);
      return interpolate(*limiter_obj,
                         evaluate(upwind_elem, state),
                         evaluate(downwind_elem, state),
                         &example_gradient,
                         *face_info,
                         fi_elem_is_upwind);
    }

    default:
      mooseError("Unsported limiter type in FunctorMaterialPropertyImpl::evaluate(FaceArg)");
  }
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::ValueType
FunctorMaterialPropertyImpl<T>::evaluate(const ElemQpArg & elem_qp, unsigned int state) const
{
  const auto sub_id = std::get<0>(elem_qp)->subdomain_id();
  auto it = _elem_qp_functor.find(sub_id);
  mooseAssert(it != _elem_qp_functor.end(), subdomainErrorMessage(sub_id));
  return it->second(elem_qp, state);
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::ValueType
FunctorMaterialPropertyImpl<T>::evaluate(const ElemSideQpArg & elem_side_qp,
                                         unsigned int state) const
{
  const auto sub_id = std::get<0>(elem_side_qp)->subdomain_id();
  auto it = _elem_side_qp_functor.find(sub_id);
  mooseAssert(it != _elem_side_qp_functor.end(), subdomainErrorMessage(sub_id));
  return it->second(elem_side_qp, state);
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::ValueType
FunctorMaterialPropertyImpl<T>::evaluate(
    const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp, unsigned int state) const
{
  const auto sub_id = std::get<2>(tqp);
  auto it = _tqp_functor.find(sub_id);
  mooseAssert(it != _tqp_functor.end(), subdomainErrorMessage(sub_id));
  return it->second(tqp, state);
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::GradientType
FunctorMaterialPropertyImpl<T>::evaluateGradient(const Elem * const &, unsigned int) const
{
  mooseError("Gradients of functor material properties not implemented");
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::GradientType
FunctorMaterialPropertyImpl<T>::evaluateGradient(const ElemFromFaceArg &, unsigned int) const
{
  mooseError("Gradients of functor material properties not implemented");
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::GradientType
FunctorMaterialPropertyImpl<T>::evaluateGradient(const FaceArg &, unsigned int) const
{
  mooseError("Gradients of functor material properties not implemented");
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::GradientType
FunctorMaterialPropertyImpl<T>::evaluateGradient(const SingleSidedFaceArg &, unsigned int) const
{
  mooseError("Gradients of functor material properties not implemented");
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::GradientType
FunctorMaterialPropertyImpl<T>::evaluateGradient(const ElemQpArg &, unsigned int) const
{
  mooseError("Gradients of functor material properties not implemented");
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::GradientType
FunctorMaterialPropertyImpl<T>::evaluateGradient(const ElemSideQpArg &, unsigned int) const
{
  mooseError("Gradients of functor material properties not implemented");
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::GradientType
FunctorMaterialPropertyImpl<T>::evaluateGradient(
    const std::tuple<Moose::ElementType, unsigned int, SubdomainID> &, unsigned int) const
{
  mooseError("Gradients of functor material properties not implemented");
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::DotType
FunctorMaterialPropertyImpl<T>::evaluateDot(const Elem * const &, unsigned int) const
{
  mooseError("Time derivatives of functor material properties not implemented");
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::DotType
FunctorMaterialPropertyImpl<T>::evaluateDot(const ElemFromFaceArg &, unsigned int) const
{
  mooseError("Time derivatives of functor material properties not implemented");
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::DotType
FunctorMaterialPropertyImpl<T>::evaluateDot(const FaceArg &, unsigned int) const
{
  mooseError("Time derivatives of functor material properties not implemented");
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::DotType
FunctorMaterialPropertyImpl<T>::evaluateDot(const SingleSidedFaceArg &, unsigned int) const
{
  mooseError("Time derivatives of functor material properties not implemented");
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::DotType
FunctorMaterialPropertyImpl<T>::evaluateDot(const ElemQpArg &, unsigned int) const
{
  mooseError("Time derivatives of functor material properties not implemented");
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::DotType
FunctorMaterialPropertyImpl<T>::evaluateDot(const ElemSideQpArg &, unsigned int) const
{
  mooseError("Time derivatives of functor material properties not implemented");
}

template <typename T>
typename FunctorMaterialPropertyImpl<T>::DotType
FunctorMaterialPropertyImpl<T>::evaluateDot(
    const std::tuple<Moose::ElementType, unsigned int, SubdomainID> &, unsigned int) const
{
  mooseError("Time derivatives of functor material properties not implemented");
}

/**
 * This is a wrapper that forwards calls to the implementation,
 * which can be switched out at any time without disturbing references to
 * FunctorMaterialPropertyImpl Implementation motivated by
 * https://stackoverflow.com/a/65455485/4493669
 */
template <typename T>
class FunctorMaterialProperty : public Moose::Functor<T>
{
public:
  using typename Moose::Functor<T>::FaceArg;
  using typename Moose::Functor<T>::SingleSidedFaceArg;
  using typename Moose::Functor<T>::ElemFromFaceArg;
  using typename Moose::Functor<T>::ElemQpArg;
  using typename Moose::Functor<T>::ElemSideQpArg;
  using typename Moose::Functor<T>::FunctorType;
  using typename Moose::Functor<T>::FunctorReturnType;
  using typename Moose::Functor<T>::ValueType;
  using typename Moose::Functor<T>::GradientType;
  using typename Moose::Functor<T>::DotType;

  /**
   * Construct wrapper from wrapped object
   */
  FunctorMaterialProperty(std::unique_ptr<FunctorMaterialPropertyImpl<T>> && wrapped)
    : Moose::Functor<T>(), _wrapped(std::move(wrapped))
  {
  }

  /**
   * Assign our wrapped object to be something new and release our previously wrapped object
   */
  FunctorMaterialProperty<T> & operator=(std::unique_ptr<FunctorMaterialPropertyImpl<T>> && wrapped)
  {
    _wrapped = std::move(wrapped);
    return *this;
  }

  virtual ~FunctorMaterialProperty() = default;

  /**
   * Set the functor that will be used in calls to the wrapped object's \p evaluate overloads
   * @param mesh The mesh that the wrapped object is defined on
   * @param block_ids The block/subdomain IDs that the wrapped object is valid for
   * @param my_lammy The functor that defines how the wrapped object is evaluated
   */
  template <typename PolymorphicLambda>
  void setFunctor(const MooseMesh & mesh,
                  const std::set<SubdomainID> & block_ids,
                  PolymorphicLambda my_lammy)
  {
    _wrapped->setFunctor(mesh, block_ids, my_lammy);
  }

  /**
   * Tests whether the wrapped object is of the requested type
   */
  template <typename T2>
  bool wrapsType() const
  {
    return dynamic_cast<T2 *>(_wrapped.get());
  }

protected:
  ///@{
  /**
   * Forward evaluate calls to wrapped object
   */
  ValueType evaluate(const libMesh::Elem * const & elem, unsigned int state = 0) const override
  {
    return _wrapped->evaluate(elem, state);
  }
  ValueType evaluate(const ElemFromFaceArg & elem_from_face, unsigned int state = 0) const override
  {
    return _wrapped->evaluate(elem_from_face, state);
  }
  ValueType evaluate(const FaceArg & face, unsigned int state = 0) const override
  {
    return _wrapped->evaluate(face, state);
  }
  ValueType evaluate(const SingleSidedFaceArg & face, unsigned int state = 0) const override
  {
    return _wrapped->evaluate(face, state);
  }
  ValueType evaluate(const ElemQpArg & qp, unsigned int state = 0) const override
  {
    return _wrapped->evaluate(qp, state);
  }
  ValueType evaluate(const ElemSideQpArg & qp, unsigned int state = 0) const override
  {
    return _wrapped->evaluate(qp, state);
  }
  ValueType evaluate(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp,
                     unsigned int state = 0) const override
  {
    return _wrapped->evaluate(tqp, state);
  }
  GradientType evaluateGradient(const libMesh::Elem * const & elem,
                                unsigned int state = 0) const override
  {
    return _wrapped->evaluateGradient(elem, state);
  }
  GradientType evaluateGradient(const ElemFromFaceArg & elem_from_face,
                                unsigned int state = 0) const override
  {
    return _wrapped->evaluateGradient(elem_from_face, state);
  }
  GradientType evaluateGradient(const FaceArg & face, unsigned int state = 0) const override
  {
    return _wrapped->evaluateGradient(face, state);
  }
  GradientType evaluateGradient(const SingleSidedFaceArg & face,
                                unsigned int state = 0) const override
  {
    return _wrapped->evaluateGradient(face, state);
  }
  GradientType evaluateGradient(const ElemQpArg & qp, unsigned int state = 0) const override
  {
    return _wrapped->evaluateGradient(qp, state);
  }
  GradientType evaluateGradient(const ElemSideQpArg & qp, unsigned int state = 0) const override
  {
    return _wrapped->evaluateGradient(qp, state);
  }
  GradientType
  evaluateGradient(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp,
                   unsigned int state = 0) const override
  {
    return _wrapped->evaluateGradient(tqp, state);
  }
  DotType evaluateDot(const libMesh::Elem * const & elem, unsigned int state = 0) const override
  {
    return _wrapped->evaluateDot(elem, state);
  }
  DotType evaluateDot(const ElemFromFaceArg & elem_from_face, unsigned int state = 0) const override
  {
    return _wrapped->evaluateDot(elem_from_face, state);
  }
  DotType evaluateDot(const FaceArg & face, unsigned int state = 0) const override
  {
    return _wrapped->evaluateDot(face, state);
  }
  DotType evaluateDot(const SingleSidedFaceArg & face, unsigned int state = 0) const override
  {
    return _wrapped->evaluateDot(face, state);
  }
  DotType evaluateDot(const ElemQpArg & qp, unsigned int state = 0) const override
  {
    return _wrapped->evaluateDot(qp, state);
  }
  DotType evaluateDot(const ElemSideQpArg & qp, unsigned int state = 0) const override
  {
    return _wrapped->evaluateDot(qp, state);
  }
  DotType evaluateDot(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp,
                      unsigned int state = 0) const override
  {
    return _wrapped->evaluateDot(tqp, state);
  }
  ///@}

private:
  /// Our wrapped object
  std::unique_ptr<FunctorMaterialPropertyImpl<T>> _wrapped;
};
