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

InputParameters
InterfaceValueMaterial::validParams()
{
  InputParameters params = InterfaceMaterial::validParams();
  params.addClassDescription("Calculates a variable's jump value across an interface.");
  params.addRequiredParam<std::string>("mat_prop_master",
                                       "The material property on the master side of the interface");
  params.addRequiredParam<std::string>("mat_prop_secondary",
                                       "The material property on the secondary side of the interface");
  params.addRequiredParam<std::string>("mat_prop_out_basename",
                                       "The base name for the output material property");
  params.addRequiredCoupledVar(
      "var_master",
      "A variable on the master side of the interface that should be equivalent to the value of "
      "the master material property (through MaterialRealAux for example");
  params.addRequiredCoupledVar(
      "var_secondary",
      "A variable on the secondary side of the interface that should be equivalent to the value of "
      "the secondary material property (through MaterialRealAux for example");
  params.addRequiredCoupledVar("nl_var_master",
                               "Master side non-linear variable for jump computation");
  params.addRequiredCoupledVar("nl_var_secondary",
                               "Slave side non-linear variable for jump computation");
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

InterfaceValueMaterial::InterfaceValueMaterial(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _mp_master_name(getParam<std::string>("mat_prop_master")),
    _mp_secondary_name(getParam<std::string>("mat_prop_secondary")),
    _mp_master(getMaterialPropertyByName<Real>(_mp_master_name)),
    _mp_secondary(getNeighborMaterialPropertyByName<Real>(_mp_secondary_name)),
    _var_master(coupledValue("var_master")),
    _var_secondary(coupledNeighborValue("var_secondary")),
    _nl_var_master(coupledValue("nl_var_master")),
    _nl_var_secondary(coupledNeighborValue("nl_var_secondary")),
    _couple_old_values_and_properties(getParam<bool>("couple_old_values_and_properties")),
    _mp_master_old(_couple_old_values_and_properties
                       ? &getMaterialPropertyOldByName<Real>(_mp_master_name)
                       : nullptr),
    _mp_secondary_old(_couple_old_values_and_properties
                      ? &getNeighborMaterialPropertyOld<Real>(_mp_secondary_name)
                      : nullptr),
    _var_master_old(_couple_old_values_and_properties ? &coupledValueOld("var_master") : nullptr),
    _var_secondary_old(_couple_old_values_and_properties ? &coupledNeighborValueOld("var_secondary")
                                                     : nullptr),
    _nl_var_master_old(_couple_old_values_and_properties ? &coupledValueOld("nl_var_master")
                                                         : nullptr),
    _nl_var_secondary_old(_couple_old_values_and_properties ? &coupledNeighborValueOld("nl_var_secondary")
                                                        : nullptr),
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

void
InterfaceValueMaterial::computeQpProperties()
{
  mooseAssert(_neighbor_elem, "Neighbor elem is NULL!");
  mooseAssert(_mp_master[_qp] == _var_master[_qp],
              "the material property and variable values on the master side do not coincide.");
  mooseAssert(_mp_secondary[_qp] == _var_secondary[_qp],
              "the material property and variable values on the secondary side do not coincide.");

  _interface_value[_qp] =
      InterfaceValueTools::getQuantity(_interface_value_type, _mp_master[_qp], _mp_secondary[_qp]);
  _interface_value_2[_qp] =
      InterfaceValueTools::getQuantity(_interface_value_type, _var_master[_qp], _var_secondary[_qp]);
  _jump[_qp] = _nl_var_master[_qp] - _nl_var_secondary[_qp];

  if (_couple_old_values_and_properties)
  {
    _interface_value_prev[_qp] = InterfaceValueTools::getQuantity(
        _interface_value_type, (*_mp_master_old)[_qp], (*_mp_secondary_old)[_qp]);
    _interface_value_prev_2[_qp] = InterfaceValueTools::getQuantity(
        _interface_value_type, (*_var_master_old)[_qp], (*_var_secondary_old)[_qp]);

    _jump_prev[_qp] = (*_nl_var_master_old)[_qp] - (*_nl_var_secondary_old)[_qp];
  }
}

void
InterfaceValueMaterial::initQpStatefulProperties()
{
  mooseAssert(_neighbor_elem, "Neighbor elem is NULL!");
  _interface_value[_qp] = 0;
  _interface_value_2[_qp] = 0;
}
