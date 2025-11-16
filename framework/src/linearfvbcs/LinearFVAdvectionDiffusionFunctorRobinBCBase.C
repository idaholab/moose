//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVAdvectionDiffusionFunctorRobinBCBase.h"

InputParameters
LinearFVAdvectionDiffusionFunctorRobinBCBase::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  return params;
}

LinearFVAdvectionDiffusionFunctorRobinBCBase::LinearFVAdvectionDiffusionFunctorRobinBCBase(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters)
{
}

Real
LinearFVAdvectionDiffusionFunctorRobinBCBase::computeBoundaryValue() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  mooseAssert(_current_face_type != FaceInfo::VarFaceNeighbors::BOTH,
              "This should not be assigned on an internal face!");
  const auto & elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto state = determineState();

  const auto alpha = getAlpha(face, state);
  const auto beta = getBeta(face, state);
  const auto gamma = getGamma(face, state);

  const auto phi = _var.getElemValue(*elem_info, state);
  const auto grad_phi = _var.gradSln(*elem_info);

  const auto & nhat = _current_face_info->normal();

  const auto d_cf = computeCellToFaceVector(); // vector from boundary cell centre to boundary face
  const auto projection = d_cf * nhat;
  const auto vc = d_cf - (projection * nhat);
  return ((alpha * phi) + (alpha * grad_phi * vc) + (gamma * projection)) /
         (alpha + (beta * projection));
}

Real
LinearFVAdvectionDiffusionFunctorRobinBCBase::computeBoundaryNormalGradient() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto alpha = getAlpha(face, state);
  mooseAssert(!MooseUtils::isZero(alpha), "Alpha should not be 0!");
  const auto beta = getBeta(face, state);
  const auto gamma = getGamma(face, state);
  return (gamma - beta * computeBoundaryValue()) / alpha;
}

// implicit terms for advection kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBCBase::computeBoundaryValueMatrixContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto alpha = getAlpha(face, state);
  const auto beta = getBeta(face, state);
  const auto & nhat = _current_face_info->normal();

  return alpha / (alpha + (beta * computeCellToFaceVector() * nhat));
}

// explicit terms for advection kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBCBase::computeBoundaryValueRHSContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  mooseAssert(_current_face_type != FaceInfo::VarFaceNeighbors::BOTH,
              "This should not be assigned on an internal face!");
  const auto & elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();

  const auto alpha = getAlpha(face, state);
  const auto beta = getBeta(face, state);
  const auto gamma = getGamma(face, state);

  const auto & grad_phi = _var.gradSln(*elem_info);

  const auto & nhat = _current_face_info->normal();

  const auto d_cf = computeCellToFaceVector(); // vector from boundary cell centre to boundary face
  const auto projection = d_cf * nhat;
  const auto vc = d_cf - (projection * nhat); // correction vector for non-orthogonal cells

  return (gamma * projection / (alpha + (beta * projection))) +
         (alpha * grad_phi * vc / (alpha + (beta * projection)));
}

// implicit terms for diffusion kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBCBase::computeBoundaryGradientMatrixContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto alpha = getAlpha(face, state);
  const auto beta = getBeta(face, state);

  const auto & nhat = _current_face_info->normal();

  return beta / (alpha + (beta * computeCellToFaceVector() * nhat));
}

// explicit terms for diffusion kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBCBase::computeBoundaryGradientRHSContribution() const
{
  mooseAssert(_current_face_type != FaceInfo::VarFaceNeighbors::BOTH,
              "This should not be assigned on an internal face!");
  const auto & elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto & grad_phi = _var.gradSln(*elem_info);

  const auto alpha = getAlpha(face, state);
  const auto beta = getBeta(face, state);
  const auto gamma = getGamma(face, state);

  const auto & nhat = _current_face_info->normal();

  const auto d_cf = computeCellToFaceVector(); // vector from boundary cell centre to boundary face
  const auto projection = d_cf * nhat;
  const auto vc = d_cf - (projection * nhat); // correction vector for non-orthogonal cells

  return (gamma / alpha) + (-beta * gamma * projection / alpha / (alpha + (beta * projection))) +
         (-beta * grad_phi * vc / (alpha + (beta * projection)));
}
