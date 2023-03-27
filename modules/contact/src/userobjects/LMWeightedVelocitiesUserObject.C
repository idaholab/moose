//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LMWeightedVelocitiesUserObject.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

registerMooseObject("ContactApp", LMWeightedVelocitiesUserObject);

InputParameters
LMWeightedVelocitiesUserObject::validParams()
{
  InputParameters params = WeightedVelocitiesUserObject::validParams();
  params.addRequiredCoupledVar(
      "lm_variable_normal",
      "The Lagrange multiplier variable representing the normal contact pressure value.");
  params.addRequiredCoupledVar(
      "lm_variable_tangential_one",
      "The Lagrange multiplier variable representing the tangential contact pressure along the "
      "first tangential direction (the only one in two dimensions).");
  params.addCoupledVar("lm_variable_tangential_two",
                       "The Lagrange multiplier variable representing the tangential contact "
                       "pressure along the second tangential direction.");
  return params;
}

LMWeightedVelocitiesUserObject::LMWeightedVelocitiesUserObject(const InputParameters & parameters)
  : WeightedVelocitiesUserObject(parameters),
    _lm_normal_var(getVar("lm_variable_normal", 0)),
    _lm_variable_tangential_one(getVar("lm_variable_tangential_one", 0)),
    _lm_variable_tangential_two(isParamValid("lm_variable_tangential_two")
                                    ? getVar("lm_variable_tangential_two", 0)
                                    : nullptr)
{
  if (!_lm_normal_var || !_lm_variable_tangential_one)
    paramError("lm_variable_normal", "The Lagrange multiplier variables must be actual variables.");

  auto check_type = [this](const auto & var, const auto & var_name)
  {
    if (!var.isNodal())
      paramError(var_name,
                 "The Lagrange multiplier variables must have degrees of freedom exclusively on "
                 "nodes, e.g. they should probably be of finite element type 'Lagrange'.");
  };
  check_type(*_lm_normal_var, "lm_variable_normal");
  check_type(*_lm_variable_tangential_one, "lm_variable_tangential_one");
  if (_lm_variable_tangential_two)
    check_type(*_lm_variable_tangential_two, "lm_variable_tangential_two");
}

const ADVariableValue &
LMWeightedVelocitiesUserObject::contactTangentialPressureDirOne() const
{
  return _lm_variable_tangential_one->adSlnLower();
}

const ADVariableValue &
LMWeightedVelocitiesUserObject::contactTangentialPressureDirTwo() const
{
  return _lm_variable_tangential_two->adSlnLower();
}

const ADVariableValue &
LMWeightedVelocitiesUserObject::contactPressure() const
{
  return _lm_normal_var->adSlnLower();
}

const VariableTestValue &
LMWeightedVelocitiesUserObject::test() const
{
  return _lm_normal_var->phiLower();
}
