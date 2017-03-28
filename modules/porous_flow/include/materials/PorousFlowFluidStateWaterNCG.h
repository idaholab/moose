/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWFLUIDSTATEWATERNCG_H
#define POROUSFLOWFLUIDSTATEWATERNCG_H

#include "PorousFlowFluidStateFlashBase.h"
#include "Water97FluidProperties.h"

class PorousFlowFluidStateWaterNCG;

template <>
InputParameters validParams<PorousFlowFluidStateWaterNCG>();

/**
 * Fluid state class for water and a non-condensable gas. Calculates the solubility
 * of the gas phase in the water using Henry's law, and provides density, viscosity
 * and mass fractions for use in Kernels.
 */
class PorousFlowFluidStateWaterNCG : public PorousFlowFluidStateFlashBase
{
public:
  PorousFlowFluidStateWaterNCG(const InputParameters & parameters);

protected:
  virtual void thermophysicalProperties() const override;

  /**
   * Enthalpy of dissolution of NCG in water calculated using Henry's constant
   * From Himmelblau, Partial molal heats and entropies of solution for gases dissolved
   * in water from the freezing to the near critical point, J. Phys. Chem. 63 (1959)
   *
   * @param temperature fluid temperature (K)
   * @param Kh Henry's constant (Pa)
   * @param dKh_dT derivative of Henry's constant wrt temperature
   * @return enthalpy of dissolution (kJ/kg)
   */
  Real enthalpyOfDissolution(Real temperature, Real Kh, Real dKh_dT) const;

  /**
   * Convert mole fraction to mass fraction
   *
   * @param xmol mole fraction
   * @return mass fraction
   */
  Real moleFractionToMassFraction(Real xmol) const;

  /// Fluid properties UserObject for water
  const Water97FluidProperties & _water_fp;
  /// Fluid properties UserObject for the NCG
  const SinglePhaseFluidPropertiesPT & _ncg_fp;
  /// Molar mass of water (kg/mol)
  const Real _Mh2o;
  /// Molar mass of non-condensable gas (kg/mol)
  const Real _Mncg;
};

#endif // POROUSFLOWFLUIDSTATEWATERNCG_H
