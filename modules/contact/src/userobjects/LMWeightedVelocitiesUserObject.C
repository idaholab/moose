//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params += LMWeightedGapUserObject::newParams();
  params.addClassDescription("Provides the mortar contact Lagrange multipliers (normal and "
                             "tangential) for constraint enforcement.");
  params.renameParam("lm_variable", "lm_variable_normal", "");
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
  : WeightedGapUserObject(parameters),
    WeightedVelocitiesUserObject(parameters),
    LMWeightedGapUserObject(parameters),
    _lm_variable_tangential_one(getVar("lm_variable_tangential_one", 0)),
    _lm_variable_tangential_two(isParamValid("lm_variable_tangential_two")
                                    ? getVar("lm_variable_tangential_two", 0)
                                    : nullptr)
{
  // Check that user inputted a variable
  checkInput(_lm_variable_tangential_one, "lm_variable_tangential_one");
  if (_lm_variable_tangential_two)
    checkInput(_lm_variable_tangential_two, "lm_variable_tangential_two");

  // Check that user inputted the right type of variable
  verifyNodal(*_lm_variable_tangential_one, "lm_variable_tangential_one");
  if (_lm_variable_tangential_two)
    verifyNodal(*_lm_variable_tangential_two, "lm_variable_tangential_two");
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
