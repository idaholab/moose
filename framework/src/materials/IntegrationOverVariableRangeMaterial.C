//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IntegrationOverVariableRangeMaterial.h"
#include "SystemBase.h"

registerMooseObject("MooseApp", IntegrationOverVariableRangeMaterial);
registerMooseObject("MooseApp", ADIntegrationOverVariableRangeMaterial);

template <bool is_ad>
InputParameters
IntegrationOverVariableRangeMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Material object for integrating properties over an integration range of variable values.");
  params.addRequiredParam<std::vector<std::string>>(
      "input_prop_names", "The names of the properties this material will integrate");
  params.addRequiredParam<std::vector<std::string>>(
      "output_prop_names", "The names of the properties this material will output");

  // Integral calculation
  params.addRequiredCoupledVar("integration_variable", "Variable to integrate the properties over");
  params.addRequiredRangeCheckedParam<Real>(
      "integration_dv", "integration_dv>0", "Step size for integration");
  params.addRequiredParam<Real>(
      "variable_reference_value",
      "Reference value to integrate from. One bound of the integral calculation");
  params.addRequiredParam<std::vector<Real>>(
      "prop_reference_values",
      "Output material property values when the variable equals the reference variable value (and "
      "thus the integral term is 0)");

  // Fixed grid mode
  params.addParam<bool>(
      "precompute_property_grid", false, "Whether to pre-compute a grid of property values");
  params.addRequiredParam<Real>("variable_min_grid_value",
                                "Minimum value in the pre-computed grid");
  params.addRequiredParam<Real>("variable_max_grid_value",
                                "Maximum value in the pre-computed grid");

  return params;
}

template <bool is_ad>
IntegrationOverVariableRangeMaterialTempl<is_ad>::IntegrationOverVariableRangeMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _input_names(getParam<std::vector<std::string>>("input_prop_names")),
    _integrated_names(getParam<std::vector<std::string>>("output_prop_names")),
    _num_props(_input_names.size()),
    _var(coupledGenericValue<is_ad>("integration_variable")),
    _var_dofs(coupledDofValues("integration_variable")),
    _writeable_var(writableVariable("integration_variable")),
    _dv(getParam<Real>("integration_dv")),
    _matprop_ref(getParam<std::vector<Real>>("prop_reference_values")),
    _v_ref(getParam<Real>("variable_reference_value")),
    _precompute_matprop_grid(getParam<bool>("precompute_property_grid")),
    _v_grid_min(getParam<Real>("variable_min_grid_value")),
    _v_grid_max(getParam<Real>("variable_max_grid_value"))
{
  // Parameter checks
  if (_integrated_names.size() != _num_props)
    paramError("output_prop_names",
               "Number of output_prop_names must match the number of "
               "input_prop_names");
  if (_matprop_ref.size() != _num_props)
    paramError("prop_reference_values",
               "Number of property reference values must match the number of "
               "input_prop_names");
  if (_precompute_matprop_grid)
    paramError("precompute_property_grid", "Not implemented");

  // Resize data structures
  _properties.resize(_num_props);
  _integrated_properties.resize(_num_props);
  _matprop_grid.resize(_num_props);
  for (const auto i : make_range(_num_props))
    _matprop_grid[i].resize(std::floor(_v_grid_max - _v_grid_min) / _dv);

  // Retrieve input properties, declare output integrated properties
  for (const auto i : make_range(_num_props))
  {
    _properties[i] = &(this->template getGenericMaterialProperty<Real, is_ad>(_input_names[i]));
    _integrated_properties[i] =
        &(this->template declareGenericProperty<Real, is_ad>(_integrated_names[i]));
  }
}

template <bool is_ad>
void
IntegrationOverVariableRangeMaterialTempl<is_ad>::initQpStatefulProperties()
{
  // should not matter
  _qp = 0;
  if (_precompute_matprop_grid)
    computeMatPropGrid(_v_grid_min, _v_grid_max, _dv);
  else
    MaterialBase::initQpStatefulProperties();
}

