/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMATERIAL2PHASEPS_H
#define PORFLOWMATERIAL2PHASEPS_H

#include "PorousFlowStateBase.h"

//Forward Declarations
class PorousFlowMaterial2PhasePS;

template<>
InputParameters validParams<PorousFlowMaterial2PhasePS>();

/**
 * Material designed to calculate fluid-phase porepressures at nodes
 */
class PorousFlowMaterial2PhasePS : public PorousFlowStateBase
{
public:
  PorousFlowMaterial2PhasePS(const InputParameters & parameters);

protected:

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  /// number of phases (=2 for this class)
  const unsigned int _num_ph;
  /// Nodal value of porepressure of the zero phase (eg, the gas phase)
  const VariableValue & _phase0_porepressure_nodal;
  /// Quadpoint value of porepressure of the zero phase (eg, the gas phase)
  const VariableValue & _phase0_porepressure_qp;
  /// Gradient(phase0_porepressure) at the qps
  const VariableGradient & _phase0_gradp_qp;
  /// Moose variable number of the phase0 porepressure
  const unsigned int _phase0_porepressure_varnum;
  /// Nodal value of saturation of the one phase (eg, the water phase)
  const VariableValue & _phase1_saturation_nodal;
  /// Quadpoint value of saturation of the one phase (eg, the water phase)
  const VariableValue & _phase1_saturation_qp;
  /// Gradient(phase1_saturation) at the qps
  const VariableGradient & _phase1_grads_qp;
  /// Moose variable number of the phase1 saturation
  const unsigned int _phase1_saturation_varnum;
  /// Nodal value of temperature
  const VariableValue & _phase0_temperature_nodal;
  /// Quadpoint value of temperature
  const VariableValue & _phase0_temperature_qp;
  /// Moose variable number of the phase0 temperature
  const unsigned int _phase0_temperature_varnum;

 private:
  void buildQpPPSSTT();
};

#endif //PORFLOWMATERIAL2PHASEPS_H
