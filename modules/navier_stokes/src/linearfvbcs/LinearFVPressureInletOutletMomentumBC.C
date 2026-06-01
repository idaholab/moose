//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVPressureInletOutletMomentumBC.h"

#include "ElemInfo.h"
#include "FEProblemBase.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVPressureInletOutletMomentumBC);

InputParameters
LinearFVPressureInletOutletMomentumBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription(
      "Adds an reference-solver-style pressure-inlet-outlet boundary condition for velocity components. "
      "On outflow it behaves like a zero-gradient / extrapolated outlet; on backflow it switches "
      "to a prescribed velocity value.");
  params.addRequiredParam<SolverVariableName>("u", "The velocity in the x direction.");
  params.addParam<SolverVariableName>("v", "The velocity in the y direction.");
  params.addParam<SolverVariableName>("w", "The velocity in the z direction.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The velocity component that this boundary condition applies to.");
  params.addParam<MooseFunctorName>(
      "backflow_value",
      "0",
      "The backflow velocity imposed when the local boundary flow reverses and becomes inflow.");
  params.addParam<MooseFunctorName>(
      NS::density,
      "1",
      "Deprecated compatibility parameter. The boundary condition now acts directly on velocity.");
  params.addParam<MooseFunctorName>(
      "density_gradient_functor",
      "0",
      "Deprecated compatibility parameter. The boundary condition now acts directly on velocity.");
  params.addParam<Real>(
      "minimum_density",
      0.0,
      "Deprecated compatibility parameter. The boundary condition now acts directly on velocity.");
  params.addParam<bool>(
      "use_two_term_expansion",
      false,
      "If an approximate linear expansion should be used to compute the outlet face value on "
      "outflow.");
  return params;
}

LinearFVPressureInletOutletMomentumBC::LinearFVPressureInletOutletMomentumBC(
    const InputParameters & parameters)
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
    _index(getParam<MooseEnum>("momentum_component")),
    _backflow_value(getFunctor<Real>("backflow_value")),
    _two_term_expansion(getParam<bool>("use_two_term_expansion"))
{
  if (!_u_var)
    paramError("u", "the u velocity variable must be a MooseLinearVariableFVReal.");

  _velocity_vars.push_back(_u_var);

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity variable must be supplied and it must "
               "be a MooseLinearVariableFVReal.");
  _velocity_vars.push_back(_v_var);

  if (_dim >= 3 && !_w_var)
    paramError("w",
               "In three dimensions, the w velocity variable must be supplied and it must be a "
               "MooseLinearVariableFVReal.");
  _velocity_vars.push_back(_w_var);

  if (_two_term_expansion)
    _var.computeCellGradients();
}

const ElemInfo &
LinearFVPressureInletOutletMomentumBC::fluidElemInfo() const
{
  return _current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ? *_current_face_info->neighborInfo()
                                                                    : *_current_face_info->elemInfo();
}

Real
LinearFVPressureInletOutletMomentumBC::computeVelocity(const ElemInfo & elem_info,
                                                       const Moose::StateArg & state) const
{
  return _var.getElemValue(elem_info, state);
}

RealGradient
LinearFVPressureInletOutletMomentumBC::computeVelocityGradient(const ElemInfo & elem_info,
                                                               const Moose::StateArg & state) const
{
  return _var.gradSln(elem_info, state);
}

bool
LinearFVPressureInletOutletMomentumBC::isBackflow() const
{
  const auto & elem_info = fluidElemInfo();
  const auto state = determineState();

  RealVectorValue cell_velocity;
  for (const auto dim_i : make_range(_dim))
    cell_velocity(dim_i) = _velocity_vars[dim_i]->getElemValue(elem_info, state);

  const Real boundary_normal_multiplier =
      _current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ? -1.0 : 1.0;
  const Real normal_velocity =
      boundary_normal_multiplier * (cell_velocity * _current_face_info->normal());
  return normal_velocity < 0.0;
}

Real
LinearFVPressureInletOutletMomentumBC::computeOutflowBoundaryValue() const
{
  const auto & elem_info = fluidElemInfo();
  const auto state = determineState();
  Real boundary_velocity = computeVelocity(elem_info, state);

  if (_two_term_expansion)
    boundary_velocity += computeVelocityGradient(elem_info, state) * computeCellToFaceVector();

  return boundary_velocity;
}

Real
LinearFVPressureInletOutletMomentumBC::computeOutflowBoundaryValueRHSContribution() const
{
  if (!_two_term_expansion)
    return 0.0;

  return computeVelocityGradient(fluidElemInfo(), determineState()) * computeCellToFaceVector();
}

Real
LinearFVPressureInletOutletMomentumBC::computeBackflowBoundaryValue() const
{
  return _backflow_value(functorFaceArg(_backflow_value, _current_face_info), determineState());
}

Real
LinearFVPressureInletOutletMomentumBC::computeBackflowBoundaryValueMatrixContribution() const
{
  return 1.0;
}

Real
LinearFVPressureInletOutletMomentumBC::computeBoundaryValue() const
{
  return isBackflow() ? computeBackflowBoundaryValue() : computeOutflowBoundaryValue();
}

Real
LinearFVPressureInletOutletMomentumBC::computeBoundaryNormalGradient() const
{
  if (!isBackflow())
    return 0.0;

  const auto & elem_info = fluidElemInfo();
  const Real distance = computeCellToFaceDistance();
  return (computeBackflowBoundaryValue() -
          computeBackflowBoundaryValueMatrixContribution() * _var.getElemValue(elem_info, determineState())) /
         distance;
}

Real
LinearFVPressureInletOutletMomentumBC::computeBoundaryValueMatrixContribution() const
{
  if (isBackflow())
    return 0.0;

  return 1.0;
}

Real
LinearFVPressureInletOutletMomentumBC::computeBoundaryValueRHSContribution() const
{
  return isBackflow() ? computeBackflowBoundaryValue() : computeOutflowBoundaryValueRHSContribution();
}

Real
LinearFVPressureInletOutletMomentumBC::computeBoundaryGradientMatrixContribution() const
{
  return isBackflow() ? computeBackflowBoundaryValueMatrixContribution() / computeCellToFaceDistance()
                      : 0.0;
}

Real
LinearFVPressureInletOutletMomentumBC::computeBoundaryGradientRHSContribution() const
{
  return isBackflow() ? computeBackflowBoundaryValue() / computeCellToFaceDistance() : 0.0;
}
