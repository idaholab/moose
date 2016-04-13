/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWMATERIALIDEALGAS_H
#define POROUSFLOWMATERIALIDEALGAS_H

#include "PorousFlowMaterialFluidPropertiesBase.h"

class PorousFlowMaterialIdealGas;

template<>
InputParameters validParams<PorousFlowMaterialIdealGas>();

/**
 * This material computes fluid properties for an ideal gas
 */
class PorousFlowMaterialIdealGas : public PorousFlowMaterialFluidPropertiesBase
{
public:
  PorousFlowMaterialIdealGas(const InputParameters & parameters);

protected:

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

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

 private:
  /// Molar mass (kg/mol)
  const Real _molar_mass;
  /// Fluid phase density at the nodes
  MaterialProperty<Real> & _density_nodal;
  /// Old fluid phase density at the nodes
  MaterialProperty<Real> & _density_nodal_old;
  /// Derivative of fluid density wrt phase pore pressure at the nodes
  MaterialProperty<Real> & _ddensity_nodal_dp;
  /// Derivative of fluid density wrt temperature at the nodes
  MaterialProperty<Real> & _ddensity_nodal_dt;
  /// Fluid phase density at the qps
  MaterialProperty<Real> & _density_qp;
  /// Derivative of fluid density wrt phase pore pressure at the qps
  MaterialProperty<Real> & _ddensity_qp_dp;
  /// Derivative of fluid density wrt temperature at the qps
  MaterialProperty<Real> & _ddensity_qp_dt;
};

#endif // POROUSFLOWMATERIALIDEALGAS_H
