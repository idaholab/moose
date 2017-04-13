/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWIDEALGAS_H
#define POROUSFLOWIDEALGAS_H

#include "PorousFlowFluidPropertiesBase.h"

class PorousFlowIdealGas;

template <>
InputParameters validParams<PorousFlowIdealGas>();

/**
 * This material computes fluid properties for an ideal gas
 */
class PorousFlowIdealGas : public PorousFlowFluidPropertiesBase
{
public:
  PorousFlowIdealGas(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /**
   * Ideal gas density as a function of pressure, temperature and molar mass.
   *
   * @param pressure gas pressure (Pa)
   * @param temperature gas temperature (C)
   * @param molar mass gas molar mass (kg/mol)
   * @return density (kg/m^3)
   */
  Real density(Real pressure, Real temperature, Real molar_mass) const;

  /**
   * Derivative of the density of an ideal gas as a function of
   * pressure.
   *
   * @param temperature gas temperature (C)
   * @param molar mass gas molar mass (kg/mol)
   * @return derivative of ideal gas density (kg/m^3) with respect to pressure
   */
  Real dDensity_dP(Real temperature, Real molar_mass) const;

  /**
   * Derivative of the density of an ideal gas as a function of
   * temperature.
   *
   * @param pressure gas pressure (Pa)
   * @param temperature gas temperature (C)
   * @param molar mass gas molar mass (kg/mol)
   * @return derivative of ideal gas density (kg/m^3) with respect to pressure
   */
  Real dDensity_dT(Real pressure, Real temperature, Real molar_mass) const;

  /// Molar mass (kg/mol)
  const Real _molar_mass;

  /// Fluid phase density at the nodes or quadpoints
  MaterialProperty<Real> & _density;

  /// Derivative of fluid density wrt phase pore pressure at the nodes or quadpoints
  MaterialProperty<Real> & _ddensity_dp;

  /// Derivative of fluid density wrt temperature at the nodes or quadpoints
  MaterialProperty<Real> & _ddensity_dt;
};

#endif // POROUSFLOWIDEALGAS_H
