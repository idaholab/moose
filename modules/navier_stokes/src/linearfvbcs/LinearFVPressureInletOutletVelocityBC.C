//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVPressureInletOutletVelocityBC.h"

#include "ElemInfo.h"
#include "FEProblemBase.h"

registerMooseObject("NavierStokesApp", LinearFVPressureInletOutletVelocityBC);

InputParameters
LinearFVPressureInletOutletVelocityBC::validParams()
{
  InputParameters params = LinearFVInletOutletScalarBC::validParams();
  params.addClassDescription(
      "Velocity boundary condition of a pressure-controlled inlet/outlet. On outflow it behaves "
      "like a zero-gradient / extrapolated outlet; on backflow it fixes the tangential velocity "
      "and extrapolates the normal velocity.");
  NS::addLinearFVVelocityVariableParams(params);
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The velocity component that this boundary condition applies to.");
  return params;
}

LinearFVPressureInletOutletVelocityBC::LinearFVPressureInletOutletVelocityBC(
    const InputParameters & parameters)
  : LinearFVInletOutletScalarBC(parameters),
    _dim(_subproblem.mesh().dimension()),
    _velocity_vars(NS::getLinearFVVelocityVariables(*this, _fv_problem, _tid, _dim)),
    _index(getParam<MooseEnum>("momentum_component"))
{
}

Real
LinearFVPressureInletOutletVelocityBC::computeBackflowBoundaryValue() const
{
  const auto & elem_info = NS::linearFVFaceSideElemInfo(*_current_face_info, _current_face_type);
  const auto state = determineState();
  const RealVectorValue normal =
      NS::linearFVOutwardUnitNormal(*_current_face_info, _current_face_type);
  const Real normal_component = normal(_index);
  const Real normal_velocity =
      NS::linearFVCellVelocity(_velocity_vars, _dim, elem_info, state) * normal;
  const Real backflow_tangential_value =
      _backflow_value(functorFaceArg(_backflow_value, _current_face_info), state);

  return normal_component * normal_velocity +
         (1.0 - normal_component * normal_component) * backflow_tangential_value;
}

Real
LinearFVPressureInletOutletVelocityBC::computeBackflowBoundaryValueMatrixContribution() const
{
  // Of u_b,i = n_i (u_P.n) + ..., the part proportional to this component's own cell value is
  // n_i^2 u_P,i; the other components of u_P.n are lagged into the right-hand side
  const Real normal_component =
      NS::linearFVOutwardUnitNormal(*_current_face_info, _current_face_type)(_index);
  return normal_component * normal_component;
}
