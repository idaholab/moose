//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LMWeightedGapUserObject.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

registerMooseObject("ContactApp", LMWeightedGapUserObject);

InputParameters
LMWeightedGapUserObject::validParams()
{
  InputParameters params = WeightedGapUserObject::validParams();
  params.addRequiredCoupledVar(
      "lm_variable", "The Lagrange multiplier variable representing the contact pressure.");
  return params;
}

LMWeightedGapUserObject::LMWeightedGapUserObject(const InputParameters & parameters)
  : WeightedGapUserObject(parameters), _lm_var(getVar("lm_variable", 0))
{
  if (!_lm_var)
    paramError("lm_variable",
               "The Lagrange multiplier variable must be an actual variable and not a constant.");

  if (!_lm_var->isNodal())
    paramError("lm_variable",
               "The Lagrange multiplier variable must have its degrees of freedom exclusively on "
               "nodes, e.g. it should probably be of finite element type 'Lagrange'.");
}

const VariableTestValue &
LMWeightedGapUserObject::test() const
{
  return _lm_var->phiLower();
}

const ADVariableValue &
LMWeightedGapUserObject::contactPressure() const
{
  return _lm_var->adSlnLower();
}
