//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VaporMixtureFluidProperties.h"
#include "NaNInterface.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

class SinglePhaseFluidProperties;

/**
 * Class for fluid properties of an arbitrary vapor mixture
 *
 *
 */
class IdealRealGasMixtureFluidProperties : public VaporMixtureFluidProperties, public NaNInterface
{
public:
  static InputParameters validParams();

  IdealRealGasMixtureFluidProperties(const InputParameters & parameters);

  /**
   * Number of secondary vapors (non-condensable components)
   *
   * @return number of secondary vapors
   */
  virtual unsigned int getNumberOfSecondaryVapors() const override { return _n_secondary_vapors; }

  /**
   * Pressure from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   * @param[in] x   vapor mass fraction values
   * @return        pressure
   */
  virtual Real p_from_v_e(Real v, Real e, const std::vector<Real> & x) const override;

  /**
   * Pressure and its derivatives from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   * @param[in] x   vapor mass fraction values
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
                          std::vector<Real> & dp_dx) const override;

  /**
   * Temperature from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   * @param[in] x   vapor mass fraction values
   * @return        temperature
   */
  virtual Real T_from_v_e(Real v, Real e, const std::vector<Real> & x) const override;

  /**
   * Temperature and its derivatives from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   * @param[in] x   vapor mass fraction values
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
                          std::vector<Real> & dT_dx) const override;

  /**
   * Speed of sound from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   * @param[in] x   vapor mass fraction values
   * @return        speed of sound
   */
  virtual Real c_from_v_e(Real v, Real e, const std::vector<Real> & x) const override;

  /**
   * Speed of sound and its derivatives from specific volume and specific internal energy
   *
   * @param[in] v   specific volume
   * @param[in] e   specific internal energy
   * @param[in] x   vapor mass fraction values
   * @param[out] c       Speed of sound
   * @param[out] dc_dv   derivative of temperature w.r.t. specific volume
   * @param[out] dc_de   derivative of temperature w.r.t. specific internal energy
   * @param[out] dc_dx   derivative of temperature w.r.t. vapor mass fraction values
   */
  virtual void c_from_v_e(Real v,
                          Real e,
                          const std::vector<Real> & x,
                          Real & c,
                          Real & dc_dv,
                          Real & dc_de,
                          std::vector<Real> & dc_dx) const override;

  /**
   * Density from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   * @return        density
   */
  virtual Real rho_from_p_T(Real p, Real T, const std::vector<Real> & x) const override;

  /**
   * Density and its derivatives from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
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
                            std::vector<Real> & drho_dx) const override;

  /**
   * Specific internal energy from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   * @return        specific internal energy
   */
  virtual Real e_from_p_T(Real p, Real T, const std::vector<Real> & x) const override;

  /**
   * Specific internal energy and its derivatives from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
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
                          std::vector<Real> & de_dx) const override;

  /**
   * Speed of sound from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   * @return        speed of sound
   */
  virtual Real c_from_p_T(Real p, Real T, const std::vector<Real> & x) const override;

  /**
   * Speed of sound and its derivatives from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   * @param[out] c       speed of sound
   * @param[out] dc_dp   derivative of speed of sound w.r.t. pressure
   * @param[out] dc_dT   derivative of speed of sound w.r.t. temperature
   * @param[out] dc_dx   derivative of speed of sound w.r.t. vapor mass fraction values
   */
  virtual void c_from_p_T(Real p,
                          Real T,
                          const std::vector<Real> & x,
                          Real & c,
                          Real & dc_dp,
                          Real & dc_dT,
                          std::vector<Real> & dc_dx) const override;

  /**
   * Isobaric heat capacity from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   * @return        isobaric heat capacity
   */
  virtual Real cp_from_p_T(Real p, Real T, const std::vector<Real> & x) const override;
  virtual void cp_from_p_T(Real p,
                           Real T,
                           const std::vector<Real> & x,
                           Real & cp,
                           Real & dcp_dp,
                           Real & dcp_dT,
                           std::vector<Real> & dcp_dx) const override;

  /**
   * Isochoric heat capacity from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   * @return        isochoric heat capacity
   */
  virtual Real cv_from_p_T(Real p, Real T, const std::vector<Real> & x) const override;
  virtual void cv_from_p_T(Real p,
                           Real T,
                           const std::vector<Real> & x,
                           Real & cv,
                           Real & dcv_dp,
                           Real & dcv_dT,
                           std::vector<Real> & dcv_dx) const override;

  /**
   * Dynamic viscosity from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   * @return        dynamic viscosity
   */
  virtual Real mu_from_p_T(Real p, Real T, const std::vector<Real> & x) const override;
  virtual void mu_from_p_T(Real p,
                           Real T,
                           const std::vector<Real> & x,
                           Real & mu,
                           Real & dmu_dp,
                           Real & dmu_dT,
                           std::vector<Real> & dmu_dx) const override;

