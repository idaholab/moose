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
#include "CellCenteredMapFunctor.h"
#include "GreenGaussGradient.h"
#include "MathFVUtils.h"
#include "libmesh/utility.h"
#include "libmesh/type_tensor.h"
#include "libmesh/compare_types.h"
#include "libmesh/threads.h"

/**
 * A functor whose evaluation relies on querying a map where the keys are face info pointers
 * and the values correspond to the face values. This is a very useful data type for
 * storing face based fields often encountered in fluid dynamics while using the finite volume
 * method
 */
template <typename T, typename Map>
class FaceCenteredMapFunctor : public Moose::FunctorBase<T>, public Map
{
public:
  using typename Moose::FunctorBase<T>::ValueType;
  using typename Moose::FunctorBase<T>::GradientType;
  using typename Moose::FunctorBase<T>::DotType;
  using ElemArg = Moose::ElemArg;
  using ElemFromFaceArg = Moose::ElemFromFaceArg;
  using FaceArg = Moose::FaceArg;
  using SingleSidedFaceArg = Moose::SingleSidedFaceArg;
  using ElemQpArg = Moose::ElemQpArg;
  using ElemSideQpArg = Moose::ElemSideQpArg;
  using ElemPointArg = Moose::ElemPointArg;

  FaceCenteredMapFunctor(const MooseMesh & mesh, const std::string & name)
    : Moose::FunctorBase<T>(name), _mesh(mesh)
  {
  }

  FaceCenteredMapFunctor(const MooseMesh & mesh,
                         const std::set<SubdomainID> & sub_ids,
                         const std::string & name)
    : Moose::FunctorBase<T>(name),
      _mesh(mesh),
      _sub_ids(sub_ids == mesh.meshSubdomains() ? std::set<SubdomainID>() : sub_ids)
  {
  }

private:
  /// The mesh that this functor lives on
  const MooseMesh & _mesh;

  /// The subdomain IDs that this functor lives on. If empty, then we consider the functor to live
  /// on all subdomains
  const std::set<SubdomainID> _sub_ids;

  ValueType evaluate(const ElemArg & elem_arg, unsigned int) const override final
  {
    mooseError("not implemented");
  }

  ValueType evaluate(const ElemFromFaceArg & elem_from_face, unsigned int) const override
  {
    mooseError("not implemented");
  }

  ValueType evaluate(const FaceArg & face, unsigned int) const override final
  {
    try
    {
      return libmesh_map_find(*this, face.fi);
    }
    catch (libMesh::LogicError &)
    {
      if (!_sub_ids.empty() && !_sub_ids.count(elem->subdomain_id()))
        mooseError("Attempted to evaluate FaceCenteredMapFunctor '",
                   this->functorName(),
                   "' with an element subdomain id of '",
                   elem->subdomain_id(),
                   "' but that subdomain id is not one of the subdomain ids the functor is "
                   "restricted to.");
      else
        mooseError("Attempted access into FaceCenteredMapFunctor '",
                   this->functorName(),
                   "' with a key that does not yet exist in the map. Make sure to fill your "
                   "FaceCenteredMapFunctor for all elements you will attempt to access later.");
    }
  }

  ValueType evaluate(const ElemPointArg & elem_point, const unsigned int state) const override final
  {
    mooseError("not implemented");
  }

  using Moose::FunctorBase<T>::evaluateGradient;

  GradientType evaluateGradient(const ElemArg & elem_arg, unsigned int) const override final
  {
    mooseError("not implemented");
  }

  GradientType evaluateGradient(const FaceArg & face, unsigned int) const override final
  {
    mooseError("not implemented");
  }

  ValueType evaluate(const SingleSidedFaceArg & ssf, unsigned int) const override
  {
    return (*this)(Moose::FV::makeCDFace(*ssf.fi));
  }

  ValueType evaluate(const ElemQpArg &, unsigned int) const override
  {
    mooseError("not implemented");
  }

  ValueType evaluate(const ElemSideQpArg &, unsigned int) const override
  {
    mooseError("not implemented");
  }
};
