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
#include "MooseFunctor.h"
#include "Moose.h"
#include "Limiter.h"
#include "MathFVUtils.h"

#include "libmesh/elem.h"
#include "libmesh/remote_elem.h"

#include <unordered_map>
#include <functional>

/**
 * A functor property that is evaluated on-the-fly via calls to various overloads of \p operator()
 */
template <typename T>
class BoundaryIntegralFunctor : public Moose::FunctorBase<T>
{
public:
  BoundaryIntegralFunctor(const std::string & name,
                          const Moose::FunctorEnvelope<T> & functor,
                          const std::set<ExecFlagType> & clearance_schedule,
                          const MooseMesh & mesh,
                          const BoundaryID boundary_id);

  virtual ~BoundaryIntegralFunctor() = default;

  bool isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                  const Elem * elem,
                                  const Moose::StateArg & time) const override;

  bool hasBlocks(SubdomainID id) const override;

  using typename Moose::FunctorBase<T>::FunctorType;
  using typename Moose::FunctorBase<T>::ValueType;
  using typename Moose::FunctorBase<T>::FunctorReturnType;

protected:
  ValueType evaluate(const Moose::ElemArg & elem_arg, const Moose::StateArg & time) const override;
  ValueType evaluate(const Moose::FaceArg & face, const Moose::StateArg & time) const override;
  ValueType evaluate(const Moose::ElemQpArg & elem_qp, const Moose::StateArg & time) const override;
  ValueType evaluate(const Moose::ElemSideQpArg & elem_side_qp,
                     const Moose::StateArg & time) const override;
  ValueType evaluate(const Moose::ElemPointArg & elem_point,
                     const Moose::StateArg & time) const override;
  ValueType evaluate(const Moose::NodeArg & elem_point,
                     const Moose::StateArg & time) const override;

protected:
  /**
   * Provide a useful error message about lack of functor material property on the provided
   * subdomain \p sub_id
   */
  void subdomainErrorMessage(SubdomainID sub_id) const;

  /// Functor to evaluate to compute the boundary integral
  const Moose::Functor<T> & _functor;

  /// Boundary id of the boundary to integrate on
  const BoundaryID _bid;

  /// The mesh that this functor operates on
  const MooseMesh & _mesh;
};

template <typename T>
BoundaryIntegralFunctor<T>::BoundaryIntegralFunctor(
    const std::string & name,
    const Moose::FunctorEnvelope<T> & functor,
    const std::set<ExecFlagType> & clearance_schedule,
    const MooseMesh & mesh,
    const BoundaryID boundary_id)
  : Moose::FunctorBase<T>(name, clearance_schedule),
    _functor(functor),
    _bid(boundary_id),
    _mesh(mesh)
{
}

template <typename T>
bool
BoundaryIntegralFunctor<T>::isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                                       const Elem *,
                                                       const Moose::StateArg &) const
{
  if (!fi.neighborPtr())
    return true;

  const bool defined_on_elem = _functor.hasBlocks(fi.elem().subdomain_id());
  const bool defined_on_neighbor = _functor.hasBlocks(fi.neighbor().subdomain_id());
  const bool extrapolated = (defined_on_elem + defined_on_neighbor) == 1;

  mooseAssert(defined_on_elem || defined_on_neighbor,
              "This shouldn't be called if we aren't defined on either side.");
  return extrapolated;
}

template <typename T>
bool
BoundaryIntegralFunctor<T>::hasBlocks(const SubdomainID id) const
{
  return _functor.hasBlocks(id);
}

template <typename T>
void
BoundaryIntegralFunctor<T>::subdomainErrorMessage(const SubdomainID sub_id) const
{
  mooseError("The provided subdomain ID ",
             std::to_string(sub_id),
             " doesn't exist in the map for lambda functor '",
             this->functorName(),
             "'! This is likely because you did not provide a functor "
             "definition on that subdomain");
}

template <typename T>
typename BoundaryIntegralFunctor<T>::ValueType
BoundaryIntegralFunctor<T>::evaluate(const Moose::ElemArg & /*elem_arg*/,
                                     const Moose::StateArg & /*time*/) const
{
  mooseError("ElemArg overload is not defined yet. How would we do the boundary integral?");
}

template <typename T>
typename BoundaryIntegralFunctor<T>::ValueType
BoundaryIntegralFunctor<T>::evaluate(const Moose::FaceArg & face,
                                     const Moose::StateArg & time) const
{
  using namespace Moose::FV;
  auto & binfo = _mesh.getMesh().get_boundary_info();

  // Compute the surface integral
  T sum = 0;
  for (const auto & [elem_id, side_id, bc_id] :
       binfo.build_side_list(libMesh::BoundaryInfo::BCTupleSortBy::BOUNDARY_ID))
  {
    if (bc_id != _bid)
      continue;
    const auto elem = _mesh.elemPtr(elem_id);
    const auto fi = _mesh.faceInfo(elem, side_id);
    mooseAssert(fi, "We should have a face info");
    Moose::FaceArg face_arg = {fi, Moose::FV::LimiterType::CentralDifference, true, true, nullptr};
    sum += _functor(face_arg, time) * fi->faceArea() * fi->faceCoord();
  }

  if (face.face_side)
  {
    const auto sub_id = face.face_side->subdomain_id();
    if (!hasBlocks(sub_id))
      subdomainErrorMessage(sub_id);

    return sum;
  }
  mooseAssert(this->isInternalFace(*face.fi),
              "If we did not have a face side, then we must be an internal face");
  return sum;
}

