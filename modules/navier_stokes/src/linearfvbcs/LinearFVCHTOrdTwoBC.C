//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVCHTOrdTwoBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVCHTOrdTwoBC);

InputParameters
LinearFVCHTOrdTwoBC::validParams()
{
  InputParameters params = LinearFVConjugateHeatTransferBC::validParams();
  params.addClassDescription("Second order robin bc based......");
  return params;
}

LinearFVCHTOrdTwoBC::LinearFVCHTOrdTwoBC(
    const InputParameters & parameters)
  :  LinearFVConjugateHeatTransferBC(parameters)
{
  LinearFVConjugateHeatTransferBC::LinearFVConjugateHeatTransferBC(params);
  LinearFVAdvectionDiffusionFunctorRobinBC::LinearFVAdvectionDiffusionFunctorRobinBC(params);
}

void
updateCoefficients()
{
  // We check where the functor contributing to the right hand side lives. We do this
  // because this functor lives on the domain where the variable of this kernel doesn't.
  const auto state = determineState();
  auto face = singleSidedFaceArg(_current_face_info);

  if (_rhs_temperature->hasFaceSide(*_current_face_info, true))
    face.face_side = _current_face_info->elemPtr();
  else
    face.face_side = _current_face_info->neighborPtr();

  const auto elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();

  // const auto multiplier = _current_face_info->normal() * (_current_face_info->faceCentroid() -
  //                                                         fluid_side_elem_info->centroid()) >
  //                                 0
  //                             ? 1
  //                             : -1;
  const auto t_coupled = (*_rhs_temperature)(face, state) -
              (  (*_rhs_conductivity)(face,state) *
                  (*_rhs_temperature).gradient(face,state)*_current_face_info->normal()
              / _htc(face, state));

  _cht_gamma =  _htc(face, state) * t_coupled;
  _cht_alpha = _var_is_fluid ? _fluid_conductivity (face, state): _solid_conductivity(face, state);
  _cht_beta  = _htc(face, state);
}

Real
LinearFVCHTOrdTwoBC::computeBoundaryValue() const
{
  updateCoefficients();
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto & elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto state = determineState();

  const auto alpha = _cht_alpha;
  const auto beta =  _beta(face, state);
  const auto gamma = _gamma(face, state);

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
LinearFVCHTOrdTwoBC::computeBoundaryNormalGradient() const
{
  updateCoefficients();
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto alpha = _cht_alpha;
  mooseAssert(alpha != 0.0, "Alpha should not be 0!");
  const auto beta = _cht_beta;
  const auto gamma = _gamma(face, state);
  return (gamma - beta * computeBoundaryValue()) / alpha;
}

Real
LinearFVCHTOrdTwoBC::computeBoundaryValueMatrixContribution() const
{
  updateCoefficients();
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto alpha = _cht_alpha;
  const auto beta = _cht_beta;
  const auto & nhat = _current_face_info->normal();

  return alpha / (alpha + (beta * computeCellToFaceVector() * nhat));
}

Real
LinearFVCHTOrdTwoBC::computeBoundaryValueRHSContribution() const
{
  updateCoefficients();
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto & elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto alpha = _cht_alpha;
  const auto beta = _cht_beta;
  const auto gamma = _gamma(face, state);
  const auto & grad_phi = _var.gradSln(*elem_info);

  const auto & nhat = _current_face_info->normal();

  const auto d_cf = computeCellToFaceVector(); // vector from boundary cell centre to boundary face
  const auto projection = d_cf * nhat;
  const auto vc = d_cf - (projection * nhat); // correction vector for non-orthogonal cells

  return (gamma * projection / (alpha + (beta * projection))) +
         (alpha * grad_phi * vc / (alpha + (beta * projection)));
}

Real
LinearFVCHTOrdTwoBC::computeBoundaryGradientMatrixContribution() const
{
  updateCoefficients();
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto alpha = _cht_alpha;
  const auto beta = _cht_beta;
  const auto & nhat = _current_face_info->normal();

  return beta / (alpha + (beta * computeCellToFaceVector() * nhat));
}

Real
LinearFVCHTOrdTwoBC::computeBoundaryGradientRHSContribution() const
{
  updateCoefficients();
  const auto & elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto & grad_phi = _var.gradSln(*elem_info);

  const auto alpha = _cht_alpha;
  const auto beta = _cht_beta;
  const auto gamma = _gamma(face, state);

  const auto & nhat = _current_face_info->normal();

  const auto d_cf = computeCellToFaceVector(); // vector from boundary cell centre to boundary face
  const auto projection = d_cf * nhat;
  const auto vc = d_cf - (projection * nhat); // correction vector for non-orthogonal cells

  return (gamma / alpha) + (-beta * gamma * projection / alpha / (alpha + (beta * projection))) +
         (-beta * grad_phi * vc / (alpha + (beta * projection)));
}
