//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
LMWeightedGapUserObject::newParams()
{
  auto params = emptyInputParameters();
  params.addRequiredCoupledVar(
      "lm_variable", "The Lagrange multiplier variable representing the contact pressure.");
  params.addParam<bool>(
      "use_petrov_galerkin", false, "Whether to use the Petrov-Galerkin approach.");
  params.addCoupledVar("aux_lm",
                       "Auxiliary Lagrange multiplier variable that is utilized together with the "
                       "Petrov-Galerkin approach.");
  return params;
}

InputParameters
LMWeightedGapUserObject::validParams()
{
  InputParameters params = WeightedGapUserObject::validParams();
  params.addClassDescription(
      "Provides the mortar normal Lagrange multiplier for constraint enforcement.");
  params += LMWeightedGapUserObject::newParams();
  return params;
}

LMWeightedGapUserObject::LMWeightedGapUserObject(const InputParameters & parameters)
  : WeightedGapUserObject(parameters),
    _lm_var(getVar("lm_variable", 0)),
    _use_petrov_galerkin(getParam<bool>("use_petrov_galerkin")),
    _aux_lm_var(isCoupled("aux_lm") ? getVar("aux_lm", 0) : nullptr)
{
  checkInput(_lm_var, "lm_variable");
  verifyLagrange(*_lm_var, "lm_variable");

  if (_use_petrov_galerkin && ((!isParamValid("aux_lm")) || _aux_lm_var == nullptr))
    paramError("use_petrov_galerkin",
               "We need to specify an auxiliary variable `aux_lm` while using the Petrov-Galerkin "
               "approach");

  if (_use_petrov_galerkin && _aux_lm_var->useDual())
    paramError("aux_lm",
               "Auxiliary LM variable needs to use standard shape function, i.e., set `use_dual = "
               "false`.");
}

void
LMWeightedGapUserObject::checkInput(const MooseVariable * const var,
                                    const std::string & var_name) const
{
  if (isCoupledConstant(var_name))
    paramError(var_name,
               "The Lagrange multiplier variable must be an actual variable and not a constant.");
  else if (!var)
    paramError(var_name,
               "The Lagrange multiplier variables must be provided and be actual variables.");
}

void
LMWeightedGapUserObject::verifyLagrange(const MooseVariable & var,
                                        const std::string & var_name) const
{
  if (var.feType().family != LAGRANGE)
    paramError(var_name, "The Lagrange multiplier variables must be of Lagrange type");
}

const VariableTestValue &
LMWeightedGapUserObject::test() const
{
  return _use_petrov_galerkin ? _aux_lm_var->phiLower() : _lm_var->phiLower();
}

const ADVariableValue &
LMWeightedGapUserObject::contactPressure() const
{
  return _lm_var->adSlnLower();
}

Real
LMWeightedGapUserObject::getNormalContactPressure(const Node * const node) const
{
  const auto sys_num = _lm_var->sys().number();
  const auto var_num = _lm_var->number();
  if (!node->n_dofs(sys_num, var_num))
    mooseError("No degrees of freedom for the Lagrange multiplier at the node. If this is being "
               "called from an aux kernel make sure that your aux variable has the same order as "
               "your Lagrange multiplier");

  const auto dof_number = node->dof_number(sys_num, var_num, /*component=*/0);
  return (*_lm_var->sys().currentSolution())(dof_number);
}
