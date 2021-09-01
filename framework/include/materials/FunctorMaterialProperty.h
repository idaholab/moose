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
#include "FunctorInterface.h"
#include "Moose.h"
#include "Limiter.h"
#include "FVUtils.h"

#include "libmesh/elem.h"
#include "libmesh/remote_elem.h"
#include "libmesh/tensor_tools.h"

#include <unordered_map>
#include <functional>

/**
 * A material property that is evaluated on-the-fly via calls to various overloads of \p operator()
 */
template <typename T>
class FunctorMaterialProperty : public FunctorInterface<T>

{
public:
  FunctorMaterialProperty(const std::string & name) : _name(name) {}

  /**
   * Set the functor that will be used in calls to \p evaluate overloads
   * @param mesh The mesh that the functor is defined on
   * @param block_ids The block/subdomain IDs that the user-provided functor is valid for
   * @param my_lammy The functor that defines this how this object is evaluated
   */
  template <typename PolymorphicLambda>
  void setFunctor(const MooseMesh & mesh,
                  const std::set<SubdomainID> & block_ids,
                  PolymorphicLambda my_lammy);

private:
  using typename FunctorInterface<T>::FaceArg;
  using typename FunctorInterface<T>::ElemFromFaceArg;
  using typename FunctorInterface<T>::ElemQpArg;
  using typename FunctorInterface<T>::ElemSideQpArg;
  using typename FunctorInterface<T>::FunctorType;
  using typename FunctorInterface<T>::FunctorReturnType;

  using ElemFn = std::function<T(const Elem * const &, const unsigned int &)>;
  using ElemAndFaceFn = std::function<T(const ElemFromFaceArg &, const unsigned int &)>;
  using ElemQpFn = std::function<T(const ElemQpArg &, const unsigned int &)>;
  using ElemSideQpFn = std::function<T(const ElemSideQpArg &, const unsigned int &)>;
  using TQpFn = std::function<T(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> &,
                                const unsigned int &)>;

  T evaluate(const Elem * const & elem, unsigned int state) const override final;
  T evaluate(const ElemFromFaceArg & elem_from_face, unsigned int state) const override final;
  T evaluate(const FaceArg & face, unsigned int state) const override final;
  T evaluate(const ElemQpArg & elem_qp, unsigned int state) const override final;
  T evaluate(const ElemSideQpArg & elem_side_qp, unsigned int state) const override final;
  T evaluate(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp,
             unsigned int state) const override final;

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

  /// Functors that will evaluate elements at quadrature points
  std::unordered_map<SubdomainID, ElemQpFn> _elem_qp_functor;

  /// Functors that will evaluate elements at side quadrature points
  std::unordered_map<SubdomainID, ElemSideQpFn> _elem_side_qp_functor;

  /// Functors that will index elemental, neighbor, or lower-dimensional data at a provided
  /// quadrature point index
  std::unordered_map<SubdomainID, TQpFn> _tqp_functor;

  /// The name of this object
  std::string _name;
};

template <typename T>
template <typename PolymorphicLambda>
void
FunctorMaterialProperty<T>::setFunctor(const MooseMesh & mesh,
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
FunctorMaterialProperty<T>::subdomainErrorMessage(const SubdomainID sub_id) const
{
  return "The provided subdomain ID " + std::to_string(sub_id) +
         " doesn't exist in the map for material property " + _name +
         "! This is likely because you did not provide a functor material "
         "definition on that subdomain";
}

template <typename T>
T
FunctorMaterialProperty<T>::evaluate(const Elem * const & elem, unsigned int state) const
{
  mooseAssert(elem && elem != libMesh::remote_elem,
              "The element must be non-null and non-remote in functor material properties");
  auto it = _elem_functor.find(elem->subdomain_id());
  mooseAssert(it != _elem_functor.end(), subdomainErrorMessage(elem->subdomain_id()));
  return it->second(elem, state);
}

template <typename T>
T
FunctorMaterialProperty<T>::evaluate(const ElemFromFaceArg & elem_from_face,
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
T
FunctorMaterialProperty<T>::evaluate(const FaceArg & face, unsigned int state) const
{
  const auto elem_sub_id = std::get<3>(face).first;
  const auto neighbor_sub_id = std::get<3>(face).second;
  const auto * const limiter = std::get<1>(face);
  mooseAssert(limiter,
              "We must have a non-null limiter in order to decide how to interpolate a functor "
              "material property");
  const auto * const face_info = std::get<0>(face);
  mooseAssert(face_info,
              "We must have a non-null face_info in order to prepare our ElemFromFace tuples");
  const bool fi_elem_is_upwind = std::get<2>(face);
  static const typename libMesh::TensorTools::IncrementRank<T>::type example_gradient(0);

  switch (limiter->interpMethod())
  {
    case Moose::FV::InterpMethod::Average:
    case Moose::FV::InterpMethod::Upwind:
    {
      const auto elem_from_face = std::make_tuple(&face_info->elem(), face_info, elem_sub_id);
      const auto neighbor_from_face =
          std::make_tuple(face_info->neighborPtr(), face_info, neighbor_sub_id);
      const auto & upwind_elem = fi_elem_is_upwind ? elem_from_face : neighbor_from_face;
      const auto & downwind_elem = fi_elem_is_upwind ? neighbor_from_face : elem_from_face;
      return Moose::FV::interpolate(*limiter,
                                    evaluate(upwind_elem, state),
                                    evaluate(downwind_elem, state),
                                    &example_gradient,
                                    *face_info,
                                    fi_elem_is_upwind);
    }

    default:
    {
      mooseAssert(!limiter->constant(),
                  "If the limiter is constant, then we should be able to build it into the functor "
                  "material property switch-case statement at faces");
      mooseError("Unsported limiter type in FunctorMaterialProperty::evaluate(FaceArg)");
    }
  }
}

template <typename T>
T
FunctorMaterialProperty<T>::evaluate(const ElemQpArg & elem_qp, unsigned int state) const
{
  const auto sub_id = std::get<0>(elem_qp)->subdomain_id();
  auto it = _elem_qp_functor.find(sub_id);
  mooseAssert(it != _elem_qp_functor.end(), subdomainErrorMessage(sub_id));
  return it->second(elem_qp, state);
}

template <typename T>
T
FunctorMaterialProperty<T>::evaluate(const ElemSideQpArg & elem_side_qp, unsigned int state) const
{
  const auto sub_id = std::get<0>(elem_side_qp)->subdomain_id();
  auto it = _elem_side_qp_functor.find(sub_id);
  mooseAssert(it != _elem_side_qp_functor.end(), subdomainErrorMessage(sub_id));
  return it->second(elem_side_qp, state);
}

template <typename T>
T
FunctorMaterialProperty<T>::evaluate(
    const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp, unsigned int state) const
{
  const auto sub_id = std::get<2>(tqp);
  auto it = _tqp_functor.find(sub_id);
  mooseAssert(it != _tqp_functor.end(), subdomainErrorMessage(sub_id));
  return it->second(tqp, state);
}
