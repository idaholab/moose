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
#include "GreenGaussGradient.h"
#include "MathFVUtils.h"
#include "libmesh/utility.h"
#include "libmesh/type_tensor.h"
#include "libmesh/compare_types.h"
#include "libmesh/threads.h"

/**
 * A functor whose evaluation relies on querying a map where the keys are element ids and the values
 * correspond to the element/cell values. This is a very useful data type for storing the result of
 * (possibly repeated) interpolation and reconstruction
 */
template <typename T, typename Map>
class CellCenteredMapFunctor : public Moose::FunctorBase<T>, public Map
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

  /**
   * Use this constructor when you want the object to live everywhere on the mesh
   */
  CellCenteredMapFunctor(const MooseMesh & mesh,
                         const std::string & name,
                         const bool extrapolated_boundary);

  /**
   * Use this constructor if you want to potentially restrict this object to a specified set of
   * subdomains/blocks
   */
  CellCenteredMapFunctor(const MooseMesh & mesh,
                         const std::set<SubdomainID> & sub_ids,
                         const std::string & name,
                         const bool extrapolated_boundary);

  bool isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                  const Elem * elem,
                                  const StateArg & state) const override;
  bool hasBlocks(SubdomainID sub_id) const override;

  /**
   * Checks whether we are defined on the provided element
   */
  bool hasBlocks(const Elem * elem) const;

  bool supportsFaceArg() const override final { return true; }
  bool supportsElemSideQpArg() const override final { return false; }
  bool mayRequireGhosting() const override final { return true; }

private:
  /// The mesh that this functor lives on
  const MooseMesh & _mesh;

  /// The subdomain IDs that this functor lives on. If empty, then we consider the functor to live
  /// on all subdomains
  const std::set<SubdomainID> _sub_ids;

  const bool _extrapolated_boundary;

  ValueType evaluate(const ElemArg & elem_arg, const StateArg &) const override;
  ValueType evaluate(const ElemPointArg & elem_point, const StateArg & state) const override;
  ValueType evaluate(const FaceArg & face, const StateArg &) const override;
  ValueType evaluate(const ElemQpArg &, const StateArg &) const override;
  ValueType evaluate(const ElemSideQpArg &, const StateArg &) const override;
  ValueType evaluate(const NodeArg & elem_arg, const StateArg &) const override;

  using Moose::FunctorBase<T>::evaluateGradient;
  GradientType evaluateGradient(const ElemArg & elem_arg, const StateArg & state) const override;
  GradientType evaluateGradient(const FaceArg & face, const StateArg & state) const override;
};

template <typename T, typename Map>
CellCenteredMapFunctor<T, Map>::CellCenteredMapFunctor(const MooseMesh & mesh,
                                                       const std::string & name,
                                                       const bool extrapolated_boundary)
  : Moose::FunctorBase<T>(name), _mesh(mesh), _extrapolated_boundary(extrapolated_boundary)
{
}

template <typename T, typename Map>
CellCenteredMapFunctor<T, Map>::CellCenteredMapFunctor(const MooseMesh & mesh,
                                                       const std::set<SubdomainID> & sub_ids,
                                                       const std::string & name,
                                                       const bool extrapolated_boundary)
  : Moose::FunctorBase<T>(name),
    _mesh(mesh),
    _sub_ids(sub_ids == mesh.meshSubdomains() ? std::set<SubdomainID>() : sub_ids),
    _extrapolated_boundary(extrapolated_boundary)
{
}

template <typename T, typename Map>
bool
CellCenteredMapFunctor<T, Map>::isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                                           const Elem *,
                                                           const StateArg &) const
{
  const bool defined_on_elem = hasBlocks(&fi.elem());
  const bool defined_on_neighbor = hasBlocks(fi.neighborPtr());
  const bool extrapolated = (defined_on_elem + defined_on_neighbor) == 1;

  mooseAssert(defined_on_elem || defined_on_neighbor,
              "This shouldn't be called if we aren't defined on either side.");
  return extrapolated;
}

template <typename T, typename Map>
bool
CellCenteredMapFunctor<T, Map>::hasBlocks(const Elem * const elem) const
{
  if (!elem)
    return false;

  return hasBlocks(elem->subdomain_id());
}

template <typename T, typename Map>
bool
CellCenteredMapFunctor<T, Map>::hasBlocks(const SubdomainID sub_id) const
{
  return _sub_ids.empty() || _sub_ids.count(sub_id);
}

