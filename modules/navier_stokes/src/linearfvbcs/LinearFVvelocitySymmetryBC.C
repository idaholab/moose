//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVVelocitySymmetryBC.h"

registerMooseObject("MooseApp", LinearFVVelocitySymmetryBC);

InputParameters
LinearFVVelocitySymmetryBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription("Adds a symmetry boundary condition for the velocity.");
  params.addRequiredParam<SolverVariableName>("u", "The velocity in the x direction.");
  params.addParam<SolverVariableName>("v", "The velocity in the y direction.");
  params.addParam<SolverVariableName>("w", "The velocity in the z direction.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");

  return params;
}

LinearFVVelocitySymmetryBC::LinearFVVelocitySymmetryBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<const MooseLinearVariableFVReal *>(
        &_fe_problem.getVariable(_tid, getParam<SolverVariableName>("u")))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const MooseLinearVariableFVReal *>(
                     &_fe_problem.getVariable(_tid, getParam<SolverVariableName>("v")))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const MooseLinearVariableFVReal *>(
                     &_fe_problem.getVariable(_tid, getParam<SolverVariableName>("w")))
               : nullptr),
    _index(getParam<MooseEnum>("momentum_component"))
{
  if (!_u_var)
    paramError("u", "the u velocity must be a MooseLinearVariableFVReal.");

  _vel_vars.psuh_back(_u_var);

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be a "
               "MooseLinearVariableFVReal.");

  _vel_vars.psuh_back(_v_var);

  if (_dim >= 3 && !_w_var)
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be a "
               "MooseLinearVariableFVReal.");

  _vel_vars.psuh_back(_w_var);
}

Real
LinearFVVelocitySymmetryBC::computeBoundaryValue() const
{
  // We allow internal boundaries too so we need to check which side we are on
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();

  // By default we approximate the boundary value with the neighboring cell value
  auto boundary_value = _var.getElemValue(*elem_info, determineState());

  // If we request linear extrapolation, we add the gradient term as well. We make sure
  // that the zer normal gradient is respected (by subtracting the normal component).
  if (_two_term_expansion)
  {
    const auto cell_gradient = _var.gradSln(*elem_info);
    const auto & normal = _current_face_info->normal();
    boundary_value +=
        (cell_gradient - (cell_gradient * normal) * normal) * computeCellToFaceVector();
  }

  return boundary_value;
}

Real
LinearFVVelocitySymmetryBC::computeBoundaryNormalGradient() const
{
  // We don't have this on a symmetry plane
  return 0.0;
}

Real
LinearFVVelocitySymmetryBC::computeBoundaryValueMatrixContribution() const
{
  // No matter if we have a one-term or two-term expansion we will always
  // have a contribution to the matrix
  const auto normal_component = _current_face_info->normal()[_index];
  return 1.0 - normal_component * normal_component;
}

Real
LinearFVVelocitySymmetryBC::computeBoundaryValueRHSContribution() const
{
  // We allow internal boundaries too so we need to check which side we are on
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();

  Real boundary_value_rhs = 1.0;
  auto scaled_normal = _current_face_info->normal();
  scaled_normal *= scaled_normal[_index];

  for (const auto dim_i : make_range(_dim))
    if (dim_i != _index)
      boundary_value_rhs -=
          scaled_normal[dim_i] * _vel_vars[dim_i]->getElemValue(*elem_info, determineState());

  return boundary_value_rhs;
}

Real
LinearFVVelocitySymmetryBC::computeBoundaryGradientMatrixContribution() const
{
  const auto normal_component = _current_face_info->normal()[_index];
  return normal_component * normal_component / computeCellToFaceDistance();
}

Real
LinearFVVelocitySymmetryBC::computeBoundaryGradientRHSContribution() const
{
  // We allow internal boundaries too so we need to check which side we are on
  const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();

  Real boundary_grad_rhs = 0.0;
  auto scaled_normal = _current_face_info->normal();
  scaled_normal *= scaled_normal[_index];

  for (const auto dim_i : make_range(_dim))
    if (dim_i != _index)
      boundary_grad_rhs +=
          scaled_normal[dim_i] * _vel_vars[dim_i]->getElemValue(*elem_info, determineState());

  return boundary_grad_rhs / computeCellToFaceDistance();
}
