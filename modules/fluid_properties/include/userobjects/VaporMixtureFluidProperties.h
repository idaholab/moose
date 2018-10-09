//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VAPORMIXTUREFLUIDPROPERTIES_H
#define VAPORMIXTUREFLUIDPROPERTIES_H

#include "FluidProperties.h"

class VaporMixtureFluidProperties;

template <>
InputParameters validParams<VaporMixtureFluidProperties>();

/**
 * Base class for fluid properties of vapor mixtures
 *
 * Each interface, in addition to requiring 2 intensive thermodynamic properties,
 * requires the mass fractions of N-1 vapors in the mixture, where N is the
 * number of vapors in the mixture. The mass fraction of the remaining vapor
 * is inferred from the fact that the mass fractions sum to unity.
 */
class VaporMixtureFluidProperties : public FluidProperties
{
public:
  VaporMixtureFluidProperties(const InputParameters & parameters);
  virtual ~VaporMixtureFluidProperties();

  /**
   * Returns the number of secondary vapors
   */
  virtual unsigned int getNumberOfSecondaryVapors() const = 0;

  /**
   * Pressure from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   * @param[in] x   vapor mass fraction values
   */
  virtual Real p_from_v_e(Real v, Real e, const std::vector<Real> & x) const = 0;

  /**
   * Pressure and its derivatives from specific volume and specific internal energy
   *
   * @param[in] v        specific volume
   * @param[in] e        specific internal energy
   * @param[in] x        vapor mass fraction values
   * @param[out] p       pressure
   * @param[out] dp_dv   derivative of pressure w.r.t. specific volume
   * @param[out] dp_de   derivative of pressure w.r.t. specific internal energy
   * @param[out] dp_dx   derivative of pressure w.r.t. vapor mass fraction values
   */
  virtual void p_from_v_e(Real v,
                          Real e,
                          const std::vector<Real> & x,
                          Real & p,
                          Real & dp_dv,
                          Real & dp_de,
                          std::vector<Real> & dp_dx) const = 0;

  /**
   * Temperature from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   * @param[in] x   vapor mass fraction values
   */
  virtual Real T_from_v_e(Real v, Real e, const std::vector<Real> & x) const = 0;

  /**
   * Temperature and its derivatives from specific volume and specific internal energy
   *
   * @param[in] v        specific volume
   * @param[in] e        specific internal energy
   * @param[in] x        vapor mass fraction values
   * @param[out] T       temperature
   * @param[out] dT_dv   derivative of temperature w.r.t. specific volume
   * @param[out] dT_de   derivative of temperature w.r.t. specific internal energy
   * @param[out] dT_dx   derivative of temperature w.r.t. vapor mass fraction values
   */
  virtual void T_from_v_e(Real v,
                          Real e,
                          const std::vector<Real> & x,
                          Real & T,
                          Real & dT_dv,
                          Real & dT_de,
                          std::vector<Real> & dT_dx) const = 0;

  /**
   * Density from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   */
  virtual Real rho_from_p_T(Real p, Real T, const std::vector<Real> & x) const = 0;

  /**
   * Density and its derivatives from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   * @param[in] x        vapor mass fraction values
   * @param[out] rho       density
   * @param[out] drho_dp   derivative of density w.r.t. pressure
   * @param[out] drho_dT   derivative of density w.r.t. temperature
   * @param[out] drho_dx   derivative of density w.r.t. vapor mass fraction values
   */
  virtual void rho_from_p_T(Real p,
                            Real T,
                            const std::vector<Real> & x,
                            Real & rho,
                            Real & drho_dp,
                            Real & drho_dT,
                            std::vector<Real> & drho_dx) const = 0;

  /**
   * Specific internal energy from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   */
  virtual Real e_from_p_T(Real p, Real T, const std::vector<Real> & x) const = 0;

  /**
   * Specific internal energy and its derivatives from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   * @param[in] x        vapor mass fraction values
   * @param[out] e       specific internal energy
   * @param[out] de_dp   derivative of specific internal energy w.r.t. pressure
   * @param[out] de_dT   derivative of specific internal energy w.r.t. temperature
   * @param[out] de_dx   derivative of specific internal energy w.r.t. vapor mass fraction values
   */
  virtual void e_from_p_T(Real p,
                          Real T,
                          const std::vector<Real> & x,
                          Real & e,
                          Real & de_dp,
                          Real & de_dT,
                          std::vector<Real> & de_dx) const = 0;

  /**
   * Sound speed from pressure and temperature
   *
   * @return Sound speed
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   */
  virtual Real c_from_p_T(Real p, Real T, const std::vector<Real> & x) const = 0;

  /**
   * Isobaric (constant-pressure) specific heat from pressure and temperature
   *
   * @return Isobaric (constant-pressure) specific heat
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   */
  virtual Real cp_from_p_T(Real p, Real T, const std::vector<Real> & x) const = 0;

  /**
   * Isochoric (constant-volume) specific heat from pressure and temperature
   *
   * @return Isochoric (constant-volume) specific heat
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   */
  virtual Real cv_from_p_T(Real p, Real T, const std::vector<Real> & x) const = 0;

  /**
   * Dynamic viscosity from pressure and temperature
   *
   * @return Dynamic viscosity
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   */
  virtual Real mu_from_p_T(Real p, Real T, const std::vector<Real> & x) const = 0;

  /**
   * Thermal conductivity from pressure and temperature
   *
   * @return Dynamic viscosity
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   */
  virtual Real k_from_p_T(Real p, Real T, const std::vector<Real> & x) const = 0;

  /**
   * Computes the mass fraction of the primary vapor given mass fractions of the
   * secondary vapors.
   *
   * This uses the relation
   * \f[
   *   \sum\limits_i^N x_i = 1 ,
   * \f]
   * where the mass fractions \f$x_i, i=2\ldots N\f$ correspond to the secondary
   * vapors.
   */
  Real primaryMassFraction(const std::vector<Real> & x) const;
};

#endif /* VAPORMIXTUREFLUIDPROPERTIES_H */