  /**
   * Thermal conductivity from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   * @return        thermal conductivity
   */
  virtual Real k_from_p_T(Real p, Real T, const std::vector<Real> & x) const override;
  virtual void k_from_p_T(Real p,
                          Real T,
                          const std::vector<Real> & x,
                          Real & k,
                          Real & dk_dp,
                          Real & dk_dT,
                          std::vector<Real> & dk_dx) const override;
  /**
   * Specific volume from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
   * @return        specific volume
   */
  virtual Real v_from_p_T(Real p, Real T, const std::vector<Real> & x) const;

  /**
   * Specific volume and its derivatives from pressure and temperature
   *
   * @param[in] p   pressure
   * @param[in] T   temperature
   * @param[in] x   vapor mass fraction values
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
   * Specific internal energy from pressure and density
   *
   * @param[in] p   pressure
   * @param[in] rho density
   * @param[in] x   vapor mass fraction values
   * @return        specific internal energy
   */
  virtual Real e_from_p_rho(Real p, Real rho, const std::vector<Real> & x) const override;

  /**
   * Specific internal energy and its derivatives from pressure and density
   *
   * @param[in] p   pressure
   * @param[in] rho density
   * @param[in] x   vapor mass fraction values
   * @param[out] e       specific internal energy
   * @param[out] de_dp   derivative of specific internal energy w.r.t. pressure
   * @param[out] de_drho derivative of specific internal energy w.r.t. density
   * @param[out] de_dx   derivative of specific internal energy w.r.t. vapor mass fraction values
   */
  virtual void e_from_p_rho(Real p,
                            Real rho,
                            const std::vector<Real> & x,
                            Real & e,
                            Real & de_dp,
                            Real & de_drho,
                            std::vector<Real> & de_dx) const override;

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

  /**
   * Temperature from pressure and specific volume
   *
   * @param[in] p   pressure
   * @param[in] v   specific volume
   * @param[in] x   vapor mass fraction values
   * @return        temperature
   */
  Real T_from_p_v(Real p, Real v, const std::vector<Real> & x) const;

  /**
   * Temperature and its derivatives from pressure and specific volume
   *
   * @param[in] p   pressure
   * @param[in] v   specific volume
   * @param[in] x   vapor mass fraction values
   * @param[out] T       temperature
   * @param[out] dT_dp   derivative of temperature w.r.t. pressure
   * @param[out] dT_dv   derivative of temperature w.r.t. specific volume
   * @param[out] dT_dx   derivative of temperature w.r.t. vapor mass fraction values
   */
  void T_from_p_v(Real p,
                  Real v,
                  const std::vector<Real> & x,
                  Real & T,
                  Real & dT_dp,
                  Real & dT_dv,
                  std::vector<Real> & dT_dx) const;

  /**
   * Pressure from temperature and specific volume
   *
   * @param[in] T   temperature
   * @param[in] v   specific volume
   * @param[in] x   vapor mass fraction values
   * @return        pressure
   */
  Real p_from_T_v(Real T, Real v, const std::vector<Real> & x) const;

  /**
   * Pressure and its derivatives from temperature and specific volume
   *
   * @param[in] T   temperature
   * @param[in] v   specific volume
   * @param[in] x   vapor mass fraction values
   * @param[out] p       pressure
   * @param[out] dp_dT   derivative of pressure w.r.t. temperature
   * @param[out] dp_dv   derivative of pressure w.r.t. specific volume
   */
  void p_from_T_v(
      Real T, Real v, const std::vector<Real> & x, Real & p, Real & dp_dT, Real & dp_dv) const;

  /**
   * Pressure and its derivatives from temperature and specific volume
   *
   * @param[in] T   temperature
   * @param[in] v   specific volume
   * @param[in] x   vapor mass fraction values
   * @param[out] p       pressure
   * @param[out] dp_dT   derivative of pressure w.r.t. temperature
   * @param[out] dp_dv   derivative of pressure w.r.t. specific volume
   * @param[out] dp_dx   derivative of pressure w.r.t. vapor mass fraction values
   */
  void p_from_T_v(Real T,
                  Real v,
                  const std::vector<Real> & x,
                  Real & p,
                  Real & dp_dT,
                  Real & dp_dv,
                  std::vector<Real> & dp_dx) const;

  /**
   * Specific internal energy from temperature and specific volume
   *
   * @param[in] T   temperature
   * @param[in] v   specific volume
   * @param[in] x   vapor mass fraction values
   * @return        specific internal energy
   */
  Real e_from_T_v(Real T, Real v, const std::vector<Real> & x) const;

