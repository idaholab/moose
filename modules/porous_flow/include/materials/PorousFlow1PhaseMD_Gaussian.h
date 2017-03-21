/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOW1PHASEMD_GAUSSIAN_H
#define POROUSFLOW1PHASEMD_GAUSSIAN_H

#include "PorousFlowVariableBase.h"

// Forward Declarations
class PorousFlow1PhaseMD_Gaussian;

template <>
InputParameters validParams<PorousFlow1PhaseMD_Gaussian>();

/**
 * Material designed to calculate fluid-phase porepressure and saturation
 * for the single-phase situation, assuming a Gaussian capillary suction
 * function and assuming the independent variable is log(mass density) and
 * assuming the fluid has a constant bulk modulus
 */
class PorousFlow1PhaseMD_Gaussian : public PorousFlowVariableBase
{
public:
  PorousFlow1PhaseMD_Gaussian(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Gaussian parameter: saturation = exp(-(al*p)^2)
  const Real _al;

  /// _al2 = al*al
  const Real _al2;

  /// fluid density = _dens0*exp(P/_bulk)
  const Real _logdens0;

  /// fluid density = _dens0*exp(P/_bulk)
  const Real _bulk;

  /// 1/_bulk/_al
  const Real _recip_bulk;

  /// (1/_bulk)^2
  const Real _recip_bulk2;

  /// Nodal or quadpoint value of mass-density of the fluid phase
  const VariableValue & _md_var;

  /// Gradient(_mass-density at quadpoints)
  const VariableGradient & _gradmd_qp_var;

  /// Moose variable number of the mass-density
  const unsigned int _md_varnum;

  /// PorousFlow variable number of the mass-density
  const unsigned int _pvar;

  virtual void buildPS();
};

#endif // POROUSFLOW1PHASEMD_GAUSSIAN_H
