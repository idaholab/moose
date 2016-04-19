/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWMATERIALBRINE_H
#define POROUSFLOWMATERIALBRINE_H

#include "PorousFlowMaterialFluidPropertiesBase.h"
#include "PorousFlowMaterialWater.h"

class PorousFlowMaterialBrine;

template<>
InputParameters validParams<PorousFlowMaterialBrine>();

/**
 * Fluid properties of Brine.
 * Provides density, viscosity, derivatives wrt pressure and temperature
 */
class PorousFlowMaterialBrine : public PorousFlowMaterialWater
{
public:
  PorousFlowMaterialBrine(const InputParameters & parameters);

protected:

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

/**
   * Density of brine.
   * From Driesner, The system H2o-NaCl. Part II: Correlations for molar volume,
   * enthalpy, and isobaric heat capacity from 0 to 1000 C, 1 to 500 bar, and 0
   * to 1 Xnacl, Geochimica et Cosmochimica Acta 71, 4902-4919 (2007).
   *
   * @param pressure brine pressure (Pa)
   * @param temperature brine temperature (C)
   * @param xnacl salt mass fraction (-)
   * @return water density (kg/m^3)
   */
  Real density(Real pressure, Real temperature, Real xnacl) const;

  /**
   * Derivative of brine density with respect to presure.
   *
   * @param pressure brine pressure (Pa)
   * @param temperature brine temperature (C)
   * @param xnacl salt mass fraction (-)
   * @return derivative of brine density wrt pressure (kg/m^3/Pa)
   */
  Real dDensity_dP(Real pressure, Real temperature, Real xnacl) const;

  /**
   * Derivative of brine density with respect to temperature
   *
   * @param pressure brine pressure (Pa)
   * @param temperature brine temperature (C)
   * @param xnacl salt mass fraction (-)
   * @return derivative of brine density wrt temperature (kg/m^3/C)
   */
  Real dDensity_dT(Real pressure, Real temperature, Real xnacl) const;

  /**
   * Viscosity of brine.
   * From Phillips et al, A technical databook for geothermal energy utilization,
   * LbL-12810 (1981).
   *
   * @param temperature brine temperature (C)
   * @param water_density water density (kg/m^3)
   * @param xnacl salt mass fraction (-)
   * @return viscosity (Pa.s)
   */
  Real viscosity(Real temperature, Real water_density, Real xnacl) const;

  /**
   * Derivative of viscosity with respect to density. Derived from
   * Eq. (10) from Release on the IAPWS Formulation 2008 for the
   * Viscosity of Ordinary Brine Substance.
   *
   * @param temperature water temperature (C)
   * @param water_density water density (kg/m^3)
   * @param xnacl salt mass fraction (-)
   * @return derivative of water viscosity wrt density
   */
  Real dViscosity_dT(Real temperature, Real water_density, Real xnacl) const;

  /**
   * Brine vapour pressure
   * From Haas, Physical properties of the coexisting phases and thermochemical
   * properties of the H20 component in boiling NaCl solutions, Geological Survey
   * Bulletin, 1421-A (1976).
   *
   * @param temperature brine temperature (C)
   * @param xnacl salt mass fraction (-)
   * @return brine vapour pressure (Pa)
   */
  Real pSat(Real temperature, Real xnacl) const;

  /**
   * Derivative of brine viscosity with respect to density
   *
   * @param temperature brine temperature (C)
   * @param water_density water density (kg/m^3)
   * @param xnacl salt mass fraction (-)
   * @return derivative of brine viscosity wrt density
   */
  Real dViscosity_dDensity(Real temperature, Real water_density, Real xnacl) const;

  /**
   * Density of halite (solid NaCl)
   * From Driesner, The system H2o-NaCl. Part II: Correlations for molar volume,
   * enthalpy, and isobaric heat capacity from 0 to 1000 C, 1 to 500 bar, and 0
   * to 1 Xnacl, Geochimica et Cosmochimica Acta 71, 4902-4919 (2007).
   *
   * @param pressure pressure (Pa)
   * @param temperature halite temperature (C)
   * @return density (kg/m^3)
   */
  Real haliteDensity(Real pressure, Real temperature) const;

  /**
   * Halite solubility
   * Originally from Potter et al., A new method for determining the solubility
   * of salts in aqueous solutions at elevated temperatures, J. Res. U.S. Geol.
   * Surv., 5, 389-395 (1977). Equation describing halite solubility is repeated
   * in Chou, Phase relations in the system NaCI-KCI-H20. III: Solubilities of
   * halite in vapor-saturated liquids above 445°C and redetermination of phase
   * equilibrium properties in the system NaCI-HzO to 1000°C and 1500 bars,
   * Geochimica et Cosmochimica Acta 51, 1965-1975 (1987).
   *
   * @param temperature temperature (C)
   * @return halite solubility (kg/kg)
   *
   * This correlation is valid for 0 <= T << 424.5 C
   */
  Real haliteSolubility(Real temperature) const;

  /// Molar mass of NaCl
  Real _Mnacl;
  /// Mass fraction of NaCl
  Real _xnacl;
};

#endif //POROUSFLOWMATERIALBRINE_H
