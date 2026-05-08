//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVInletOutletScalarBC.h"

#include "FEProblemBase.h"

registerMooseObject("NavierStokesApp", LinearFVInletOutletScalarBC);

InputParameters
LinearFVInletOutletScalarBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription(
      "Adds an inlet-outlet-style BC for linear FV scalar transport. On outflow this behaves "
      "like a zero-gradient / extrapolated outlet; on backflow it switches to a prescribed "
      "boundary value.");
  params.addRequiredParam<SolverVariableName>("u", "The velocity in the x direction.");
  params.addParam<SolverVariableName>("v", "The velocity in the y direction.");
  params.addParam<SolverVariableName>("w", "The velocity in the z direction.");
  params.addParam<MooseFunctorName>(
      "backflow_value",
      "0",
      "The boundary value imposed when the local boundary flow reverses and becomes inflow.");
  params.addParam<bool>(
      "use_two_term_expansion",
      false,
      "If an approximate linear expansion should be used to compute the outlet face value on "
      "outflow.");
  return params;
}

LinearFVInletOutletScalarBC::LinearFVInletOutletScalarBC(const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<const MooseLinearVariableFVReal *>(
        &_fv_problem.getVariable(_tid, getParam<SolverVariableName>("u")))),
    _v_var(parameters.isParamValid("v")
               ? dynamic_cast<const MooseLinearVariableFVReal *>(
                     &_fv_problem.getVariable(_tid, getParam<SolverVariableName>("v")))
               : nullptr),
    _w_var(parameters.isParamValid("w")
               ? dynamic_cast<const MooseLinearVariableFVReal *>(
                     &_fv_problem.getVariable(_tid, getParam<SolverVariableName>("w")))
               : nullptr),
    _backflow_value(getFunctor<Real>("backflow_value")),
    _two_term_expansion(getParam<bool>("use_two_term_expansion"))
{
  if (!_u_var)
    paramError("u", "the u velocity must be a MooseLinearVariableFVReal.");

  _vel_vars.push_back(_u_var);

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be a "
               "MooseLinearVariableFVReal.");
  _vel_vars.push_back(_v_var);

  if (_dim >= 3 && !_w_var)
    paramError("w",
               "In three dimensions, the w velocity must be supplied and it must be a "
               "MooseLinearVariableFVReal.");
  _vel_vars.push_back(_w_var);

  if (_two_term_expansion)
    _var.computeCellGradients();
}

const ElemInfo &
LinearFVInletOutletScalarBC::fluidElemInfo() const
{
  return _current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ? *_current_face_info->neighborInfo()
                                                                    : *_current_face_info->elemInfo();
}

bool
LinearFVInletOutletScalarBC::isBackflow() const
{
  const auto & elem_info = fluidElemInfo();
  const auto state = determineState();
  RealVectorValue cell_velocity;
  for (const auto dim_i : make_range(_dim))
    cell_velocity(dim_i) = _vel_vars[dim_i]->getElemValue(elem_info, state);

  const Real boundary_normal_multiplier =
      _current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ? -1.0 : 1.0;
  const Real normal_velocity = boundary_normal_multiplier * (cell_velocity * _current_face_info->normal());
  return normal_velocity < 0.0;
}

Real
LinearFVInletOutletScalarBC::computeOutflowBoundaryValue() const
{
  const auto & elem_info = fluidElemInfo();
  const auto state = determineState();
  Real boundary_value = _var.getElemValue(elem_info, state);

  if (_two_term_expansion)
    boundary_value += _var.gradSln(elem_info, state) * computeCellToFaceVector();

  return boundary_value;
}

Real
LinearFVInletOutletScalarBC::computeOutflowBoundaryValueRHSContribution() const
{
  if (!_two_term_expansion)
    return 0.0;

  return _var.gradSln(fluidElemInfo(), determineState()) * computeCellToFaceVector();
}

Real
LinearFVInletOutletScalarBC::computeBackflowBoundaryValue() const
{
  return _backflow_value(functorFaceArg(_backflow_value, _current_face_info), determineState());
}

Real
LinearFVInletOutletScalarBC::computeBoundaryValue() const
{
  return isBackflow() ? computeBackflowBoundaryValue() : computeOutflowBoundaryValue();
}

Real
LinearFVInletOutletScalarBC::computeBoundaryNormalGradient() const
{
  if (!isBackflow())
    return 0.0;

  const auto & elem_info = fluidElemInfo();
  const Real distance = computeCellToFaceDistance();
  return (computeBackflowBoundaryValue() - _var.getElemValue(elem_info, determineState())) / distance;
}

Real
LinearFVInletOutletScalarBC::computeBoundaryValueMatrixContribution() const
{
  return isBackflow() ? 0.0 : 1.0;
}

Real
LinearFVInletOutletScalarBC::computeBoundaryValueRHSContribution() const
{
  return isBackflow() ? computeBackflowBoundaryValue() : computeOutflowBoundaryValueRHSContribution();
}

Real
LinearFVInletOutletScalarBC::computeBoundaryGradientMatrixContribution() const
{
  return isBackflow() ? 1.0 / computeCellToFaceDistance() : 0.0;
}

Real
LinearFVInletOutletScalarBC::computeBoundaryGradientRHSContribution() const
{
  return isBackflow() ? computeBackflowBoundaryValue() / computeCellToFaceDistance() : 0.0;
}
