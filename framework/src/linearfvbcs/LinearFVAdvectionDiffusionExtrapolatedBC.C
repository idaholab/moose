//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVAdvectionDiffusionExtrapolatedBC.h"

registerMooseObject("MooseApp", LinearFVAdvectionDiffusionExtrapolatedBC);

InputParameters
LinearFVAdvectionDiffusionExtrapolatedBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription(
      "Adds a boundary condition which calculates the face values and face gradients assuming one "
      "or two term expansions from the cell centroid. This kernel is only compatible "
      "with advection-diffusion problems.");
  params.addParam<bool>(
      "use_two_term_expansion",
      false,
      "If an approximate linear expansion should be used to compute the face value.");
  return params;
}

LinearFVAdvectionDiffusionExtrapolatedBC::LinearFVAdvectionDiffusionExtrapolatedBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _two_term_expansion(getParam<bool>("use_two_term_expansion"))
{
  if (_two_term_expansion)
    _var.computeCellGradients();
}

Real
LinearFVAdvectionDiffusionExtrapolatedBC::computeBoundaryValue() const
{
  // We allow internal boundaries too so we need to check which side we are on
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();

  // By default we approximate the boundary value with the neighboring cell value
  auto boundary_value = _var.getElemValue(*elem_info, determineState());

  // If we request linear extrapolation, we add the gradient term as well
  if (_two_term_expansion)
    boundary_value += _var.gradSln(*elem_info) * computeCellToFaceVector();

  return boundary_value;
}

Real
LinearFVAdvectionDiffusionExtrapolatedBC::computeBoundaryNormalGradient() const
{
  // By default we assume that the face value is the same as the cell center value so we
  // have a zero gradient.
  Real normal_gradient = 0.0;

  // If we request linear extrapolation, we will have a gradient. We use
  if (_two_term_expansion)
  {
    const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
    normal_gradient = _var.gradSln(*elem_info) * _current_face_info->normal();
  }
  return normal_gradient;
}

Real
LinearFVAdvectionDiffusionExtrapolatedBC::computeBoundaryValueMatrixContribution() const
{
  return 1.0;
}

Real
LinearFVAdvectionDiffusionExtrapolatedBC::computeBoundaryValueRHSContribution() const
{
  // If we approximate the face value with the cell value, we
  // don't need to add anything to the right hand side
  Real contribution = 0.0;

  // If we have linear extrapolation, we chose to add the linear term to
  // the right hand side instead of the system matrix.
  if (_two_term_expansion)
  {
    const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                               ? _current_face_info->elemInfo()
                               : _current_face_info->neighborInfo();
    contribution = _var.gradSln(*elem_info) * computeCellToFaceVector();
  }

  return contribution;
}

Real
LinearFVAdvectionDiffusionExtrapolatedBC::computeBoundaryGradientMatrixContribution() const
{
  return 1.0 / computeCellToFaceDistance();
}

Real
LinearFVAdvectionDiffusionExtrapolatedBC::computeBoundaryGradientRHSContribution() const
{
  return computeBoundaryValue() / computeCellToFaceDistance();
}
