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

  // Node-based scaling needs a first-order multiplier; the second-order dual basis has a
  // non-positive per-node normalization (needs the transformed dual basis, out of scope).
  if (_use_nodal_scaling && _lm_var->feType().order != FIRST)
    paramError("use_nodal_scaling",
               "Node-based scaling is only implemented for a first-order Lagrange multiplier.");

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
                                    const std::string & var_param_name) const
{
  if (isCoupledConstant(var_param_name))
    paramError(var_param_name,
               "The Lagrange multiplier variable must be an actual variable and not a constant.");
  else if (!var)
    paramError(var_param_name,
               "The Lagrange multiplier variables must be provided and be actual variables.");
}

void
LMWeightedGapUserObject::verifyLagrange(const MooseVariable & var,
                                        const std::string & var_param_name) const
{
  if (var.feType().family != LAGRANGE)
    paramError(var_param_name, "The Lagrange multiplier variables must be of Lagrange type");
}

const VariableTestValue &
LMWeightedGapUserObject::test() const
{
  return _use_petrov_galerkin ? _aux_lm_var->phiLower() : _lm_var->phiLower();
}

const ADVariableValue &
LMWeightedGapUserObject::contactPressure() const
{
  // Without scaling, hand back the native dual interpolation of the stored Lagrange multiplier.
  // With scaling, return the physical contact pressure recomputed once per segment in reinit().
  return _use_nodal_scaling ? _scaled_contact_pressure : _lm_var->adSlnLower();
}

void
LMWeightedGapUserObject::reinit()
{
  if (!_use_nodal_scaling)
    return;

  // The stored multiplier is scaled (zhat_j = kappa_j lambda_j); interpolate the physical pressure
  // sum_j Phi_j (zhat_j/kappa_j) for the coupling (Popp 2013 eq. 39), cached once per segment.
  // Phi_j is Real, so the multiplier derivatives are seeded exactly.
  const auto & phi = _lm_var->phiLower();
  const Elem * const lower_elem = _assembly.lowerDElem();
  const auto sys_num = _lm_var->sys().number();
  const auto var_num = _lm_var->number();
  const auto & current_solution = *_lm_var->sys().currentSolution();

  const std::size_t n_qp = phi.size() == 0 ? 0 : phi[0].size();
  _scaled_contact_pressure.resize(n_qp);
  for (const auto qp : make_range(n_qp))
    _scaled_contact_pressure[qp] = 0;

  for (const auto j : make_range(lower_elem->n_nodes()))
  {
    const Node * const node = lower_elem->node_ptr(j);
    const auto dof_index = node->dof_number(sys_num, var_num, /*component=*/0);
    ADReal lm_value = current_solution(dof_index);
    Moose::derivInsert(lm_value.derivatives(), dof_index, 1.);
    const ADReal physical_pressure = lm_value / nodalScale(node);
    for (const auto qp : make_range(n_qp))
      _scaled_contact_pressure[qp] += phi[j][qp] * physical_pressure;
  }
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
  // Recover the physical contact pressure lambda_j = zhat_j / kappa_j (nodalScale() is 1 when
  // scaling is disabled), so reported/consumed contact pressures are physical (Popp 2013, eq. 39).
  // nodalScale() is keyed by the displaced-mesh node pointer populated during assembly; callers
  // (e.g. an undisplaced aux kernel) may pass a different pointer of the same id, so map through
  // the mesh to match -- exactly as getNormalGap() does above.
  return (*_lm_var->sys().currentSolution())(dof_number) /
         nodalScale(_subproblem.mesh().nodePtr(node->id()));
}
