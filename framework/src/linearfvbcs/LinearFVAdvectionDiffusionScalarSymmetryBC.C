//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVAdvectionDiffusionScalarSymmetryBC.h"

registerMooseObject("MooseApp", LinearFVAdvectionDiffusionScalarSymmetryBC);

InputParameters
LinearFVAdvectionDiffusionScalarSymmetryBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription("Adds a symmetry boundary condition for a scalar quantity.");
  params.addParam<bool>(
      "use_two_term_expansion",
      false,
      "If an approximate linear expansion should be used to compute the face value.");
  return params;
}

LinearFVAdvectionDiffusionScalarSymmetryBC::LinearFVAdvectionDiffusionScalarSymmetryBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _two_term_expansion(getParam<bool>("use_two_term_expansion"))
{
  if (_two_term_expansion)
    _var.computeCellGradients();
}

Real
LinearFVAdvectionDiffusionScalarSymmetryBC::computeBoundaryValue() const
{
  // We allow internal boundaries too so we need to check which side we are on
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();

  // By default we approximate the boundary value with the neighboring cell value
  auto boundary_value = _var.getElemValue(*elem_info, determineState());

  // If we request linear extrapolation, we add the gradient term as well. We make sure
  // that the zero normal gradient is respected (by subtracting the normal component).
  if (_two_term_expansion)
  {
    const auto cell_gradient = _var.gradSln(*elem_info);
    const auto & normal = _current_face_info->normal();
    const auto d_cf = computeCellToFaceVector();

    const auto correction_vector = (d_cf - (d_cf * normal) * normal);
    boundary_value += cell_gradient * correction_vector;
  }

  return boundary_value;
}

Real
LinearFVAdvectionDiffusionScalarSymmetryBC::computeBoundaryNormalGradient() const
{
  // We don't have this on a symmetry plane
  return 0.0;
}

Real
LinearFVAdvectionDiffusionScalarSymmetryBC::computeBoundaryValueMatrixContribution() const
{
  // No matter if we have a one-term or two-term expansion we will always
  // have a contribution to the matrix
  return 1.0;
}

Real
LinearFVAdvectionDiffusionScalarSymmetryBC::computeBoundaryValueRHSContribution() const
{
  // If we request linear extrapolation, we add the gradient term to the right hand
  // side.
  if (_two_term_expansion)
  {
    // We allow internal boundaries too so we need to check which side we are on
    const auto & elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                 ? _current_face_info->elemInfo()
                                 : _current_face_info->neighborInfo();

    const auto & d_cf = computeCellToFaceVector();
    const auto & normal = _current_face_info->normal();
    const auto correction_vector = (d_cf - (d_cf * normal) * normal);
    const auto & cell_gradient = _var.gradSln(*elem_info);

    return cell_gradient * correction_vector;
  }

  return 0.0;
}

Real
LinearFVAdvectionDiffusionScalarSymmetryBC::computeBoundaryGradientMatrixContribution() const
{
  // Nothing to add here, considering that we have a symmetry condition
  return 0.0;
}

Real
LinearFVAdvectionDiffusionScalarSymmetryBC::computeBoundaryGradientRHSContribution() const
{
  // Nothing to add here, considering that we have a symmetry condition
  return 0.0;
}
