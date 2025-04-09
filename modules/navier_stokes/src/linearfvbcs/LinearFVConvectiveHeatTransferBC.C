//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVConvectiveHeatTransferBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVConvectiveHeatTransferBC);

InputParameters
LinearFVConvectiveHeatTransferBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addRequiredParam<SolverVariableName>(NS::T_fluid, "The fluid temperature variable");
  params.addRequiredParam<SolverVariableName>(NS::T_solid, "The solid/wall temperature variable");
  params.addRequiredParam<MooseFunctorName>("h", "The convective heat transfer coefficient");
  params.addClassDescription(
      "Class describing a convective heat transfer between two domains.");
  return params;
}

LinearFVConvectiveHeatTransferBC::LinearFVConvectiveHeatTransferBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _htc(getFunctor<Real>("h"))
{
  _temp_fluid = dynamic_cast<const MooseLinearVariableFV<Real> *>(&_subproblem.getVariable(_tid, getParam<SolverVariableName>(NS::T_fluid)));
  if (!_temp_fluid)
    mooseError("Not a linearFVvariable!");

  _temp_solid = dynamic_cast<const MooseLinearVariableFV<Real> *>(&_subproblem.getVariable(_tid, getParam<SolverVariableName>(NS::T_solid)));
  if (!_temp_solid)
    mooseError("Not a linearFVvariable!");

  _var_is_fluid = _temp_fluid == &_var;

  // We determine which one of our variable is
  if (_var_is_fluid)
    _rhs_temperature = _temp_solid;
  else
    _rhs_temperature = _temp_fluid;
}

Real
LinearFVConvectiveHeatTransferBC::computeBoundaryValue() const
{
  const auto elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();

  // By default we approximate the boundary value with the neighboring cell value
  auto boundary_value = _var.getElemValue(*elem_info, determineState());

  // If we request linear extrapolation, we add the gradient term as well
  // if (_two_term_expansion)
  //   boundary_value += _var.gradSln(*elem_info) * computeCellToFaceVector();

  return boundary_value;
}

Real
LinearFVConvectiveHeatTransferBC::computeBoundaryNormalGradient() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto elem_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                             ? _current_face_info->elemInfo()
                             : _current_face_info->neighborInfo();

  const auto neighbor_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                             ? _current_face_info->neighborInfo()
                             : _current_face_info->elemInfo();

  const auto fluid_side_elem_info = _var_is_fluid ? elem_info : neighbor_info;
  const auto solid_side_elem_info = _var_is_fluid ? neighbor_info : elem_info;

  const auto state = determineState();

  const bool multiplier = _var_is_fluid ? 1.0 : -1.0;

  return multiplier * _htc(face, state) * (_temp_fluid->getElemValue(*fluid_side_elem_info, state) - _temp_fluid->getElemValue(*solid_side_elem_info, state));
}

Real
LinearFVConvectiveHeatTransferBC::computeBoundaryValueMatrixContribution() const
{
  // There should be no cross flow over this surface, but just in case
  // we add this here.
  return 1.0;
}

Real
LinearFVConvectiveHeatTransferBC::computeBoundaryValueRHSContribution() const
{
  // First approach: normally, we should not expect any cross flow through
  // these boundaries. Just in case someone comes up with something here,
  // we will do an expansion.

  // If we approximate the face value with the cell value, we
  // don't need to add anything to the right hand side
  Real contribution = 0.0;

  // If we have linear extrapolation, we chose to add the linear term to
  // the right hand side instead of the system matrix.
  // if (_two_term_expansion)
  // {
  //   const auto elem_info = _current_face_type == FaceInfo::VarFaceNeighbors::ELEM
  //                              ? _current_face_info->elemInfo()
  //                              : _current_face_info->neighborInfo();
  //   contribution = _var.gradSln(*elem_info) * computeCellToFaceVector();
  // }

  return contribution;
}

Real
LinearFVConvectiveHeatTransferBC::computeBoundaryGradientMatrixContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  return _htc(face, state);
}

Real
LinearFVConvectiveHeatTransferBC::computeBoundaryGradientRHSContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto neighbor_info = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
                             ? _current_face_info->neighborInfo()
                             : _current_face_info->elemInfo();

  return _htc(face, state) * _rhs_temperature->getElemValue(*neighbor_info, state);
}
