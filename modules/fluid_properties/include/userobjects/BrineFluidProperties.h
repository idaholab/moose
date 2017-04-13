/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
 */
class BrineFluidProperties : public MultiComponentFluidPropertiesPT
{
public:
  BrineFluidProperties(const InputParameters & parameters);
  virtual ~BrineFluidProperties();

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

  /**
   * Density of brine
   * From Driesner, The system H2O-NaCl. Part II: Correlations for molar volume,
   * enthalpy, and isobaric heat capacity from 0 to 1000 C, 1 to 500 bar, and 0
   * to 1 Xnacl, Geochimica et Cosmochimica Acta 71, 4902-4919 (2007).
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xnacl NaCl mass fraction (-)
   * @return density (kg/m^3)
   */
  virtual Real rho(Real pressure, Real temperature, Real xnacl) const override;

  /**
   * Density of brine and derivatives wrt pressure, temperature and mass fraction
   * From Driesner, The system H2O-NaCl. Part II: Correlations for molar volume,
   * enthalpy, and isobaric heat capacity from 0 to 1000 C, 1 to 500 bar, and 0
   * to 1 Xnacl, Geochimica et Cosmochimica Acta 71, 4902-4919 (2007).
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xnacl NaCl mass fraction (-)
   * @param[out] rho density (kg/m^3)
   * @param[out] drho_dp derivative of density wrt pressure
   * @param[out] drho_dT derivative of density wrt temperature
   * @param[out] drho_dx derivative of density wrt NaCl mass fraction
   */
  virtual void rho_dpTx(Real pressure,
                        Real temperature,
                        Real xnacl,
                        Real & rho,
                        Real & drho_dp,
                        Real & drho_dT,
                        Real & drho_dx) const override;

  /**
   * Viscosity of brine
   * From Phillips et al, A technical databook for geothermal energy utilization,
   * LbL-12810 (1981).
   *
   * @param water_density water density (kg/m^3)
   * @param temperature brine temperature (K)
   * @param xnacl salt mass fraction (-)
   * @return viscosity (Pa.s)
   */
  virtual Real mu(Real water_density, Real temperature, Real xnacl) const override;

  /**
   * Viscosity of brine and derivatives wrt pressure, temperature and mass fraction
   * From Phillips et al, A technical databook for geothermal energy utilization,
   * LbL-12810 (1981).
   *
   * @param water_density water density (kg/m^3)
   * @param temperature brine temperature (K)
   * @param xnacl salt mass fraction (-)
   * @param[out] mu viscosity (Pa.s)
   * @param[out] dmu_drho derivative of viscosity wrt water density
   * @param[out] dmu_dT derivative of viscosity wrt temperature
   * @param[out] dmu_dx derivative of viscosity wrt NaCl mass fraction
   */
  virtual void mu_drhoTx(Real water_density,
                         Real temperature,
                         Real xnacl,
                         Real & mu,
                         Real & dmu_drho,
                         Real & dmu_dT,
                         Real & dmu_dx) const override;

  /**
   * Enthalpy of brine
   * From Driesner, The system H2O-NaCl. Part II: Correlations for molar volume,
   * enthalpy, and isobaric heat capacity from 0 to 1000 C, 1 to 500 bar, and 0
   * to 1 Xnacl, Geochimica et Cosmochimica Acta 71, 4902-4919 (2007).
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xnacl NaCl mass fraction (-)
   * @return enthalpy (J/kg)
   */
  virtual Real h(Real pressure, Real temperature, Real xnacl) const override;

  /**
   * Enthalpy of brine and derivatives wrt pressure, temperature and mass fraction
   * From Driesner, The system H2O-NaCl. Part II: Correlations for molar volume,
   * enthalpy, and isobaric heat capacity from 0 to 1000 C, 1 to 500 bar, and 0
   * to 1 Xnacl, Geochimica et Cosmochimica Acta 71, 4902-4919 (2007).
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xnacl NaCl mass fraction (-)
   * @param[out] h enthalpy (J/kg)
   * @param[out] dh_dp derivative of enthalpy wrt pressure
   * @param[out] dh_dT derivative of enthalpy wrt temperature
   * @param[out] dh_dx derivative of enthalpy wrt NaCl mass fraction
   */
  virtual void h_dpTx(Real pressure,
                      Real temperature,
                      Real xnacl,
                      Real & h,
                      Real & dh_dp,
                      Real & dh_dT,
                      Real & dh_dx) const override;

  /**
   * Isobaric heat capacity of brine
   * From Driesner, The system H2O-NaCl. Part II: Correlations for molar volume,
   * enthalpy, and isobaric heat capacity from 0 to 1000 C, 1 to 500 bar, and 0
   * to 1 Xnacl, Geochimica et Cosmochimica Acta 71, 4902-4919 (2007).
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xnacl NaCl mass fraction (-)
   * @return isobaric heat capacity (J/kg/K)
   */
  virtual Real cp(Real pressure, Real temperature, Real xnacl) const override;

  /**
   * Internal energy of brine
   * Calculated from h - p / rho, where enthalpy and density are given above
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xnacl NaCl mass fraction (-)
   * @return internal energy (J/kg)
   */
  virtual Real e(Real pressure, Real temperature, Real xnacl) const override;

  /**
   * Internal energy of brine and derivatives wrt pressure, temperature and mass fraction
   * From Driesner, The system H2O-NaCl. Part II: Correlations for molar volume,
   * enthalpy, and isobaric heat capacity from 0 to 1000 C, 1 to 500 bar, and 0
   * to 1 Xnacl, Geochimica et Cosmochimica Acta 71, 4902-4919 (2007).
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param xnacl NaCl mass fraction (-)
   * @param[out] e internal energy (J/kg)
   * @param[out] de_dp derivative of internal energy wrt pressure
   * @param[out] de_dT derivative of internal energy wrt temperature
   * @param[out] de_dx derivative of internal energy wrt NaCl mass fraction
   */
  virtual void e_dpTx(Real pressure,
                      Real temperature,
                      Real xnacl,
                      Real & e,
                      Real & de_dp,
                      Real & de_dT,
                      Real & de_dx) const override;

  /**
   * Thermal conductivity of brine
   * From Phillips et al, A technical databook for geothermal energy utilization,
   * LBL-12810 (1981).
   * Note: uses water thermal conductivity from IAPWS rather than the correlation
   * given by Phillips et al.
   *
   * @param water_density water density (kg/m^3)
   * @param temperature fluid temperature (K)
   * @param xnacl NaCl mass fraction (-)
   * @return thermal conductivity (W/m/K)
   */
  virtual Real k(Real water_density, Real temperature, Real xnacl) const override;

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
  Real pSat(Real temperature, Real xnacl) const;

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

  /// Proved access to UserObject for specified component
  virtual const SinglePhaseFluidPropertiesPT & getComponent(unsigned int component) const override;

  /// Fluid component numbers for water and NaCl
  static const unsigned int WATER = 0;
  static const unsigned int NACL = 1;

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
  const SinglePhaseFluidPropertiesPT * _halite_fp;

  /// Molar mass of NaCl (kg/mol)
  Real _Mnacl;
  /// Molar mass of water (H2O) (kg/mol)
  Real _Mh2o;
};

#endif /* BRINEFLUIDPROPERTIES_H */
