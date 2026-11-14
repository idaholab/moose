//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVLeastSquares.h"
#include "FVUtils.h"

namespace NS
{
const FaceInfo *
faceInfoForElementSide(const MooseMesh & mesh, const Elem * const elem, const unsigned int side)
{
  if (!elem)
    return nullptr;

  const Elem * const loc_neighbor = elem->neighbor_ptr(side);
  if (loc_neighbor == libMesh::remote_elem)
    return nullptr;

  const bool elem_has_fi = Moose::FV::elemHasFaceInfo(*elem, loc_neighbor);
  if (!elem_has_fi && !loc_neighbor)
    return nullptr;

  return mesh.faceInfo(elem_has_fi ? elem : loc_neighbor,
                       elem_has_fi ? side : loc_neighbor->which_neighbor_am_i(elem));
}

Real
faceNormalScalarGradient(const Moose::Functor<Real> & functor,
                         const FaceInfo & fi,
                         const Elem * const boundary_elem,
                         const bool face_is_internal,
                         const Moose::StateArg & time_arg)
{
  if (face_is_internal)
    return faceNormalScalarGradient(
        functor,
        fi,
        time_arg,
        MetaPhysicL::raw_value(functor.gradient(Moose::ElemArg{fi.elemPtr(), false}, time_arg)),
        MetaPhysicL::raw_value(
            functor.gradient(Moose::ElemArg{fi.neighborPtr(), false}, time_arg)));

  // On an uncoupled boundary patch the explicit non-orthogonal correction is zero and the
  // operative snGrad comes from the boundary patch field.
  return MetaPhysicL::raw_value(
             functor.gradient(Moose::FaceArg{&fi,
                                             Moose::FV::LimiterType::CentralDifference,
                                             true,
                                             false,
                                             boundary_elem,
                                             nullptr},
                              time_arg)) *
         fi.normal();
}

Point
faceNonOrthogonalCorrection(const FaceInfo & fi)
{
  const Point delta = fi.dCN();
  const Real non_orth_delta_coeff = 1.0 / std::max(fi.normal() * delta, 0.05 * delta.norm());
  return fi.normal() - delta * non_orth_delta_coeff;
}

Real
faceNormalScalarGradient(const Moose::Functor<Real> & functor,
                         const FaceInfo & fi,
                         const Moose::StateArg & time_arg,
                         const RealVectorValue & elem_grad,
                         const RealVectorValue & neighbor_grad)
{
  const Real elem_value = functor(Moose::ElemArg{fi.elemPtr(), false}, time_arg);
  const Real neighbor_value = functor(Moose::ElemArg{fi.neighborPtr(), false}, time_arg);
  const Point delta = fi.dCN();
  const Real delta_mag = delta.norm();

  if (delta_mag <= libMesh::TOLERANCE)
    return 0.0;

  const Real value_jump = neighbor_value - elem_value;
  const Real non_orth_delta_coeff = 1.0 / std::max(fi.normal() * delta, 0.05 * delta_mag);
  const Real base_part = non_orth_delta_coeff * value_jump;

  const Point non_orth_correction_vector = fi.normal() - delta * non_orth_delta_coeff;
  if (non_orth_correction_vector.norm_sq() == 0.0)
    return base_part;
  RealVectorValue face_grad = 0;
  Moose::FV::interpolate(
      Moose::FV::InterpMethod::Average, face_grad, elem_grad, neighbor_grad, fi, true);
  return base_part + non_orth_correction_vector * face_grad;
}
}