  /**
   * Specific internal energy and its derivatives from temperature and specific volume
   *
   * @param[in] T   temperature
   * @param[in] v   specific volume
   * @param[in] x   vapor mass fraction values
   * @param[out] e       specific internal energy
   * @param[out] de_dT   derivative of specific internal energy w.r.t. temperature
   * @param[out] de_dv   derivative of specific internal energy w.r.t. specific volume
   * @param[out] de_dx   derivative of specific internal energy w.r.t. vapor mass fraction values
   */
  void e_from_T_v(Real T,
                  Real v,
                  const std::vector<Real> & x,
                  Real & e,
                  Real & de_dT,
                  Real & de_dv,
                  std::vector<Real> & de_dx) const;

  /**
   * Specific entropy and its derivatives from temperature and specific volume
   *
   * @param[in] T   temperature
   * @param[in] v   specific volume
   * @param[in] x   vapor mass fraction values
   * @param[out] s       specific entropy
   * @param[out] ds_dT   derivative of specific entropy w.r.t. temperature
   * @param[out] ds_dv   derivative of specific entropy w.r.t. specific volume
   * @param[out] ds_dx   derivative of specific entropy w.r.t. vapor mass fraction values
   */
  void s_from_T_v(
      Real T, Real v, const std::vector<Real> & x, Real & s, Real & ds_dT, Real & ds_dv) const;

  /**
   * Speed of sound from temperature and specific volume
   *
   * @param[in] T   temperature
   * @param[in] v   specific volume
   * @param[in] x   vapor mass fraction values
   * @return        speed of sound
   */
  Real c_from_T_v(Real T, Real v, const std::vector<Real> & x) const;

  /**
   * Speed of sound and its derivatives from temperature and specific volume
   *
   * @param[in] T   temperature
   * @param[in] v   specific volume
   * @param[in] x   vapor mass fraction values
   * @param[out] c       speed of sound
   * @param[out] dc_dT   derivative of speed of sound w.r.t. temperature
   * @param[out] dc_dv   derivative of speed of sound w.r.t. specific volume
   * @param[out] dc_dx   derivative of speed of sound w.r.t. vapor mass fraction values
   */
  void c_from_T_v(Real T,
                  Real v,
                  const std::vector<Real> & x,
                  Real & c,
                  Real & dc_dT,
                  Real & dc_dv,
                  std::vector<Real> & dc_dx) const;

  /**
   * Isobaric heat capacity from temperature and specific volume
   *
   * @param[in] T   temperature
   * @param[in] v   specific volume
   * @param[in] x   vapor mass fraction values
   * @return        isobaric heat capacity
   */
  Real cp_from_T_v(Real T, Real v, const std::vector<Real> & x) const;

  /**
   * Isochoric heat capacity from temperature and specific volume
   *
   * @param[in] T   temperature
   * @param[in] v   specific volume
   * @param[in] x   vapor mass fraction values
   * @return        isochoric heat capacity
   */
  Real cv_from_T_v(Real T, Real v, const std::vector<Real> & x) const;

  /**
   * Dynamic viscosity from temperature and specific volume
   *
   * @param[in] T   temperature
   * @param[in] v   specific volume
   * @param[in] x   vapor mass fraction values
   * @return        dynamic viscosity
   */
  Real mu_from_T_v(Real T, Real v, const std::vector<Real> & x) const;

  /**
   * Thermal conductivity from temperature and specific volume
   *
   * @param[in] T   temperature
   * @param[in] v   specific volume
   * @param[in] x   vapor mass fraction values
   * @return        thermal conductivity
   */
  Real k_from_T_v(Real T, Real v, const std::vector<Real> & x) const;

  /**
   * Mass fraction of primary (condensable) component at saturation from pressure and temperature
   *
   * @param[in] T   temperature
   * @param[in] p   pressure
   * @return        mass fraction of primary (condensable) component at saturation
   */
  Real xs_prim_from_p_T(Real p, Real T, const std::vector<Real> & x) const;

protected:
  /// Primary vapor fluid properties
  const SinglePhaseFluidProperties * const _fp_primary;
  /// Secondary vapor fluid properties
  std::vector<const SinglePhaseFluidProperties *> _fp_secondary;
  /// Names of secondary vapor fluid properties
  const std::vector<UserObjectName> _fp_secondary_names;
  /// Number of secondary vapors
  const unsigned int _n_secondary_vapors;
  /// molar (or universal) gas constant
  constexpr static const Real R_molar = 8.3144598;
  /// maximum temperature of all components
  const Real _T_mix_max;
};

#pragma GCC diagnostic pop
