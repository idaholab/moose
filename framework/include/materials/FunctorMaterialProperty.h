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

#include "libmesh/elem.h"
#include "libmesh/remote_elem.h"

#include <unordered_map>
#include <functional>

class FunctorPropertyValue
{
public:
  FunctorPropertyValue() = default;

  virtual ~FunctorPropertyValue() = default;
};

template <typename T>
class FunctorMaterialProperty : public FunctorPropertyValue, public FunctorInterface<T>
{
public:
  FunctorMaterialProperty(const std::string & name) : FunctorPropertyValue(), _name(name) {}

  using ElemFn = std::function<T(const Elem * const &)>;
  using FaceFn = std::function<T(const std::pair<const FaceInfo *, const Moose::FV::Limiter *> &)>;

  template <typename PolymorphicLambda>
  FunctorMaterialProperty<T> & setFunction(const MooseMesh & mesh,
                                           const std::set<SubdomainID> & block_ids,
                                           PolymorphicLambda my_lammy);

  T operator()(const Elem * elem) const override final;
  T operator()(
      const std::pair<const FaceInfo *, const Moose::FV::Limiter *> & face) const override final;

private:
  std::unordered_map<SubdomainID, ElemFn> _elem_functor;
  FaceFn _face_functor;
  std::string _name;
};

template <typename T>
template <typename PolymorphicLambda>
FunctorMaterialProperty<T> &
FunctorMaterialProperty<T>::setFunction(const MooseMesh & mesh,
                                        const std::set<SubdomainID> & block_ids,
                                        PolymorphicLambda my_lammy)
{
  for (const auto block_id : block_ids)
  {
    if (block_id == Moose::ANY_BLOCK_ID)
    {
      const auto & inner_block_ids = mesh.meshSubdomains();
      for (const auto inner_block_id : inner_block_ids)
        _elem_functor.emplace(inner_block_id, my_lammy);
    }
    else
      _elem_functor.emplace(block_id, my_lammy);
  }

  _face_functor = my_lammy;

  return *this;
}

template <typename T>
T
FunctorMaterialProperty<T>::operator()(const Elem * const & elem) const
{
  mooseAssert(elem && elem != libMesh::remote_elem,
              "The element must be non-null and non-remote in functor material properties");
  auto it = _elem_functor.find(elem->subdomain_id());
  mooseAssert(it != _elem_functor.end(), "The provided subdomain ID doesn't exist in the map!");
  return it->second(elem);
}

template <typename T>
T
FunctorMaterialProperty<T>::operator()(
    const std::pair<const FaceInfo *, const Moose::FV::Limiter *> & face) const
{
  mooseAssert(face.first && face.second, "FaceInfo and Limiter must be non-null");
  return _face_functor(face);
}
