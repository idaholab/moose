/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMATERIAL1PHASEP_H
#define POROUSFLOWMATERIAL1PHASEP_H

#include "PorousFlowVariableBase.h"

//Forward Declarations
class PorousFlowMaterial1PhaseP;

template<>
InputParameters validParams<PorousFlowMaterial1PhaseP>();

/**
 * Base material designed to calculate fluid phase porepressure and saturation
 * for the single-phase situation assuming constant effective saturation and
 * porepressure as the nonlinear variable.
 * Inherit and over-ride effectiveSaturation, dEffectiveSaturation, and
 * d2EffectiveSaturation to implement other capillary pressure functions
 */
class PorousFlowMaterial1PhaseP : public PorousFlowVariableBase
{
public:
  PorousFlowMaterial1PhaseP(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  /**
   * Assemble std::vectors of porepressure, saturation and temperature at the nodes
   * and quadpoints
   */
  void buildQpPPSS();

  /**
   * Effective saturation as a function of porepressure.
   * Default is constant saturation = 1.
   * Over-ride in derived classes to implement other effective saturation forulations
   *
   * @param pressure porepressure (Pa)
   * @return effective saturation
   */
  virtual Real effectiveSaturation(Real pressure) const;

  /**
   * Derivative of effective saturation wrt to porepressure.
   * Default = 0 for constant saturation.
   * Over-ride in derived classes to implement other effective saturation forulations
   *
   * @param pressure porepressure (Pa)
   * @return derivative of effective saturation wrt porepressure
   */
  virtual Real dEffectiveSaturation_dP(Real pressure) const;

  /**
   * Second derivative of effective saturation wrt to porepressure.
   * Default = 0 for constant saturation.
   * Over-ride in derived classes to implement other effective saturation forulations
   *
   * @param pressure porepressure (Pa)
   * @return second derivative of effective saturation wrt porepressure
   */
  virtual Real d2EffectiveSaturation_dP2(Real pressure) const;

  /// Nodal value of porepressure of the fluid phase
  const VariableValue & _porepressure_nodal_var;
  /// Quadpoint value of porepressure of the fluid phase
  const VariableValue & _porepressure_qp_var;
  /// Gradient(_porepressure at quadpoints)
  const VariableGradient & _gradp_qp_var;
  /// Moose variable number of the porepressure
  const unsigned int _porepressure_varnum;
};

#endif //POROUSFLOWMATERIAL1PHASEP_H
