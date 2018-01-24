//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BRINEFLUIDPROPERTIES_H
#define BRINEFLUIDPROPERTIES_H

#include "MultiComponentFluidPropertiesPT.h"
#include "Water97FluidProperties.h"

class BrineFluidProperties;

template <>
InputParameters validParams<BrineFluidProperties>();

/**
 * Brine (NaCl in H2O) fluid properties as a function of pressure (Pa),
 * temperature (K) and NaCl mass fraction
 *
 * Most properties from:
 * Driesner, The system H2O-NaCl. Part II: Correlations for molar volume,
 * enthalpy, and isobaric heat capacity from 0 to 1000 C, 1 to 5000 bar, and 0
 * to 1 Xnacl, Geochimica et Cosmochimica Acta 71, 4902-4919 (2007)
 *
 * Viscosity and thermal conductivity from:
 * Phillips et al, A technical databook for geothermal energy utilization,
 * LbL-12810 (1981)
 * Note: uses water thermal conductivity from IAPWS rather than the correlation
 * given by Phillips et al.
 */
class BrineFluidProperties : public MultiComponentFluidPropertiesPT
{
public:
  BrineFluidProperties(const InputParameters & parameters);
  virtual ~BrineFluidProperties();

  /**
   * Fluid name
   * @return "brine"
   */
  virtual std::string fluidName() const override;

  /**
   * Average molar mass of brine
   * @param xnacl NaCl mass fraction (-)
   * @return average molar mass (kg/mol)
   */
  Real molarMass(Real xnacl) const;

  /**
   * NaCl molar mass
   * @return molar mass of NaCl (kg/mol)
   */
  Real molarMassNaCl() const;

  /**
   * H2O molar mass
   * @return molar mass of H2O (kg/mol)
   */
  Real molarMassH2O() const;

  virtual Real rho(Real pressure, Real temperature, Real xnacl) const override;

  virtual void rho_dpTx(Real pressure,
                        Real temperature,
                        Real xnacl,
                        Real & rho,
                        Real & drho_dp,
                        Real & drho_dT,
                        Real & drho_dx) const override;

  virtual Real mu_from_rho_T(Real water_density, Real temperature, Real xnacl) const override;

  virtual void mu_drhoTx(Real water_density,
                         Real temperature,
                         Real xnacl,
                         Real dwater_density_dT,
                         Real & mu,
                         Real & dmu_drho,
                         Real & dmu_dT,
                         Real & dmu_dx) const override;

  virtual Real h(Real pressure, Real temperature, Real xnacl) const override;

  virtual void h_dpTx(Real pressure,
                      Real temperature,
                      Real xnacl,
                      Real & h,
                      Real & dh_dp,
                      Real & dh_dT,
                      Real & dh_dx) const override;

  virtual Real cp(Real pressure, Real temperature, Real xnacl) const override;

  virtual Real e(Real pressure, Real temperature, Real xnacl) const override;

  virtual void e_dpTx(Real pressure,
                      Real temperature,
                      Real xnacl,
                      Real & e,
                      Real & de_dp,
                      Real & de_dT,
                      Real & de_dx) const override;

  virtual Real k_from_rho_T(Real water_density, Real temperature, Real xnacl) const override;

  /**
   * Brine vapour pressure
   * From Haas, Physical properties of the coexisting phases and thermochemical
   * properties of the H2O component in boiling NaCl solutions, Geological Survey
   * Bulletin, 1421-A (1976).
   *
   * @param temperature brine temperature (K)
   * @param xnacl salt mass fraction (-)
   * @return brine vapour pressure (Pa)
   */
  Real vaporPressure(Real temperature, Real xnacl) const;

  /**
   * Solubility of halite (solid NaCl) in water
   * Originally from Potter et al., A new method for determining the solubility
   * of salts in aqueous solutions at elevated temperatures, J. Res. U.S. Geol.
   * Surv., 5, 389-395 (1977). Equation describing halite solubility is repeated
   * in Chou, Phase relations in the system NaCI-KCI-H2O. III: Solubilities of
   * halite in vapor-saturated liquids above 445 C and redetermination of phase
   * equilibrium properties in the system NaCI-HzO to 1000 C and 1500 bars,
   * Geochimica et Cosmochimica Acta 51, 1965-1975 (1987).
   * Note: this correlation is valid for 0 <= T <= 424.5 C
   *
   * @param temperature temperature (K)
   * @return halite solubility (kg/kg)
   */
  Real haliteSolubility(Real temperature) const;

  /// Fluid component numbers for water and NaCl
  static const unsigned int WATER = 0;
  static const unsigned int NACL = 1;

  virtual const SinglePhaseFluidPropertiesPT & getComponent(unsigned int component) const override;

protected:
  /**
   * Conversion from mass fraction to molal concentration (molality)
   * @param xnacl NaCl mass fraction (kg/kg)
   * @return molal concentration (mol/kg)
   */
  Real massFractionToMolalConc(Real xnacl) const;

  /**
   * Conversion from mass fraction to mole fraction
   * @param xnacl NaCl mass fraction (kg/kg)
   * @return mole fraction (mol/mol)
   */
  Real massFractionToMoleFraction(Real xnacl) const;

  /// Water97FluidProperties UserObject
  const Water97FluidProperties * _water97_fp;
  /// Water97FluidProperties UserObject
  const SinglePhaseFluidPropertiesPT * _water_fp;
  /// NaClFluidProperties UserObject
  const SinglePhaseFluidPropertiesPT * _nacl_fp;

  /// Molar mass of NaCl (kg/mol)
  Real _Mnacl;
  /// Molar mass of water (H2O) (kg/mol)
  Real _Mh2o;
};

#endif /* BRINEFLUIDPROPERTIES_H */
