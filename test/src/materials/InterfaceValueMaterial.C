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

template <>
InputParameters
validParams<InterfaceValueMaterial>()
{
  InputParameters params = validParams<InterfaceMaterial>();
  params.addClassDescription("Calculates a variable's jump value across an interface.");
  params.addRequiredParam<std::string>("mat_prop_master",
                                       "The material property on the master side of the interface");
  params.addRequiredParam<std::string>("mat_prop_slave",
                                       "The material property on the slave side of the interface");
  params.addRequiredParam<std::string>("mat_prop_out_basename",
                                       "The base name for the output material property");
  params.addRequiredCoupledVar(
      "var_master",
      "A variable on the master side of the interface that should be equivalent to the value of "
      "the master material property (through MaterialRealAux for example");
  params.addRequiredCoupledVar(
      "var_slave",
      "A variable on the slave side of the interface that should be equivalent to the value of "
      "the slave material property (through MaterialRealAux for example");
  params.addRequiredCoupledVar("nl_var_master",
                               "Master side non-linear variable for jump computation");
  params.addRequiredCoupledVar("nl_var_slave",
                               "Slave side non-linear variable for jump computation");
  params.addRequiredParam<std::string>("mat_prop_var_out_basename",
                                       "The base name for the output material property");
  params.addParam<MooseEnum>("interface_value_type",
                             InterfaceValueTools::InterfaceAverageOptions(),
                             "Type of scalar output");
  return params;
}

InterfaceValueMaterial::InterfaceValueMaterial(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _mp_master_name(getParam<std::string>("mat_prop_master")),
    _mp_slave_name(getParam<std::string>("mat_prop_slave")),
    _mp_master(getMaterialPropertyByName<Real>(_mp_master_name)),
    _mp_slave(getNeighborMaterialPropertyByName<Real>(_mp_slave_name)),
    _var_master(coupledValue("var_master")),
    _var_slave(coupledNeighborValue("var_slave")),
    _nl_var_master(coupledValue("nl_var_master")),
    _nl_var_slave(coupledNeighborValue("nl_var_slave")),

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
    _jump(declareProperty<Real>(std::string(_interface_value_type) + "_jump"))
{
}

void
InterfaceValueMaterial::computeQpProperties()
{
  mooseAssert(_mp_master[_qp] == _var_master[_qp],
              "the material property and variable values on the master side do not coincide.");
  mooseAssert(_mp_slave[_qp] == _var_slave[_qp],
              "the material property and variable values on the slave side do not coincide.");

  _interface_value[_qp] =
      InterfaceValueTools::getQuantity(_interface_value_type, _mp_master[_qp], _mp_slave[_qp]);
  _interface_value_2[_qp] =
      InterfaceValueTools::getQuantity(_interface_value_type, _var_master[_qp], _var_slave[_qp]);
  _jump[_qp] = _nl_var_master[_qp] - _nl_var_slave[_qp];
}

void
InterfaceValueMaterial::initQpStatefulProperties()
{
  _interface_value[_qp] = 0;
  _interface_value_2[_qp] = 0;
}
