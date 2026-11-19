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
#include "MooseMesh.h"
#include "MooseError.h"

#include <set>

/**
 * A functor defined by its values on faces.
 *
 * Face values are the only thing a concrete face functor supplies. Everything else follows from
 * them and is implemented here once: the cell value, which is reconstructed from the surrounding
 * face values, the face-argument evaluation, and the block restriction. A face field that is stored
 * and one that is computed from other fields therefore differ in a single method and behave
 * identically everywhere they are consumed.
 */
template <typename T>
class FaceFunctorBase : public Moose::FunctorBase<T>
{
public:
  using typename Moose::FunctorBase<T>::ValueType;
  using typename Moose::FunctorBase<T>::GradientType;
  using typename Moose::FunctorBase<T>::DotType;
  using ElemArg = Moose::ElemArg;
  using FaceArg = Moose::FaceArg;
  using ElemQpArg = Moose::ElemQpArg;
  using ElemSideQpArg = Moose::ElemSideQpArg;
  using ElemPointArg = Moose::ElemPointArg;
  using StateArg = Moose::StateArg;
  using NodeArg = Moose::NodeArg;

  FaceFunctorBase(const MooseMesh & mesh, const std::string & name)
    : Moose::FunctorBase<T>(name), _mesh(mesh)
  {
  }

  FaceFunctorBase(const MooseMesh & mesh,
                  const std::set<SubdomainID> & sub_ids,
                  const std::string & name)
    : Moose::FunctorBase<T>(name),
      _mesh(mesh),
      _sub_ids(sub_ids == mesh.meshSubdomains() ? std::set<SubdomainID>() : sub_ids)
  {
  }

  bool hasBlocks(const SubdomainID sub_id) const override
  {
    return _sub_ids.empty() || _sub_ids.count(sub_id);
  }

  /**
   * The value of this functor on a face. This is what a concrete face functor defines.
   * @param fi The object containing the face information
   */
  virtual ValueType evaluate(const FaceInfo * const fi) const = 0;

  bool supportsFaceArg() const override final { return true; }
  bool supportsElemSideQpArg() const override final { return false; }

protected:
  /// The mesh that this functor lives on
  const MooseMesh & _mesh;

  /// The subdomain IDs that this functor lives on. If empty, then we consider the functor to live
  /// on all subdomains
  const std::set<SubdomainID> _sub_ids;

  // Keep the public evaluate(const FaceInfo *) from hiding the base class overloads
  using Moose::FunctorBase<T>::evaluate;

private:
  ValueType evaluate(const ElemArg &, const StateArg &) const override final;
  ValueType evaluate(const FaceArg &, const StateArg &) const override final;
  ValueType evaluate(const ElemPointArg &, const StateArg &) const override
  {
    mooseError("not implemented");
  }
  ValueType evaluate(const ElemQpArg &, const StateArg &) const override
  {
    mooseError("not implemented");
  }
  ValueType evaluate(const ElemSideQpArg &, const StateArg &) const override
  {
    mooseError("not implemented");
  }
  ValueType evaluate(const NodeArg &, const StateArg &) const override final
  {
    mooseError("not implemented");
  }
};

extern template class FaceFunctorBase<ADRealVectorValue>;
extern template class FaceFunctorBase<RealVectorValue>;
extern template class FaceFunctorBase<Real>;
