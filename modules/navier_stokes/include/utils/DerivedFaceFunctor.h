//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FaceFunctorBase.h"

#include <functional>

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
class DerivedFaceFunctor : public FaceFunctorBase<T>
{
public:
  using Base = FaceFunctorBase<T>;
  using typename Base::ValueType;

  /// @param face_value the definition of this quantity on a face
  DerivedFaceFunctor(const MooseMesh & mesh,
                     const std::set<SubdomainID> & sub_ids,
                     const std::string & name,
                     std::function<T(const FaceInfo &)> face_value)
    : Base(mesh, sub_ids, name), _face_value(std::move(face_value))
  {
  }

  ValueType evaluate(const FaceInfo * const fi) const override
  {
    mooseAssert(fi, "A derived face functor needs a face.");
    return _face_value(*fi);
  }

private:
  // Keep the public evaluate(const FaceInfo *) from hiding the base class overloads
  using Moose::FunctorBase<T>::evaluate;

  const std::function<T(const FaceInfo &)> _face_value;
};
