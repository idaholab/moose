/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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

  virtual void act() override;

protected:
  virtual void addDictator() override;

  /// porepressure NonlinearVariable name
  const NonlinearVariableName _pp_var;

  /// Determines the coupling type
  const enum class CouplingTypeEnum {
    Hydro,
    ThermoHydro,
    HydroMechanical,
    ThermoHydroMechanical
  } _coupling_type;

  /// whether steady or transient simulation
  const enum class SimulationTypeChoiceEnum { STEADY, TRANSIENT } _simulation_type;

  /// Name of the fluid-properties UserObject
  const UserObjectName & _fp;

  /// fluid specific heat capacity at constant volume
  const Real _biot_coefficient;

  /// add a AuxVariables to record Darcy velocity
  const bool _add_darcy_aux;

  /// add AuxVariables for stress
  const bool _add_stress_aux;
};

#endif // POROUSFLOWSINGLEPHASEBASE_H
