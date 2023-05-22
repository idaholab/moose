//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapHeatConductanceTest.h"
#include "Function.h"
#include "Assembly.h"

using namespace Moose;

registerMooseObject("MooseTestApp", GapHeatConductanceTest);

InputParameters
GapHeatConductanceTest::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  params.addParam<MaterialPropertyName>(
      "secondary_gap_conductance",
      "gap_conductance",
      "The material property name providing the gap conductance on the secondary side");
  params.addParam<MaterialPropertyName>(
      "primary_gap_conductance",
      "gap_conductance",
      "The material property name providing the gap conductance on the primary side");
  params.addParam<FunctionName>(
      "secondary_mms_function", 0, "An mms function to apply to the secondary side");
  params.addParam<FunctionName>(
      "primary_mms_function", 0, "An mms function to apply to the primary side");
  params.addParam<bool>("functor_evals_for_primal",
                        false,
                        "Whether to perform elem-point functor evaluations of the primal variables "
                        "as opposed to indexing pre-evaluated quadrature point data");
  return params;
}

GapHeatConductanceTest::GapHeatConductanceTest(const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _secondary_gap_conductance(getADMaterialProperty<Real>("secondary_gap_conductance")),
    _primary_gap_conductance(getNeighborADMaterialProperty<Real>("primary_gap_conductance")),
    _secondary_mms_function(getFunction("secondary_mms_function")),
    _primary_mms_function(getFunctionByName(getParam<FunctionName>("primary_mms_function"))),
    _functor_evals_for_primal(getParam<bool>("functor_evals_for_primal"))
{
}

ADReal
GapHeatConductanceTest::computeQpResidual(MortarType type)
{
  switch (type)
  {
    case MortarType::Secondary:
      return (_lambda[_qp] + _secondary_mms_function.value(_t, _phys_points_secondary[_qp])) *
             _test_secondary[_i][_qp];

    case MortarType::Primary:
      return (-_lambda[_qp] + _primary_mms_function.value(_t, _phys_points_primary[_qp])) *
             _test_primary[_i][_qp];

    case MortarType::Lower:
    {
      ADReal heat_transfer_coeff(0);
      auto gap = (_phys_points_secondary[_qp] - _phys_points_primary[_qp]).norm();
      mooseAssert(MetaPhysicL::raw_value(gap) > TOLERANCE * TOLERANCE,
                  "Gap distance is too small in GapHeatConductanceTest");

      heat_transfer_coeff =
          (0.5 * (_secondary_gap_conductance[_qp] + _primary_gap_conductance[_qp])) / gap;

      const auto u_secondary =
          _functor_evals_for_primal
              ? _secondary_var(
                    ElemPointArg({_interior_secondary_elem, _phys_points_secondary[_qp], false}),
                    Moose::currentState())
              : _u_secondary[_qp];
      const auto u_primary =
          _functor_evals_for_primal
              ? _primary_var(
                    ElemPointArg({_interior_primary_elem, _phys_points_primary[_qp], false}),
                    Moose::currentState())
              : _u_primary[_qp];
      return _test[_i][_qp] * (_lambda[_qp] - heat_transfer_coeff * (u_secondary - u_primary));
    }

    default:
      return 0;
  }
}

void
GapHeatConductanceTest::computeJacobian(MortarType mortar_type)
{
  std::vector<DualReal> residuals;
  size_t test_space_size = 0;
  typedef ConstraintJacobianType JType;
  typedef MortarType MType;
  std::vector<JType> jacobian_types;
  std::vector<dof_id_type> dof_indices;
  Real scaling_factor = 1;

  switch (mortar_type)
  {
    case MType::Secondary:
      dof_indices = _secondary_var.dofIndices();
      jacobian_types = {JType::SecondarySecondary, JType::SecondaryPrimary, JType::SecondaryLower};
      scaling_factor = _secondary_var.scalingFactor();
      break;

    case MType::Primary:
      dof_indices = _primary_var.dofIndicesNeighbor();
      jacobian_types = {JType::PrimarySecondary, JType::PrimaryPrimary, JType::PrimaryLower};
      scaling_factor = _primary_var.scalingFactor();
      break;

    case MType::Lower:
      if (_var)
        dof_indices = _var->dofIndicesLower();
      jacobian_types = {JType::LowerSecondary, JType::LowerPrimary, JType::LowerLower};
      scaling_factor = _var->scalingFactor();
      break;
  }
  test_space_size = dof_indices.size();

  residuals.resize(test_space_size, 0);
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
    for (_i = 0; _i < test_space_size; _i++)
      residuals[_i] += _JxW_msm[_qp] * _coord[_qp] * computeQpResidual(mortar_type);

  // Trim interior node variable derivatives
  const auto & primary_ip_lowerd_map = amg().getPrimaryIpToLowerElementMap(
      *_lower_primary_elem, *_lower_primary_elem->interior_parent(), *_lower_secondary_elem);
  const auto & secondary_ip_lowerd_map =
      amg().getSecondaryIpToLowerElementMap(*_lower_secondary_elem);
  const std::array<const MooseVariableField<Real> *, 2> secondary_side_var_array = {
      &_secondary_var};
  const std::array<const MooseVariableField<Real> *, 2> primary_side_var_array = {&_primary_var};

  if (_secondary_var.feType().family == LAGRANGE)
    trimInteriorNodeDerivatives(secondary_ip_lowerd_map, secondary_side_var_array, residuals, true);
  if (_primary_var.feType().family == LAGRANGE)
    trimInteriorNodeDerivatives(primary_ip_lowerd_map, primary_side_var_array, residuals, false);
  addResidualsAndJacobianWithoutConstraints(_assembly, residuals, dof_indices, scaling_factor);
}