template <typename T, typename Map>
typename CellCenteredMapFunctor<T, Map>::ValueType
CellCenteredMapFunctor<T, Map>::evaluate(const ElemArg & elem_arg, const StateArg &) const
{
  const Elem * const elem = elem_arg.elem;

  try
  {
    return libmesh_map_find(*this, elem->id());
  }
  catch (libMesh::LogicError &)
  {
    if (!_sub_ids.empty() && !_sub_ids.count(elem->subdomain_id()))
      mooseError("Attempted to evaluate CellCenteredMapFunctor '",
                 this->functorName(),
                 "' with an element subdomain id of '",
                 elem->subdomain_id(),
                 "' but that subdomain id is not one of the subdomain ids the functor is "
                 "restricted to.");
    else
      mooseError("Attempted access into CellCenteredMapFunctor '",
                 this->functorName(),
                 "' with a key that does not yet exist in the map. Make sure to fill your "
                 "CellCenteredMapFunctor for all elements you will attempt to access later.");
  }
}

template <typename T, typename Map>
typename CellCenteredMapFunctor<T, Map>::ValueType
CellCenteredMapFunctor<T, Map>::evaluate(const ElemPointArg & elem_point,
                                         const StateArg & state) const
{
  return (*this)(elem_point.makeElem(), state) +
         (elem_point.point - elem_point.elem->vertex_average()) *
             this->gradient(elem_point.makeElem(), state);
}

template <typename T, typename Map>
typename CellCenteredMapFunctor<T, Map>::ValueType
CellCenteredMapFunctor<T, Map>::evaluate(const FaceArg & face, const StateArg & state) const
{
  const auto & fi = *face.fi;
  mooseAssert(face.limiter_type == Moose::FV::LimiterType::CentralDifference,
              "this implementation currently only supports linear interpolations");

  const bool defined_on_elem = hasBlocks(&fi.elem());
  const bool defined_on_neighbor = hasBlocks(fi.neighborPtr());
  if (defined_on_elem && defined_on_neighbor)
    return Moose::FV::linearInterpolation(*this, face, state);

  if (defined_on_elem)
  {
    const auto elem_arg = face.makeElem();
    const auto elem_value = (*this)(elem_arg, state);
    if (!_extrapolated_boundary)
      return elem_value;
    // Two term expansion
    return elem_value + this->gradient(elem_arg, state) * (fi.faceCentroid() - fi.elemCentroid());
  }
  else
  {
    mooseAssert(defined_on_neighbor, "We should be defined on one of the sides");
    const auto neighbor_arg = face.makeNeighbor();
    const auto neighbor_value = (*this)(neighbor_arg, state);
    if (!_extrapolated_boundary)
      return neighbor_value;

    // Two term expansion
    return neighbor_value +
           this->gradient(neighbor_arg, state) * (fi.faceCentroid() - fi.neighborCentroid());
  }
}

template <typename T, typename Map>
typename CellCenteredMapFunctor<T, Map>::ValueType
CellCenteredMapFunctor<T, Map>::evaluate(const ElemQpArg &, const StateArg &) const
{
  mooseError("not implemented");
}

template <typename T, typename Map>
typename CellCenteredMapFunctor<T, Map>::ValueType
CellCenteredMapFunctor<T, Map>::evaluate(const ElemSideQpArg &, const StateArg &) const
{
  mooseError("not implemented");
}

template <typename T, typename Map>
typename CellCenteredMapFunctor<T, Map>::ValueType
CellCenteredMapFunctor<T, Map>::evaluate(const NodeArg &, const StateArg &) const
{
  mooseError("not implemented");
}

template <typename T, typename Map>
typename CellCenteredMapFunctor<T, Map>::GradientType
CellCenteredMapFunctor<T, Map>::evaluateGradient(const ElemArg & elem_arg,
                                                 const StateArg & state) const
{
  return Moose::FV::greenGaussGradient(elem_arg, state, *this, _extrapolated_boundary, _mesh);
}

template <typename T, typename Map>
typename CellCenteredMapFunctor<T, Map>::GradientType
CellCenteredMapFunctor<T, Map>::evaluateGradient(const FaceArg & face, const StateArg & state) const
{
  return Moose::FV::greenGaussGradient(face, state, *this, _extrapolated_boundary, _mesh);
}
