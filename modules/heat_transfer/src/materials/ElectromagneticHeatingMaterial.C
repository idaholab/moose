//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElectromagneticHeatingMaterial.h"
#include "FormulationEnums.h"

registerMooseObject("HeatTransferApp", ElectromagneticHeatingMaterial);

InputParameters
ElectromagneticHeatingMaterial::validParams()
{
  InputParameters params = ADMaterial::validParams();
  params.addClassDescription(
      "Material class used to provide the electric field as a material property and computes the "
      "residuals for electromagnetic/electrostatic heating based objects.");
  params.addCoupledVar(
      "electric_field",
      "The electic field vector or electrostatic potential scalar to produce the field.");
  params.addCoupledVar(
      "complex_electric_field",
      "The complex component of the electic field vector for the harmonic formulation.");
  params.addParam<std::string>("electric_field_material_name",
                               "electric_field",
                               "User-specified material property name for the field.");
  params.addParam<std::string>("electric_field_heating_name",
                               "electric_field_heating",
                               "User-specified material property name for the Joule Heating.");
  params.addParam<Real>("heating_scaling", 1.0, "Coefficient to multiply by heating term.");
  params.addParam<MaterialPropertyName>(
      "electrical_conductivity",
      "electrical_conductivity",
      "Material property providing electrical conductivity of the material.");
  MooseEnum formulation("time frequency", "time");
  MooseEnum solver("electrostatic electromagnetic", "electrostatic");
  params.addParam<MooseEnum>(
      "formulation",
      formulation,
      "The domain formulation of the Joule heating, time or frequency (default = time).");
  params.addParam<MooseEnum>(
      "solver", solver, "Electrostatic or electromagnetic field solver (default = electrostatic).");
  return params;
}

ElectromagneticHeatingMaterial::ElectromagneticHeatingMaterial(const InputParameters & parameters)
  : ADMaterial(parameters),
    _electric_field_var(*getFieldVar("electric_field", 0)),
    _is_vector(_electric_field_var.isVector()),
    _efield(_is_vector ? adCoupledVectorValue("electric_field") : _ad_grad_zero),
    _efield_complex(_is_vector ? adCoupledVectorValue("complex_electric_field") : _ad_grad_zero),
    _grad_potential(_is_vector ? _ad_grad_zero : adCoupledGradient("electric_field")),
    _electric_field(
        declareADProperty<RealVectorValue>(getParam<std::string>("electric_field_material_name"))),
    _electric_field_complex(declareADProperty<RealVectorValue>(
        getParam<std::string>("electric_field_material_name") + "_complex")),
    _electric_field_heating(
        declareADProperty<Real>(getParam<std::string>("electric_field_heating_name"))),
    _heating_scaling(getParam<Real>("heating_scaling")),
    _elec_cond(getADMaterialProperty<Real>("electrical_conductivity")),
    _formulation(getParam<MooseEnum>("formulation")),
    _solver(getParam<MooseEnum>("solver"))
{
  if ((_formulation == ElectromagneticFormulation::FREQUENCY) &&
      (_solver == ElectromagneticFormulation::ELECTROSTATIC))
  {
    mooseError("The frequency domain is selected, but the solver type is electrostatic! Please "
               "check input file.");
  }

  if ((_solver == ElectromagneticFormulation::ELECTROMAGNETIC) && !_is_vector)
  {
    mooseError("The solver type is electromagnetic, but only a scalar potential is provided! "
               "Please check input file.");
  }

  if ((_formulation == ElectromagneticFormulation::FREQUENCY) && !_is_vector)
  {
    mooseError("The frequency domain is selected, but only a scalar potential is provided! "
               "Please check input file.");
  }
}

void
ElectromagneticHeatingMaterial::computeQpProperties()
{
  computeFieldValue();
  computeJouleHeating();
}

void
ElectromagneticHeatingMaterial::computeFieldValue()
{
  if (_solver == ElectromagneticFormulation::ELECTROSTATIC)
    _electric_field[_qp] = -_grad_potential[_qp];
  else
    _electric_field[_qp] = _efield[_qp];

  if (_formulation == ElectromagneticFormulation::FREQUENCY)
    _electric_field_complex[_qp] = _efield_complex[_qp];
}

void
ElectromagneticHeatingMaterial::computeJouleHeating()
{
  if (_formulation == ElectromagneticFormulation::FREQUENCY)
    _electric_field_heating[_qp] = _heating_scaling * 0.5 * _elec_cond[_qp] *
                                   (_electric_field[_qp] * _electric_field[_qp] +
                                    _electric_field_complex[_qp] * _electric_field_complex[_qp]);
  else
    _electric_field_heating[_qp] =
        _heating_scaling * _elec_cond[_qp] * _electric_field[_qp] * _electric_field[_qp];
}