template <typename T>
typename BoundaryIntegralFunctor<T>::ValueType
BoundaryIntegralFunctor<T>::evaluate(const Moose::ElemQpArg & /*elem_qp*/
                                     ,
                                     const Moose::StateArg & /*time*/) const
{
  mooseError("ElemQpArg argument is not implemented for the BoundaryIntegralFunctor");
}

template <typename T>
typename BoundaryIntegralFunctor<T>::ValueType
BoundaryIntegralFunctor<T>::evaluate(const Moose::ElemSideQpArg & /*elem_side_qp*/,
                                     const Moose::StateArg & /*time*/) const
{
  mooseError("ElemSideQpArg argument is not implemented for the BoundaryIntegralFunctor");
}

template <typename T>
typename BoundaryIntegralFunctor<T>::ValueType
BoundaryIntegralFunctor<T>::evaluate(const Moose::ElemPointArg & /*elem_point_arg*/,
                                     const Moose::StateArg & /*time*/) const
{
  mooseError("ElemPointArg argument is not implemented for the BoundaryIntegralFunctor");
}

template <typename T>
typename BoundaryIntegralFunctor<T>::ValueType
BoundaryIntegralFunctor<T>::evaluate(const Moose::NodeArg & /*node_arg*/,
                                     const Moose::StateArg & /*time*/) const
{
  mooseError("ElemPointArg argument is not implemented for the BoundaryIntegralFunctor");
}

/**
 * A functor that is evaluated on-the-fly via calls to various overloads of \p operator()
 */
template <typename T>
class BoundaryAverageFunctor : public BoundaryIntegralFunctor<T>
{
public:
  BoundaryAverageFunctor(const std::string & name,
                         const Moose::FunctorEnvelope<T> & functor,
                         const std::set<ExecFlagType> & clearance_schedule,
                         const MooseMesh & mesh,
                         const BoundaryID boundary_id);

  virtual ~BoundaryAverageFunctor() = default;

  using typename Moose::FunctorBase<T>::FunctorType;
  using typename Moose::FunctorBase<T>::ValueType;
  using typename Moose::FunctorBase<T>::FunctorReturnType;
  using BoundaryIntegralFunctor<T>::hasBlocks;
  using BoundaryIntegralFunctor<T>::subdomainErrorMessage;
  using BoundaryIntegralFunctor<T>::_mesh;
  using BoundaryIntegralFunctor<T>::_bid;
  using BoundaryIntegralFunctor<T>::_functor;

protected:
  ValueType evaluate(const Moose::FaceArg & face, const Moose::StateArg & time) const override;
};

template <typename T>
BoundaryAverageFunctor<T>::BoundaryAverageFunctor(const std::string & name,
                                                  const Moose::FunctorEnvelope<T> & functor,
                                                  const std::set<ExecFlagType> & clearance_schedule,
                                                  const MooseMesh & mesh,
                                                  const BoundaryID boundary_id)
  : BoundaryIntegralFunctor<T>(name, functor, clearance_schedule, mesh, boundary_id)
{
}

template <typename T>
typename BoundaryAverageFunctor<T>::ValueType
BoundaryAverageFunctor<T>::evaluate(const Moose::FaceArg & face, const Moose::StateArg & time) const
{
  using namespace Moose::FV;
  auto & binfo = _mesh.getMesh().get_boundary_info();

  // Compute the surface integral
  T sum = 0;
  T area = 0;
  for (const auto & [elem_id, side_id, bc_id] :
       binfo.build_side_list(libMesh::BoundaryInfo::BCTupleSortBy::BOUNDARY_ID))
  {
    if (bc_id != _bid)
      continue;
    const auto elem = _mesh.elemPtr(elem_id);
    const auto fi = _mesh.faceInfo(elem, side_id);
    mooseAssert(fi, "We should have a face info");
    Moose::FaceArg face_arg = {fi, Moose::FV::LimiterType::CentralDifference, true, true, nullptr};
    sum += _functor(face_arg, time) * fi->faceArea() * fi->faceCoord();
    area += fi->faceArea() * fi->faceCoord();
  }

  if (face.face_side)
  {
    const auto sub_id = face.face_side->subdomain_id();
    if (!hasBlocks(sub_id))
      subdomainErrorMessage(sub_id);

    return sum / area;
  }
  mooseAssert(this->isInternalFace(*face.fi),
              "If we did not have a face side, then we must be an internal face");
  return sum / area;
}
