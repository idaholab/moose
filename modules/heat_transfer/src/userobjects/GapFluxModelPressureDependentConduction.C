//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapFluxModelPressureDependentConduction.h"
#include "MortarContactPressureProvider.h"
#include "MooseVariableFieldBase.h"

registerMooseObject("HeatTransferApp", GapFluxModelPressureDependentConduction);

InputParameters
GapFluxModelPressureDependentConduction::validParams()
{
  InputParameters params = GapFluxModelBase::validParams();

  params.addClassDescription("Heat flux model across a closed gap to calculate the conductance "
                             "between two solid materials");
  params.addRequiredCoupledVar("temperature", "The name of the temperature variable");
  params.addParam<UserObjectName>(
      "contact_pressure_uo",
      "The mortar contact user object that provides the normal contact pressure. Required when the "
      "contact pressure comes from a mechanical contact solve.");
  params.addCoupledVar("contact_pressure",
                       "An auxiliary variable holding a prescribed normal contact pressure");

  params.addParam<Real>(
      "scaling_coefficient",
      1.0,
      "The leading coefficient for the closed gap conductance value; used for tuning");
  params.addRequiredParam<MaterialPropertyName>(
      "primary_conductivity", "The thermal conductivity of the primary surface solid material");
  params.addRequiredParam<MaterialPropertyName>(
      "secondary_conductivity", "The thermal conductivity of the secondary surface solid material");
  params.addRequiredParam<MaterialPropertyName>(
      "primary_hardness", "The hardness value of the primary surface material");
  params.addRequiredParam<MaterialPropertyName>("secondary_hardness",
                                                "The hardness of the secondary surface material");

  return params;
}

GapFluxModelPressureDependentConduction::GapFluxModelPressureDependentConduction(
    const InputParameters & parameters)
  : GapFluxModelBase(parameters),
    _primary_T(adCoupledNeighborValue("temperature")),
    _secondary_T(adCoupledValue("temperature")),
    _contact_pressure_provider(
        isParamValid("contact_pressure_uo")
            ? &getUserObject<MortarContactPressureProvider>("contact_pressure_uo")
            : nullptr),
    _contact_pressure(isCoupled("contact_pressure") ? &adCoupledLowerValue("contact_pressure")
                                                    : nullptr),
    _scaling(getParam<Real>("scaling_coefficient")),
    _primary_conductivity(getNeighborADMaterialProperty<Real>("primary_conductivity")),
    _secondary_conductivity(getADMaterialProperty<Real>("secondary_conductivity")),
    _primary_hardness(getNeighborADMaterialProperty<Real>("primary_hardness")),
    _secondary_hardness(getADMaterialProperty<Real>("secondary_hardness"))
{
  if (_contact_pressure_provider && _contact_pressure)
    paramError("contact_pressure",
               "Provide the contact pressure through either 'contact_pressure' or "
               "'contact_pressure_uo', but not both.");
  if (!_contact_pressure_provider && !_contact_pressure)
    paramError("contact_pressure_uo",
               "Provide the contact pressure through 'contact_pressure_uo' or 'contact_pressure'.");

  // A contact Lagrange multiplier can store a scaled pressure, for example under node-based or
  // physically derived contact scaling, so a solved pressure must come from its contact object
  if (_contact_pressure && getFieldVar("contact_pressure", 0)->kind() == Moose::VAR_SOLVER)
    paramError("contact_pressure",
               "'contact_pressure' couples the solution variable '",
               getFieldVar("contact_pressure", 0)->name(),
               "'. A contact Lagrange multiplier can store a scaled pressure, so pass the mortar "
               "contact user object through 'contact_pressure_uo' instead.");
}

ADReal
GapFluxModelPressureDependentConduction::contactPressure() const
{
  if (_contact_pressure)
    return (*_contact_pressure)[_qp];

  // Interpolate the nodal physical pressures of the secondary face with the contact basis
  const auto & basis = _contact_pressure_provider->tractionBasis();
  ADReal pressure = 0;
  for (const auto j : index_range(basis))
    pressure += basis[j][_qp] * _contact_pressure_provider->nodalContactPressure(
                                    _lower_secondary_elem->node_ref(j));
  return pressure;
}

ADReal
GapFluxModelPressureDependentConduction::computeFlux() const
{
  // Check that the surfaces are in actual contact with the pressure:
  const auto contact_pressure = contactPressure();
  if (contact_pressure <= 0.0)
    return 0.0;

  // calculate the harmonic means of the two material properties
  const ADReal k_sum = _primary_conductivity[_qp] + _secondary_conductivity[_qp];
  const ADReal k_harmonic = 2 * _primary_conductivity[_qp] * _secondary_conductivity[_qp] / k_sum;

  const ADReal h_sum = _primary_hardness[_qp] + _secondary_hardness[_qp];
  const ADReal h_harmonic = 2 * _primary_hardness[_qp] * _secondary_hardness[_qp] / h_sum;

  return _scaling * k_harmonic * (_primary_T[_qp] - _secondary_T[_qp]) * contact_pressure /
         h_harmonic;
}
