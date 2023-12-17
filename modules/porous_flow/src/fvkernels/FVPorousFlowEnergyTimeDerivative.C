//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPorousFlowEnergyTimeDerivative.h"
#include "PorousFlowDictator.h"

registerADMooseObject("PorousFlowApp", FVPorousFlowEnergyTimeDerivative);

InputParameters
FVPorousFlowEnergyTimeDerivative::validParams()
{
  InputParameters params = FVTimeKernel::validParams();
  params.addRequiredParam<UserObjectName>("PorousFlowDictator",
                                          "The PorousFlowDictator UserObject");
  params.addClassDescription("Derivative of heat energy with respect to time");
  return params;
}

FVPorousFlowEnergyTimeDerivative::FVPorousFlowEnergyTimeDerivative(
    const InputParameters & parameters)
  : FVTimeKernel(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _num_phases(_dictator.numPhases()),
    _fluid_present(_num_phases > 0),
    _porosity(getADMaterialProperty<Real>("PorousFlow_porosity_qp")),
    _porosity_old(getMaterialPropertyOld<Real>("PorousFlow_porosity_qp")),
    _density(_fluid_present
                 ? &getADMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")
                 : nullptr),
    _density_old(_fluid_present ? &getMaterialPropertyOld<std::vector<Real>>(
                                      "PorousFlow_fluid_phase_density_qp")
                                : nullptr),
    _rock_energy(getADMaterialProperty<Real>("PorousFlow_matrix_internal_energy_nodal")),
    _rock_energy_old(getMaterialPropertyOld<Real>("PorousFlow_matrix_internal_energy_nodal")),
    _energy(_fluid_present ? &getADMaterialProperty<std::vector<Real>>(
                                 "PorousFlow_fluid_phase_internal_energy_qp")
                           : nullptr),
    _energy_old(_fluid_present ? &getMaterialPropertyOld<std::vector<Real>>(
                                     "PorousFlow_fluid_phase_internal_energy_qp")
                               : nullptr),
    _saturation(_fluid_present
                    ? &getADMaterialProperty<std::vector<Real>>("PorousFlow_saturation_qp")
                    : nullptr),
    _saturation_old(_fluid_present
                        ? &getMaterialPropertyOld<std::vector<Real>>("PorousFlow_saturation_qp")
                        : nullptr)
{
}

ADReal
FVPorousFlowEnergyTimeDerivative::computeQpResidual()
{
  /// Porous matrix heat energy
  ADReal energy = (1.0 - _porosity[_qp]) * _rock_energy[_qp];
  Real energy_old = (1.0 - _porosity_old[_qp]) * _rock_energy_old[_qp];

  /// Add the fluid heat energy
  if (_fluid_present)
    for (const auto p : make_range(_num_phases))
    {
      energy += _porosity[_qp] * (*_density)[_qp][p] * (*_saturation)[_qp][p] * (*_energy)[_qp][p];
      energy_old += _porosity_old[_qp] * (*_density_old)[_qp][p] * (*_saturation_old)[_qp][p] *
                    (*_energy_old)[_qp][p];
    }

  return (energy - energy_old) / _dt;
}
