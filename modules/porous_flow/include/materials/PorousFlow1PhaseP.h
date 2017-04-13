/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOW1PHASEP_H
#define POROUSFLOW1PHASEP_H

#include "PorousFlowVariableBase.h"

// Forward Declarations
class PorousFlow1PhaseP;

template <>
InputParameters validParams<PorousFlow1PhaseP>();

/**
 * Base material designed to calculate fluid phase porepressure and saturation
 * for the single-phase situation assuming constant effective saturation and
 * porepressure as the nonlinear variable.
 * Inherit and over-ride effectiveSaturation, dEffectiveSaturation, and
 * d2EffectiveSaturation to implement other capillary pressure functions
 */
class PorousFlow1PhaseP : public PorousFlowVariableBase
{
public:
  PorousFlow1PhaseP(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /**
   * Assemble std::vectors of porepressure, saturation and temperature at the quadpoints
   */
  void buildQpPPSS();

  /**
   * Effective saturation as a function of porepressure.
   * Default is constant saturation = 1.
   * If porepressure < 0 then saturation < 1.
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

  /// Nodal or quadpoint value of porepressure of the fluid phase
  const VariableValue & _porepressure_var;

  /// Gradient(_porepressure at quadpoints)
  const VariableGradient & _gradp_qp_var;

  /// Moose variable number of the porepressure
  const unsigned int _porepressure_varnum;

  /// the PorousFlow variable number of the porepressure
  const unsigned int _p_var_num;
};

#endif // POROUSFLOW1PHASEP_H
