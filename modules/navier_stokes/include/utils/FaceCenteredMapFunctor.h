//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
 * A functor whose evaluation relies on querying a map where the keys are face info ids
 * and the values correspond to the face values. The primary purpose of this functor is to
 * store face based fields (face flux, face velocity, normal gradient) often encountered
 * in fluid dynamics problems using the finite volume method
 */
template <typename T, typename Map>
class FaceCenteredMapFunctor : public Moose::FunctorBase<T>, public Map
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

  FaceCenteredMapFunctor(const MooseMesh & mesh, const std::string & name);

  FaceCenteredMapFunctor(const MooseMesh & mesh,
                         const std::set<SubdomainID> & sub_ids,
                         const std::string & name);

  bool hasBlocks(SubdomainID sub_id) const override;

  /**
   * Evaluate the face functor using a FaceInfo argument.
   * @param fi The object containing the face information
   */
  ValueType evaluate(const FaceInfo * const fi) const;

  bool supportsFaceArg() const override final { return true; }
  bool supportsElemSideQpArg() const override final { return false; }

private:
  /// The mesh that this functor lives on
  const MooseMesh & _mesh;

  /// The subdomain IDs that this functor lives on. If empty, then we consider the functor to live
  /// on all subdomains
  const std::set<SubdomainID> _sub_ids;

  ValueType evaluate(const ElemArg & elem_arg, const StateArg & state) const override final;
  ValueType evaluate(const FaceArg & face, const StateArg & state) const override final;
  ValueType evaluate(const ElemPointArg &, const StateArg &) const override;
  ValueType evaluate(const ElemQpArg &, const StateArg &) const override;
  ValueType evaluate(const ElemSideQpArg &, const StateArg &) const override;
  ValueType evaluate(const NodeArg & node_arg, const StateArg & state) const override final;
};

template <typename T, typename Map>
FaceCenteredMapFunctor<T, Map>::FaceCenteredMapFunctor(const MooseMesh & mesh,
                                                       const std::string & name)
  : Moose::FunctorBase<T>(name), _mesh(mesh)
{
}

template <typename T, typename Map>
FaceCenteredMapFunctor<T, Map>::FaceCenteredMapFunctor(const MooseMesh & mesh,
                                                       const std::set<SubdomainID> & sub_ids,
                                                       const std::string & name)
  : Moose::FunctorBase<T>(name),
    _mesh(mesh),
    _sub_ids(sub_ids == mesh.meshSubdomains() ? std::set<SubdomainID>() : sub_ids)
{
}

template <typename T, typename Map>
bool
FaceCenteredMapFunctor<T, Map>::hasBlocks(const SubdomainID sub_id) const
{
  return _sub_ids.empty() || _sub_ids.count(sub_id);
}

template <typename T, typename Map>
typename FaceCenteredMapFunctor<T, Map>::ValueType
FaceCenteredMapFunctor<T, Map>::evaluate(const ElemPointArg &, const StateArg &) const
{
  mooseError("not implemented");
}

template <typename T, typename Map>
typename FaceCenteredMapFunctor<T, Map>::ValueType
FaceCenteredMapFunctor<T, Map>::evaluate(const ElemQpArg &, const StateArg &) const
{
  mooseError("not implemented");
}

template <typename T, typename Map>
typename FaceCenteredMapFunctor<T, Map>::ValueType
FaceCenteredMapFunctor<T, Map>::evaluate(const ElemSideQpArg &, const StateArg &) const
{
  mooseError("not implemented");
}

template <typename T, typename Map>
typename FaceCenteredMapFunctor<T, Map>::ValueType
FaceCenteredMapFunctor<T, Map>::evaluate(const NodeArg &, const StateArg &) const
{
  mooseError("not implemented");
}

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
