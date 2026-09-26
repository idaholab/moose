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
  params.set<bool>("use_nodal_normal_derivatives") = true;
  params.renameCoupledVar("lm_variable", "lm_variable_normal", "");
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

  // Each tangential Lagrange multiplier must share the normal multiplier's finite element type and
  // dual/standard basis choice. tangentialTractionBasis() requires matching node counts and dof
  // numbering for node-for-node interpolation.
  verifyLagrange(
      *_lm_variable_tangential_one, "lm_variable_tangential_one", *_lm_var, "lm_variable_normal");
  if (_lm_variable_tangential_two)
    verifyLagrange(
        *_lm_variable_tangential_two, "lm_variable_tangential_two", *_lm_var, "lm_variable_normal");
}

void
LMWeightedVelocitiesUserObject::initialize()
{
  // Takes care of WeightedGapUserObject::initialize() as well
  WeightedVelocitiesUserObject::initialize();
  initializeNodalScaling();
}

void
LMWeightedVelocitiesUserObject::finalize()
{
  WeightedVelocitiesUserObject::finalize();
  finalizeNodalScaling();
}

void
LMWeightedVelocitiesUserObject::computeQpIProperties()
{
  WeightedVelocitiesUserObject::computeQpIProperties();
  computeQpINodalScaling();
}

ADReal
LMWeightedVelocitiesUserObject::nodalTangentialPressure(const Node & node,
                                                        const unsigned int direction) const
{
  mooseAssert(direction < 2, "There are at most two tangent directions.");
  const auto * const lm_var =
      direction == 0 ? _lm_variable_tangential_one : _lm_variable_tangential_two;
  mooseAssert(lm_var, "The tangential Lagrange multiplier for this direction must exist.");

  const auto sys_num = lm_var->sys().number();
  const auto var_num = lm_var->number();
  mooseAssert(node.n_dofs(sys_num, var_num),
              "The tangential Lagrange multiplier must have a degree of freedom at this node.");

  // Seed the lower-dimensional nodal Lagrange multiplier derivative explicitly. Nodal scaling
  // applies only to the normal complementarity row; the tangential dof value is the physical
  // pressure lambda_t.
  const auto dof_index = node.dof_number(sys_num, var_num, 0);
  ADReal nodal_pressure = (*lm_var->sys().currentSolution())(dof_index);
  if (Moose::doDerivatives(_subproblem, _sys))
    Moose::derivInsert(nodal_pressure.derivatives(), dof_index, 1.);
  return nodal_pressure;
}

const VariableTestValue &
LMWeightedVelocitiesUserObject::tangentialTractionBasis(const unsigned int direction) const
{
  mooseAssert(direction < 2, "There are at most two tangent directions.");
  const auto * const lm_var =
      direction == 0 ? _lm_variable_tangential_one : _lm_variable_tangential_two;
  mooseAssert(lm_var, "The tangential Lagrange multiplier for this direction must exist.");
  return lm_var->phiLower();
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
