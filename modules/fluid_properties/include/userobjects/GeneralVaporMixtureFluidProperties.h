//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GENERALVAPORMIXTUREFLUIDPROPERTIES_H
#define GENERALVAPORMIXTUREFLUIDPROPERTIES_H

#include "VaporMixtureFluidProperties.h"

class GeneralVaporMixtureFluidProperties;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<GeneralVaporMixtureFluidProperties>();

/**
 * Class for fluid properties of an arbitrary vapor mixture
 *
 * This model assumes that the gases occupy separate volumes, but share the same
 * pressure \f$p\f$ and temperature \f$T\f$. Specific volume and specific
 * internal energy have the following mixture relations, where \f$i\f$ denotes
 * the index of the gas in the mixture, and the lack of a subscript denotes the
 * mixture quantity:
 * \f[
 *   v = \sum\limits_i^N x_i v_i(p, T) ,
 * \f]
 * \f[
 *   e = \sum\limits_i^N x_i e_i(p, T) .
 * \f]
 * Here \f$v_i(p, T)\f$ and \f$e_i(p, T)\f$ denote the equation-of-state calls
 * from the respective gas from \f$p\f$ and \f$T\f$. Therefore, if \f$v\f$ and
 * \f$e\f$ are known, then to get \f$p\f$ and \f$T\f$, a size-2 nonlinear system
 * needs to be solved.
 */
class GeneralVaporMixtureFluidProperties : public VaporMixtureFluidProperties
{
public:
  GeneralVaporMixtureFluidProperties(const InputParameters & parameters);
  virtual ~GeneralVaporMixtureFluidProperties();

  virtual unsigned int getNumberOfSecondaryVapors() const override { return _n_secondary_vapors; }
  virtual Real p_from_v_e(Real v, Real e, const std::vector<Real> & x) const override;
  virtual void p_from_v_e(Real v,
                          Real e,
                          const std::vector<Real> & x,
                          Real & p,
                          Real & dp_dv,
                          Real & dp_de,
                          std::vector<Real> & dp_dx) const override;
  virtual Real T_from_v_e(Real v, Real e, const std::vector<Real> & x) const override;
  virtual void T_from_v_e(Real v,
                          Real e,
                          const std::vector<Real> & x,
                          Real & T,
                          Real & dT_dv,
                          Real & dT_de,
                          std::vector<Real> & dT_dx) const override;
  virtual Real rho_from_p_T(Real p, Real T, const std::vector<Real> & x) const override;
  virtual void rho_from_p_T(Real p,
                            Real T,
                            const std::vector<Real> & x,
                            Real & rho,
                            Real & drho_dp,
                            Real & drho_dT,
                            std::vector<Real> & drho_dx) const override;
  virtual Real e_from_p_T(Real p, Real T, const std::vector<Real> & x) const override;
  virtual void e_from_p_T(Real p,
                          Real T,
                          const std::vector<Real> & x,
                          Real & e,
                          Real & de_dp,
                          Real & de_dT,
                          std::vector<Real> & de_dx) const override;
  virtual Real c_from_p_T(Real p, Real T, const std::vector<Real> & x) const override;
  virtual Real cp_from_p_T(Real p, Real T, const std::vector<Real> & x) const override;
  virtual Real cv_from_p_T(Real p, Real T, const std::vector<Real> & x) const override;
  virtual Real mu_from_p_T(Real p, Real T, const std::vector<Real> & x) const override;
  virtual Real k_from_p_T(Real p, Real T, const std::vector<Real> & x) const override;

  /**
   * Specific volume from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   */
  virtual Real v_from_p_T(Real p, Real T, const std::vector<Real> & x) const;

  /**
   * Specific volume and its derivatives from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   * @param[in] x        vapor mass fraction values
   * @param[out] v       specific volume
   * @param[out] dv_dp   derivative of specific volume w.r.t. pressure
   * @param[out] dv_dT   derivative of specific volume w.r.t. temperature
   * @param[out] dv_dx   derivative of specific volume w.r.t. vapor mass fraction values
   */
  virtual void v_from_p_T(Real p,
                          Real T,
                          const std::vector<Real> & x,
                          Real & v,
                          Real & dv_dp,
                          Real & dv_dT,
                          std::vector<Real> & dv_dx) const;

  /**
   * Pressure and temperature from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   * @param[in] x   vapor mass fraction values
   * @param[out] p  pressure
   * @param[out] T  temperature
   */
  void p_T_from_v_e(Real v, Real e, const std::vector<Real> & x, Real & p, Real & T) const;

  /**
   * Pressure and temperature from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   * @param[in] x   vapor mass fraction values
   * @param[out] p       pressure
   * @param[out] dp_dv   derivative of pressure w.r.t. specific volume
   * @param[out] dp_de   derivative of pressure w.r.t. specific internal energy
   * @param[out] dp_dx   derivative of pressure w.r.t. vapor mass fraction values
   * @param[out] T       temperature
   * @param[out] dT_dv   derivative of temperature w.r.t. specific volume
   * @param[out] dT_de   derivative of temperature w.r.t. specific internal energy
   * @param[out] dT_dx   derivative of temperature w.r.t. vapor mass fraction values
   */
  void p_T_from_v_e(Real v,
                    Real e,
                    const std::vector<Real> & x,
                    Real & p,
                    Real & dp_dv,
                    Real & dp_de,
                    std::vector<Real> & dp_dx,
                    Real & T,
                    Real & dT_dv,
                    Real & dT_de,
                    std::vector<Real> & dT_dx) const;

protected:
  /// Primary vapor fluid properties
  const SinglePhaseFluidProperties * _fp_primary;
  /// Secondary vapor fluid properties
  std::vector<const SinglePhaseFluidProperties *> _fp_secondary;
  /// Names of secondary vapor fluid properties
  const std::vector<UserObjectName> _fp_secondary_names;

  /// Number of secondary vapors
  const unsigned int _n_secondary_vapors;

  /// Initial guess for pressure
  const Real _p_initial_guess;
  /// Initial guess for temperature
  const Real _T_initial_guess;
  /// Damping factor for Newton updates
  const Real _newton_damping;
  /// Relative tolerance for Newton iteration
  const Real _newton_rel_tol;
  /// Maximum iterations for Newton iteration
  const unsigned int _newton_max_its;
  /// Option to update guesses after each converged solve
  const bool _update_guesses;

  /// Current guess for pressure (in case guess is chosen to be changed)
  mutable Real _p_guess;
  /// Current guess for temperature (in case guess is chosen to be changed)
  mutable Real _T_guess;
};

#endif /* GENERALVAPORMIXTUREFLUIDPROPERTIES_H */
