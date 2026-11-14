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

/**
 * A face functor that stores its face values in a map keyed by face info id. The primary purpose
 * of this functor is to store face based fields (face flux, face velocity, normal gradient) often
 * encountered in fluid dynamics problems using the finite volume method.
 */
template <typename T, typename Map>
class FaceCenteredMapFunctor : public FaceFunctorBase<T>, public Map
{
public:
  using Base = FaceFunctorBase<T>;
  using typename Base::StateArg;
  using typename Base::ValueType;

  FaceCenteredMapFunctor(const MooseMesh & mesh, const std::string & name) : Base(mesh, name) {}

  FaceCenteredMapFunctor(const MooseMesh & mesh,
                         const std::set<SubdomainID> & sub_ids,
                         const std::string & name)
    : Base(mesh, sub_ids, name)
  {
  }

  ValueType evaluate(const FaceInfo * const fi) const override;

private:
  // Keep the public evaluate(const FaceInfo *) from hiding the base class overloads
  using Moose::FunctorBase<T>::evaluate;
};

extern template class FaceCenteredMapFunctor<ADRealVectorValue,
                                             std::unordered_map<dof_id_type, ADRealVectorValue>>;
extern template class FaceCenteredMapFunctor<RealVectorValue,
                                             std::unordered_map<dof_id_type, RealVectorValue>>;
extern template class FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>;

template <typename T, typename Map>
inline void
dataStore(std::ostream & stream, FaceCenteredMapFunctor<T, Map> & m, void * context)
{
  Map & m_map = m;
  dataStore(stream, m_map, context);
}

template <typename T, typename Map>
inline void
dataLoad(std::istream & stream, FaceCenteredMapFunctor<T, Map> & m, void * context)
{
  Map & m_map = m;
  dataLoad(stream, m_map, context);
}
