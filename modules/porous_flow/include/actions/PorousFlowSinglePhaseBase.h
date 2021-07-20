//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowActionBase.h"

/**
 * Base class for actions involving a single fluid phase
 */
class PorousFlowSinglePhaseBase : public PorousFlowActionBase
{
public:
  static InputParameters validParams();

  PorousFlowSinglePhaseBase(const InputParameters & params);

protected:
  virtual void addDictator() override;
  virtual void addKernels() override;
  virtual void addAuxObjects() override;
  virtual void addMaterialDependencies() override;
  virtual void addMaterials() override;

  /// Porepressure NonlinearVariable name
  const VariableName _pp_var;

  /// Determines the coupling type
  const enum class CouplingTypeEnum {
    Hydro,
    ThermoHydro,
    HydroMechanical,
    ThermoHydroMechanical
  } _coupling_type;

  /// Flags to indicate whether thermal or mechanical effects are included
  const bool _thermal;
  const bool _mechanical;

  /// Determines the fluid-properties type
  const enum class FluidPropertiesTypeEnum {
    PorousFlowSingleComponentFluid,
    PorousFlowBrine,
    Custom
  } _fluid_properties_type;

  /// Name of the fluid-properties UserObject
  UserObjectName _fp;

  /// Fluid specific heat capacity at constant volume
  const Real _biot_coefficient;

  /// Add a AuxVariables to record Darcy velocity
  const bool _add_darcy_aux;

  /// Add AuxVariables for stress
  const bool _add_stress_aux;

  /// Name of the NaCl variable
  VariableName _nacl_name;

  /// Name of the variables (if any) that will record the fluid-components' rate of change
  const std::vector<AuxVariableName> _save_component_rate_in;

  /// Unit used for temperature
  const MooseEnum _temperature_unit;

  /// Unit used for porepressure
  const MooseEnum _pressure_unit;

  /// Unit used for time
  const MooseEnum _time_unit;

  /// base_name used in the TensorMechanics strain calculator
  const std::string _base_name;
};
