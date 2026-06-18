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

registerMooseObject("NavierStokesApp", LinearFVPressureInletOutletMomentumBC);

InputParameters
LinearFVPressureInletOutletMomentumBC::validParams()
{
  InputParameters params = LinearFVInletOutletScalarBC::validParams();
  params.addClassDescription(
      "Adds a pressure-controlled inlet/outlet boundary condition for velocity components. On "
      "outflow it behaves like a zero-gradient / extrapolated outlet; on backflow it fixes the "
      "tangential velocity and extrapolates the normal velocity.");
  params.addRequiredParam<SolverVariableName>("u", "The velocity in the x direction.");
  params.addParam<SolverVariableName>("v", "The velocity in the y direction.");
  params.addParam<SolverVariableName>("w", "The velocity in the z direction.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The velocity component that this boundary condition applies to.");
  return params;
}

LinearFVPressureInletOutletMomentumBC::LinearFVPressureInletOutletMomentumBC(
    const InputParameters & parameters)
  : LinearFVInletOutletScalarBC(parameters),
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
    _index(getParam<MooseEnum>("momentum_component"))
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
}

RealVectorValue
LinearFVPressureInletOutletMomentumBC::cellVelocity(const ElemInfo & elem_info,
                                                    const Moose::StateArg & state) const
{
  RealVectorValue velocity;
  for (const auto dim_i : make_range(_dim))
    velocity(dim_i) = _velocity_vars[dim_i]->getElemValue(elem_info, state);

  return velocity;
}

RealVectorValue
LinearFVPressureInletOutletMomentumBC::outwardUnitNormal() const
{
  auto normal = _current_face_info->normal();
  if (_current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    normal *= -1.0;

  const Real normal_magnitude = normal.norm();
  if (normal_magnitude <= libMesh::TOLERANCE)
    return RealVectorValue();

  return normal / normal_magnitude;
}

Real
LinearFVPressureInletOutletMomentumBC::computeBackflowBoundaryValue() const
{
  const auto & elem_info = fluidElemInfo();
  const auto state = determineState();
  const RealVectorValue normal = outwardUnitNormal();
  const Real normal_component = normal(_index);
  const Real normal_velocity = cellVelocity(elem_info, state) * normal;
  const Real backflow_tangential_value =
      _backflow_value(functorFaceArg(_backflow_value, *_current_face_info), state);

  return normal_component * normal_velocity +
         (1.0 - normal_component * normal_component) * backflow_tangential_value;
}

Real
LinearFVPressureInletOutletMomentumBC::computeBackflowBoundaryValueMatrixContribution() const
{
  const Real normal_component = outwardUnitNormal()(_index);
  return normal_component * normal_component;
}