template <bool is_ad>
void
IntegrationOverVariableRangeMaterialTempl<is_ad>::computeQpProperties()
{
  // Prevent infinite recursion with re-init
  if (_looping)
    return;
  _looping = true;

  // Check range
  if (_v_grid_min > _var[_qp] || _v_grid_max < _var[_qp])
    mooseError("Variable out of range for integration grid: " +
               std::to_string(MetaPhysicL::raw_value(_var[_qp])));

  // Compute a reduced grid since we know the current variable value
  if (!_precompute_matprop_grid && _var[_qp] > _v_ref)
    computeMatPropGrid(_v_ref, MetaPhysicL::raw_value(_var[_qp]), _dv);
  else if (!_precompute_matprop_grid && _var[_qp] <= _v_ref)
    computeMatPropGrid(MetaPhysicL::raw_value(_var[_qp]), _v_ref, _dv);

  // Now integrate each property using the grid of pre-computed input property values
  for (const auto i : make_range(_num_props))
  {
    GenericReal<is_ad> integral = 0;

    const unsigned int n_intervals =
        std::floor(std::abs(MetaPhysicL::raw_value(_var[_qp]) - _v_ref) / _dv);
    const auto sign = _var[_qp] > _v_ref ? 1 : -1;

    // start from the reference
    const unsigned int i_start = std::floor(_v_ref - _v_grid_min) / _dv;

    // Step integration
    for (const auto j : make_range(n_intervals))
      integral += _dv * _matprop_grid[i][i_start + sign * j];

    // Last interval, interval size from the current variable value, and material property at the current value
    // TODO: not second order
    integral += (_var[_qp] - _v_ref - n_intervals * _dv) * (*_properties[i])[_qp];

    (*_integrated_properties[i])[_qp] = _matprop_ref[i] + integral;
  }

  _looping = false;
}

template <bool is_ad>
void
IntegrationOverVariableRangeMaterialTempl<is_ad>::computeMatPropGrid(Real v_min,
                                                                     Real v_max,
                                                                     Real dv)
{
  mooseAssert(_matprop_grid.size() == _num_props, "Grid size is not as expected");
  // Save dof values
  DenseVector<Real> saved_dof_values(_var_dofs.stdVector());
  const auto qp_old = _qp;

  // This only works for a constant monomial
  // const auto dof_index = _current_elem->dof_number(_writeable_var.sys().number(),
  // _writeable_var.number(), 0);

  const auto n_intervals = std::floor((v_max - v_min) / dv);
  const unsigned int i_start = std::floor(v_min - _v_grid_min) / _dv;
  for (const auto j : make_range(n_intervals))
  {
    // Set the variable dof values, at the center of the interval for second order integration
    // Set to to all dofs to make it constant over the element
    DenseVector<Real> new_dofs(saved_dof_values.size(), v_min + (j + 0.5) * _dv);
    _writeable_var.setDofValues(new_dofs);

    // Recompute the material properties
    mooseAssert(_current_elem, "Only implemented for elements");
    // TODO: decide if we call this and trigger the infinite loop OR
    // call materials one by one, and risk missing dependencies
    _fe_problem.reinitMaterials(_current_elem->subdomain_id(), _tid);
    // reinitMaterials will call for all Qps, but material property output only needs 1
    _qp = qp_old;

    // Replace values at indices in grid. Grid always runs from v_grid_min to v_grid_max
    // but some values may not be filled
    for (const auto i : make_range(_num_props))
      _matprop_grid[i][i_start + j] = (*_properties[i])[_qp];
  }

  // Re-init at the current variable value, in case another property uses it
  _writeable_var.setDofValues(saved_dof_values);
  // To optimize: only reinit the properties of interest
  _fe_problem.reinitMaterials(_current_elem->subdomain_id(), _tid);
  _qp = qp_old;
}

template class IntegrationOverVariableRangeMaterialTempl<false>;
template class IntegrationOverVariableRangeMaterialTempl<true>;
