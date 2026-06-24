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
#include "MortarContactUtils.h"

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
  params.addParam<bool>("derive_c_from_elasticity",
                        false,
                        "Compute an effective elastic modulus from contact surface material "
                        "properties for use as the c_normal constraint parameter.");
  params.addParam<std::string>(
      "secondary_base_name",
      "",
      "Base name prefix of the elasticity_tensor material property on the secondary body. "
      "Must match the base_name used on the secondary material block that computes the "
      "elasticity tensor.");
  params.addParam<std::string>(
      "primary_base_name",
      "",
      "Base name prefix of the elasticity_tensor material property on the primary body. "
      "Must match the base_name used on the primary material block that computes the "
      "elasticity tensor.");
  params.addParam<bool>(
      "use_automatic_differentiation",
      false,
      "Whether the elasticity tensor material property was declared as an AD property. "
      "Set to true if the material block uses an AD elasticity tensor.");
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
    _derive_c_from_elasticity(getParam<bool>("derive_c_from_elasticity")),
    _use_automatic_differentiation(getParam<bool>("use_automatic_differentiation")),
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

  if (_derive_c_from_elasticity)
  {
    const std::string sec_base = getParam<std::string>("secondary_base_name");
    const std::string pri_base = getParam<std::string>("primary_base_name");
    const std::string sec_name =
        sec_base.empty() ? "elasticity_tensor" : sec_base + "_elasticity_tensor";
    const std::string pri_name =
        pri_base.empty() ? "elasticity_tensor" : pri_base + "_elasticity_tensor";
    if (_use_automatic_differentiation)
      fetchElasticityTensorProperties<true>(sec_name, pri_name);
    else
      fetchElasticityTensorProperties<false>(sec_name, pri_name);
  }
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

void
LMWeightedGapUserObject::initialize()
{
  WeightedGapUserObject::initialize();
  clearDerivedC();
}

void
LMWeightedGapUserObject::finalize()
{
  WeightedGapUserObject::finalize();
  if (_derive_c_from_elasticity)
    finalizeDerivedC();
}

void
LMWeightedGapUserObject::computeQpIProperties()
{
  WeightedGapUserObject::computeQpIProperties();
  accumulateDerivedCIfNeeded();
}

void
LMWeightedGapUserObject::finalizeDerivedC()
{
  Moose::Mortar::Contact::communicateGaps(_dof_to_derived_c,
                                          _subproblem.mesh(),
                                          _nodal,
                                          /*normalize_c=*/true,
                                          _communicator,
                                          /*send_data_back=*/false);
  for (auto & [dof, c_pr] : _dof_to_derived_c)
    c_pr.first /= c_pr.second;
}

void
LMWeightedGapUserObject::accumulateDerivedCIfNeeded()
{
  if (!_derive_c_from_elasticity)
    return;
  if (_use_automatic_differentiation)
    accumulateDerivedC<true>();
  else
    accumulateDerivedC<false>();
}

template <bool is_ad>
void
LMWeightedGapUserObject::fetchElasticityTensorProperties(const std::string & sec_name,
                                                         const std::string & pri_name)
{
  if constexpr (is_ad)
  {
    _elasticity_tensor_secondary_ad = &getGenericMaterialProperty<RankFourTensor, true>(sec_name);
    _elasticity_tensor_primary_ad =
        &getGenericNeighborMaterialProperty<RankFourTensor, true>(pri_name);
  }
  else
  {
    _elasticity_tensor_secondary = &getGenericMaterialProperty<RankFourTensor, false>(sec_name);
    _elasticity_tensor_primary =
        &getGenericNeighborMaterialProperty<RankFourTensor, false>(pri_name);
  }
}

template <bool is_ad>
void
LMWeightedGapUserObject::accumulateDerivedC()
{
  const GenericMaterialProperty<RankFourTensor, is_ad> * sec_ptr;
  const GenericMaterialProperty<RankFourTensor, is_ad> * pri_ptr;
  if constexpr (is_ad)
  {
    sec_ptr = _elasticity_tensor_secondary_ad;
    pri_ptr = _elasticity_tensor_primary_ad;
  }
  else
  {
    sec_ptr = _elasticity_tensor_secondary;
    pri_ptr = _elasticity_tensor_primary;
  }

  const RealVectorValue & n = _normals[_i];
  GenericReal<is_ad> C_nn_sec = 0;
  GenericReal<is_ad> C_nn_pri = 0;
  for (const auto a : make_range(3))
    for (const auto b : make_range(3))
      for (const auto c : make_range(3))
        for (const auto d : make_range(3))
        {
          const Real w = n(a) * n(b) * n(c) * n(d);
          C_nn_sec += w * (*sec_ptr)[_qp](a, b, c, d);
          C_nn_pri += w * (*pri_ptr)[_qp](a, b, c, d);
        }
  const GenericReal<is_ad> C_nn_harm = 2.0 * C_nn_sec * C_nn_pri / (C_nn_sec + C_nn_pri);
  const Real test_weight = (*_test)[_i][_qp] * _qp_factor;
  const auto * const dof = static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i));
  auto & [c_num, c_denom] = _dof_to_derived_c[dof];
  c_num += C_nn_harm * test_weight;
  c_denom += test_weight;
}

template void LMWeightedGapUserObject::fetchElasticityTensorProperties<false>(const std::string &,
                                                                              const std::string &);
template void LMWeightedGapUserObject::fetchElasticityTensorProperties<true>(const std::string &,
                                                                             const std::string &);
template void LMWeightedGapUserObject::accumulateDerivedC<false>();
template void LMWeightedGapUserObject::accumulateDerivedC<true>();
