//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideIntegralShiftedVariablePostprocessor.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("ShiftedBoundaryMethodApp", SideIntegralShiftedVariablePostprocessor);

InputParameters
SideIntegralShiftedVariablePostprocessor::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredCoupledVar("variable",
                               "The name of the variable which this postprocessor integrates");

  params.addParam<UserObjectName>(
      "sbm_distance_uo",
      "UserObject that provides signed distance and normal vector calculations.");

  // no shifted testing
  params.addParam<bool>(
      "no_shifted", false, "Disable shifted terms (u + grad_u * d and area correction term).");

  params.addClassDescription("Computes a surface integral of the shifted specified variable");
  return params;
}

SideIntegralShiftedVariablePostprocessor::SideIntegralShiftedVariablePostprocessor(
    const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(coupledValue("variable")),
    _grad_u(coupledGradient("variable")),
    _sbm_distance_uo(nullptr),
    _shifted(!getParam<bool>("no_shifted"))
{
  addMooseVariableDependency(&mooseVariableField());
}

void
SideIntegralShiftedVariablePostprocessor::initialSetup()
{
  if (_shifted)
  {
    if (!isParamSetByUser("sbm_distance_uo"))
      mooseError("Please provide 'sbm_distance_uo' when using shifted terms.");
    _sbm_distance_uo = &getUserObject<BoundaryShortestDistanceToSurface>("sbm_distance_uo");
  }
}

Real
SideIntegralShiftedVariablePostprocessor::computeQpIntegral()
{
  const auto elem_side = std::make_pair(_current_elem->id(), _current_side);

  const auto d =
      _shifted ? _sbm_distance_uo->surrogateDistance(elem_side, _qp) : RealVectorValue(0., 0., 0.);

  const auto true_normal =
      _shifted ? _sbm_distance_uo->trueNormal(elem_side, _qp) : RealVectorValue(_normals[_qp]);
  const auto true_dot_surrogate_normal = _shifted ? true_normal * _normals[_qp] : 1.0;

  // When shifted terms are disabled, d is zero and true_dot_surrogate_normal is one,
  // so this expression reduces to the integral of the original variable u.
  return (_u[_qp] + _grad_u[_qp] * d) * true_dot_surrogate_normal;
}
