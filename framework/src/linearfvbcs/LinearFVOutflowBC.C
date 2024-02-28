//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVOutflowBC.h"

registerMooseObject("MooseApp", LinearFVOutflowBC);

InputParameters
LinearFVOutflowBC::validParams()
{
  InputParameters params = LinearFVBoundaryCondition::validParams();
  params.addClassDescription("Adds a boundary condition which represents a surface with outflowing "
                             "material with a constant velocity.");
  params.addParam<bool>(
      "use_two_term_expansion",
      false,
      "If an approximate linear expansion should be used to compute the face value.");
  params.addRequiredParam<RealVectorValue>("velocity", "The velocity at the outflow surface.");
  return params;
}

LinearFVOutflowBC::LinearFVOutflowBC(const InputParameters & parameters)
  : LinearFVBoundaryCondition(parameters),
    _two_term_expansion(getParam<bool>("use_two_term_expansion")),
    _velocity(getParam<RealVectorValue>("velocity"))
{
  if (_two_term_expansion)
    _var.computeCellGradients();
}

Real
LinearFVOutflowBC::computeBoundaryValue() const
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
LinearFVOutflowBC::computeBoundaryNormalGradient() const
{
  // By default we assume that the face value is the same as the cell center value so we
  // have a zero gradient.
  Real normal_gradient = 0.0;

  // If we request linear extrapolation, we will have a gradient
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
LinearFVOutflowBC::computeBoundaryValueMatrixContribution() const
{
  return 1.0;
}

Real
LinearFVOutflowBC::computeBoundaryValueRHSContribution() const
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
LinearFVOutflowBC::computeBoundaryGradientMatrixContribution() const
{
  return 0.0;
}

Real
LinearFVOutflowBC::computeBoundaryGradientRHSContribution() const
{
  return computeBoundaryNormalGradient();
}

RealVectorValue
LinearFVOutflowBC::computeCellToFaceVector() const
{
  const auto is_on_mesh_boundary = !_current_face_info->neighborPtr();
  const auto defined_on_elem =
      is_on_mesh_boundary ? true : (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM);
  if (is_on_mesh_boundary)
    return _current_face_info->dCN();
  else
    return (_current_face_info->faceCentroid() - (defined_on_elem
                                                      ? _current_face_info->elemCentroid()
                                                      : _current_face_info->neighborCentroid()));
}
