//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWSINGLEPHASEBASE_H
#define POROUSFLOWSINGLEPHASEBASE_H

#include "PorousFlowActionBase.h"

class PorousFlowSinglePhaseBase;

template <>
InputParameters validParams<PorousFlowSinglePhaseBase>();

/**
 * Base class for actions involving a single fluid phase
 */
class PorousFlowSinglePhaseBase : public PorousFlowActionBase
{
public:
  PorousFlowSinglePhaseBase(const InputParameters & params);

protected:
  virtual void addDictator() override;
  virtual void addKernels() override;
  virtual void addAuxObjects() override;
  virtual void addMaterialDependencies() override;
  virtual void addMaterials() override;

  /// Porepressure NonlinearVariable name
  const NonlinearVariableName _pp_var;

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

  /// Name of the fluid-properties UserObject
  const UserObjectName & _fp;

  /// Fluid specific heat capacity at constant volume
  const Real _biot_coefficient;

  /// Add a AuxVariables to record Darcy velocity
  const bool _add_darcy_aux;

  /// Add AuxVariables for stress
  const bool _add_stress_aux;

  /// Use PorousFlowBrine material
  const bool _use_brine;

  /// Index of NaCl in list of fluid components
  const unsigned _nacl_index;
};

#endif // POROUSFLOWSINGLEPHASEBASE_H
