//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVVelocitySymmetryBC.h"
#include "FEProblemBase.h"

registerMooseObject("NavierStokesApp", LinearFVVelocitySymmetryBC);

InputParameters
LinearFVVelocitySymmetryBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription("Adds a symmetry boundary condition for the velocity.");
  NS::addLinearFVVelocityVariableParams(params);
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
    _velocity_vars(NS::getLinearFVVelocityVariables(*this, _fv_problem, _tid, _dim)),
    _index(getParam<MooseEnum>("momentum_component"))
{
}

Real
LinearFVVelocitySymmetryBC::computeBoundaryValue() const
{
  // We allow internal boundaries too so we need to check which side we are on
  const auto & elem_info = NS::linearFVFaceSideElemInfo(*_current_face_info, _current_face_type);

  // By default we approximate the boundary value with the neighboring cell value
  auto boundary_value = _var.getElemValue(elem_info, determineState());
  auto reflected_boundary_value = boundary_value;

  // We don't have to flip the sign of the normal because we are subtacting normal*normal.
  auto scaled_normal = _current_face_info->normal();
  scaled_normal *= 2 * scaled_normal(_index);

  for (const auto dim_i : make_range(_dim))
    reflected_boundary_value -=
        scaled_normal(dim_i) * _velocity_vars[dim_i]->getElemValue(elem_info, determineState());

  return 0.5 * (boundary_value + reflected_boundary_value);
}

Real
LinearFVVelocitySymmetryBC::computeBoundaryNormalGradient() const
{
  // We allow internal boundaries too so we need to check which side we are on
  const auto & elem_info = NS::linearFVFaceSideElemInfo(*_current_face_info, _current_face_type);

  Real boundary_normal_grad = 0.0;

  // We don't have to flip the sign of the normal because we are subtacting normal*normal.
  auto scaled_normal = _current_face_info->normal();
  scaled_normal *= scaled_normal(_index);

  for (const auto dim_i : make_range(_dim))
    boundary_normal_grad +=
        scaled_normal(dim_i) * _velocity_vars[dim_i]->getElemValue(elem_info, determineState());

  return boundary_normal_grad / computeCellToFaceDistance();
}

Real
LinearFVVelocitySymmetryBC::computeBoundaryValueMatrixContribution() const
{
  // No matter if we have a one-term or two-term expansion we will always
  // have a contribution to the matrix
  const auto normal_component = _current_face_info->normal()(_index);
  const auto normal_component_sq = normal_component * normal_component;
  return 1.0 - normal_component_sq;
}

Real
LinearFVVelocitySymmetryBC::computeBoundaryValueRHSContribution() const
{
  // We allow internal boundaries too so we need to check which side we are on
  const auto & elem_info = NS::linearFVFaceSideElemInfo(*_current_face_info, _current_face_type);

  // We don't have to flip the sign of the normal because we are subtacting normal*normal.
  auto scaled_normal = _current_face_info->normal();
  scaled_normal *= scaled_normal(_index);

  auto current_bd_value = computeBoundaryValue();
  const auto normal_component = _current_face_info->normal()(_index);
  const auto normal_component_sq = normal_component * normal_component;

  return current_bd_value -
         (1.0 - normal_component_sq) * _var.getElemValue(elem_info, determineState());
}

Real
LinearFVVelocitySymmetryBC::computeBoundaryGradientMatrixContribution() const
{
  // We don't have to flip the sign of the normal because we are subtacting normal*normal.
  const auto normal_component = _current_face_info->normal()(_index);
  const auto normal_component_sq = normal_component * normal_component;

  return normal_component_sq / computeCellToFaceDistance();
}

Real
LinearFVVelocitySymmetryBC::computeBoundaryGradientRHSContribution() const
{
  // We allow internal boundaries too so we need to check which side we are on
  const auto & elem_info = NS::linearFVFaceSideElemInfo(*_current_face_info, _current_face_type);

  auto boundary_value = _var.getElemValue(elem_info, determineState());

  // We don't have to flip the sign of the normal because we are subtacting normal*normal.
  const auto normal_component = _current_face_info->normal()(_index);
  const auto normal_component_sq = normal_component * normal_component;
  return computeBoundaryNormalGradient() -
         normal_component_sq / computeCellToFaceDistance() * boundary_value;
}
