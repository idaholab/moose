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

#include <functional>
#include <set>

/**
 * A face functor whose face values follow from other functors rather than from stored data.
 *
 * A quantity that is an exact function of fields the provider already holds does not need storage
 * of its own: a stored copy can only ever agree or disagree with its definition, and keeping the
 * two in step means remembering to refresh it whenever an input moves. Expressing it as a function
 * removes that obligation, and with it the possibility of reading a value that was correct for an
 * earlier state.
 *
 * The function has to be total. The stored fields it replaces were zero-initialised, so a face they
 * had not reached yet read as zero rather than failing, and evaluations during initialisation rely
 * on that.
 */
template <typename T>
class DerivedFaceFunctor : public Moose::FunctorBase<T>
{
public:
  using typename Moose::FunctorBase<T>::ValueType;
  using ElemArg = Moose::ElemArg;
  using FaceArg = Moose::FaceArg;
  using ElemQpArg = Moose::ElemQpArg;
  using ElemSideQpArg = Moose::ElemSideQpArg;
  using ElemPointArg = Moose::ElemPointArg;
  using StateArg = Moose::StateArg;
  using NodeArg = Moose::NodeArg;

  /// @param face_value the definition of this quantity on a face
  DerivedFaceFunctor(const MooseMesh & mesh,
                     const std::set<SubdomainID> & sub_ids,
                     const std::string & name,
                     std::function<T(const FaceInfo &)> face_value)
    : Moose::FunctorBase<T>(name),
      _sub_ids(sub_ids == mesh.meshSubdomains() ? std::set<SubdomainID>() : sub_ids),
      _face_value(std::move(face_value))
  {
  }

  bool hasBlocks(const SubdomainID sub_id) const override
  {
    return _sub_ids.empty() || _sub_ids.count(sub_id);
  }

  /**
   * The value of this functor on a face
   * @param fi The object containing the face information
   */
  ValueType evaluate(const FaceInfo * const fi) const
  {
    mooseAssert(fi, "A derived face functor needs a face.");
    return _face_value(*fi);
  }

  bool supportsFaceArg() const override final { return true; }
  bool supportsElemSideQpArg() const override final { return false; }

private:
  // Keep the public evaluate(const FaceInfo *) from hiding the base class overloads
  using Moose::FunctorBase<T>::evaluate;
  ValueType evaluate(const FaceArg & face, const StateArg &) const override final
  {
    return evaluate(face.fi);
  }
  ValueType evaluate(const ElemArg &, const StateArg &) const override final
  {
    mooseError("A derived face functor is only defined on faces.");
  }
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

  /// The subdomain IDs that this functor lives on. If empty, then we consider the functor to live
  /// on all subdomains
  const std::set<SubdomainID> _sub_ids;

  const std::function<T(const FaceInfo &)> _face_value;
};
