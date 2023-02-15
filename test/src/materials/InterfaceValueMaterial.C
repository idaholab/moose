//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceValueMaterial.h"
#include "InterfaceValueTools.h"

registerMooseObject("MooseTestApp", InterfaceValueMaterial);
registerMooseObject("MooseTestApp", ADInterfaceValueMaterial);

template <bool is_ad>
InputParameters
InterfaceValueMaterialTempl<is_ad>::validParams()
{
  InputParameters params = InterfaceMaterial::validParams();
  params.addClassDescription("Calculates a variable's jump value across an interface.");
  params.addRequiredParam<std::string>(
      "mat_prop_primary", "The material property on the primary side of the interface");
  params.addRequiredParam<std::string>(
      "mat_prop_secondary", "The material property on the secondary side of the interface");
  params.addRequiredParam<std::string>("mat_prop_out_basename",
                                       "The base name for the output material property");
  params.addRequiredCoupledVar(
      "var_primary",
      "A variable on the primary side of the interface that should be equivalent to the value of "
      "the primary material property (through MaterialRealAux for example");
  params.addRequiredCoupledVar(
      "var_secondary",
      "A variable on the secondary side of the interface that should be equivalent to the value of "
      "the secondary material property (through MaterialRealAux for example");
  params.addRequiredCoupledVar("nl_var_primary",
                               "Primary side non-linear variable for jump computation");
  params.addRequiredCoupledVar("nl_var_secondary",
                               "Secondary side non-linear variable for jump computation");
  params.addRequiredParam<std::string>("mat_prop_var_out_basename",
                                       "The base name for the output material property");
  params.addParam<MooseEnum>("interface_value_type",
                             InterfaceValueTools::InterfaceAverageOptions(),
                             "Type of scalar output");
  params.addParam<bool>("couple_old_values_and_properties",
                        false,
                        "get also old variable and material properties values");
  return params;
}

template <bool is_ad>
InterfaceValueMaterialTempl<is_ad>::InterfaceValueMaterialTempl(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _mp_primary_name(getParam<std::string>("mat_prop_primary")),
    _mp_secondary_name(getParam<std::string>("mat_prop_secondary")),
    _mp_primary(getMaterialPropertyByName<Real>(_mp_primary_name)),
    _mp_secondary(getNeighborMaterialPropertyByName<Real>(_mp_secondary_name)),
    _var_primary(coupledGenericValue<is_ad>("var_primary")),
    _var_secondary(coupledGenericNeighborValue<is_ad>("var_secondary")),
    _nl_var_primary(coupledValue("nl_var_primary")),
    _nl_var_secondary(coupledNeighborValue("nl_var_secondary")),
    _couple_old_values_and_properties(getParam<bool>("couple_old_values_and_properties")),
    _mp_primary_old(_couple_old_values_and_properties
                        ? &getMaterialPropertyOldByName<Real>(_mp_primary_name)
                        : nullptr),
    _mp_secondary_old(_couple_old_values_and_properties
                          ? &getNeighborMaterialPropertyOld<Real>(_mp_secondary_name)
                          : nullptr),
    _var_primary_old(_couple_old_values_and_properties ? &coupledValueOld("var_primary") : nullptr),
    _var_secondary_old(_couple_old_values_and_properties ? &coupledNeighborValueOld("var_secondary")
                                                         : nullptr),
    _nl_var_primary_old(_couple_old_values_and_properties ? &coupledValueOld("nl_var_primary")
                                                          : nullptr),
    _nl_var_secondary_old(
        _couple_old_values_and_properties ? &coupledNeighborValueOld("nl_var_secondary") : nullptr),
    _interface_value_type(parameters.get<MooseEnum>("interface_value_type")),
    _mp_out_base_name(getParam<std::string>("mat_prop_out_basename")),
    _mp_var_out_base_name(getParam<std::string>("mat_prop_var_out_basename")),
    _interface_value(
        declareProperty<Real>(_mp_out_base_name + "_" + std::string(_interface_value_type))),
    _interface_value_2(
        declareProperty<Real>(_mp_var_out_base_name + "_" + std::string(_interface_value_type))),
    _interface_value_old(
        getMaterialPropertyOld<Real>(_mp_out_base_name + "_" + std::string(_interface_value_type))),
    _interface_value_2_old(getMaterialPropertyOld<Real>(_mp_var_out_base_name + "_" +
                                                        std::string(_interface_value_type))),
    _interface_value_prev(declareProperty<Real>(_mp_out_base_name + "_" +
                                                std::string(_interface_value_type) + "_prev")),
    _interface_value_prev_2(declareProperty<Real>(_mp_var_out_base_name + "_" +
                                                  std::string(_interface_value_type) + "_prev_2")),
    _jump(declareProperty<Real>(std::string(_interface_value_type) + "_jump")),
    _jump_prev(declareProperty<Real>(std::string(_interface_value_type) + "_jump_prev"))
{
}

template <bool is_ad>
void
InterfaceValueMaterialTempl<is_ad>::computeQpProperties()
{
  mooseAssert(_neighbor_elem, "Neighbor elem is NULL!");
  mooseAssert(_mp_primary[_qp] == _var_primary[_qp],
              "the material property and variable values on the primary side do not coincide.");
  mooseAssert(_mp_secondary[_qp] == _var_secondary[_qp],
              "the material property and variable values on the secondary side do not coincide.");

  _interface_value[_qp] =
      InterfaceValueTools::getQuantity(_interface_value_type, _mp_primary[_qp], _mp_secondary[_qp]);
  _interface_value_2[_qp] =
      InterfaceValueTools::getQuantity(_interface_value_type,
                                       MetaPhysicL::raw_value(_var_primary[_qp]),
                                       MetaPhysicL::raw_value(_var_secondary[_qp]));
  _jump[_qp] = _nl_var_primary[_qp] - _nl_var_secondary[_qp];

  if (_couple_old_values_and_properties)
  {
    _interface_value_prev[_qp] = InterfaceValueTools::getQuantity(
        _interface_value_type, (*_mp_primary_old)[_qp], (*_mp_secondary_old)[_qp]);
    _interface_value_prev_2[_qp] = InterfaceValueTools::getQuantity(
        _interface_value_type, (*_var_primary_old)[_qp], (*_var_secondary_old)[_qp]);

    _jump_prev[_qp] = (*_nl_var_primary_old)[_qp] - (*_nl_var_secondary_old)[_qp];
  }
}

template <bool is_ad>
void
InterfaceValueMaterialTempl<is_ad>::initQpStatefulProperties()
{
  mooseAssert(_neighbor_elem, "Neighbor elem is NULL!");
  _interface_value[_qp] = 0;
  _interface_value_2[_qp] = 0;
}

template class InterfaceValueMaterialTempl<false>;
template class InterfaceValueMaterialTempl<true>;
