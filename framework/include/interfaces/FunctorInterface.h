//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <tuple>

#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseError.h"
#include "FunctorInterface.h"

#include "libmesh/elem.h"
#include "libmesh/remote_elem.h"

#include <unordered_map>
#include <functional>

class FaceInfo;
namespace Moose
{
namespace FV
{
class Limiter;
}
}

template <typename T>
class FunctorInterface
{
public:
  using FaceArg = std::tuple<const FaceInfo *, const Moose::FV::Limiter *, bool>;
  using ElemAndFaceArg = std::tuple<const libMesh::Elem *, const FaceInfo *, SubdomainID>;
  using FunctorType = FunctorInterface<T>;
  using FunctorReturnType = T;
  virtual ~FunctorInterface() = default;

  virtual T operator()(const libMesh::Elem * const & elem) const = 0;
  virtual T operator()(const ElemAndFaceArg & elem_and_face) const = 0;
  virtual T
  operator()(const std::tuple<const FaceInfo *, const Moose::FV::Limiter *, bool> & face) const = 0;
  virtual T operator()(const unsigned int & qp) const = 0;
  virtual T operator()(const std::pair<Moose::ElementType, unsigned int> & tqp) const = 0;
};

template <typename T>
class GenericFunctor : public FunctorInterface<T>
{
public:
  GenericFunctor(const std::string & name) : _name(name) {}

  using typename FunctorInterface<T>::FaceArg;
  using typename FunctorInterface<T>::ElemAndFaceArg;
  using typename FunctorInterface<T>::FunctorType;
  using typename FunctorInterface<T>::FunctorReturnType;

  using ElemFn = std::function<T(const Elem * const &)>;
  using ElemAndFaceFn = std::function<T(const ElemAndFaceArg &)>;
  using FaceFn = std::function<T(const FaceArg &)>;
  using QpFn = std::function<T(const unsigned int &)>;
  using TQpFn = std::function<T(const std::pair<Moose::ElementType, unsigned int> &)>;

  template <typename PolymorphicLambda>
  void setFunction(const MooseMesh & mesh,
                   const std::set<SubdomainID> & block_ids,
                   PolymorphicLambda my_lammy);

  T operator()(const Elem * const & elem) const override final;
  T operator()(const ElemAndFaceArg & elem_and_face) const override final;
  T operator()(const FaceArg & face) const override final;
  T operator()(const unsigned int & qp) const override final;
  T operator()(const std::pair<Moose::ElementType, unsigned int> & tqp) const override final;

private:
  std::unordered_map<SubdomainID, ElemFn> _elem_functor;
  std::unordered_map<SubdomainID, ElemAndFaceFn> _elem_and_face_functor;
  FaceFn _face_functor;
  QpFn _qp_functor;
  TQpFn _tqp_functor;
  std::string _name;
};

template <typename T>
template <typename PolymorphicLambda>
void
GenericFunctor<T>::setFunction(const MooseMesh & mesh,
                               const std::set<SubdomainID> & block_ids,
                               PolymorphicLambda my_lammy)
{
  auto add_lammy = [this, my_lammy](const SubdomainID block_id) {
    _elem_functor.emplace(block_id, my_lammy);
    _elem_and_face_functor.emplace(block_id, my_lammy);
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

  _face_functor = my_lammy;
  _qp_functor = my_lammy;
  _tqp_functor = my_lammy;
}

template <typename T>
T
GenericFunctor<T>::operator()(const Elem * const & elem) const
{
  mooseAssert(elem && elem != libMesh::remote_elem,
              "The element must be non-null and non-remote in functor material properties");
  auto it = _elem_functor.find(elem->subdomain_id());
  mooseAssert(it != _elem_functor.end(), "The provided subdomain ID doesn't exist in the map!");
  return it->second(elem);
}

template <typename T>
T
GenericFunctor<T>::operator()(const ElemAndFaceArg & elem_and_face) const
{
  mooseAssert((std::get<0>(elem_and_face) && std::get<0>(elem_and_face) != libMesh::remote_elem) ||
                  std::get<1>(elem_and_face),
              "The element must be non-null and non-remote or the face must be non-null in functor "
              "material properties");
  auto it = _elem_and_face_functor.find(std::get<2>(elem_and_face));
  mooseAssert(it != _elem_and_face_functor.end(),
              "The provided subdomain ID doesn't exist in the map!");
  return it->second(elem_and_face);
}

template <typename T>
T
GenericFunctor<T>::operator()(const GenericFunctor<T>::FaceArg & face) const
{
  mooseAssert(std::get<0>(face), "FaceInfo must be non-null");
  return _face_functor(face);
}

template <typename T>
T
GenericFunctor<T>::operator()(const unsigned int & qp) const
{
  return _qp_functor(qp);
}

template <typename T>
T
GenericFunctor<T>::operator()(const std::pair<Moose::ElementType, unsigned int> & tqp) const
{
  return _tqp_functor(tqp);
}
