/**********************************************************************/
/*                 SOFTWARE COPYRIGHT NOTIFICATION                    */
/*                                                                    */
/*                              SAM                                   */
/*             © COPYRIGHT 2017 UChicago Argonne, LLC                 */
/*                       ALL RIGHTS RESERVED                          */
/*                                                                    */
/* This software was produced by UChicago Argonne, LLC under Contract */
/* No. DE-AC02-06CH11357 with the U. S. Department of Energy.         */
/*                                                                    */
/* THIS SOFTWARE DISCLOSES MATERIAL PROTECTED UNDER COPYRIGHT LAW AND */
/* FURTHER DISSEMINATION IS PROHIBITED WITHOUT PRIOR WRITTEN CONSENT  */
/* OF ARGONNE NATIONAL LABORATORY.                                    */
/*                                                                    */
/* See COPYRIGHT for full details.                                    */
/**********************************************************************/

#ifndef PBSODIUMFLUIDPROPERTIES_H
#define PBSODIUMFLUIDPROPERTIES_H

#include "SinglePhaseFluidProperties.h"

class PBSodiumFluidProperties : public SinglePhaseFluidProperties
{
public:
  PBSodiumFluidProperties(const InputParameters & parameters);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

  virtual Real rho_from_p_T(Real pressure, Real temperature) const;
  virtual void
  rho_from_p_T(Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const;
  virtual Real h_from_p_T(Real pressure, Real temperature) const;

  virtual Real beta_from_p_T(Real pressure, Real temperature) const;
  virtual Real cv_from_p_T(Real pressure, Real temperature) const;
  virtual Real cp_from_p_T(Real pressure, Real temperature) const;
  virtual void
  cp_from_p_T(Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const;
  virtual Real mu_from_p_T(Real pressure, Real temperature) const;
  virtual Real mu_from_rho_T(Real rho, Real temprature) const;
  virtual Real k_from_p_T(Real pressure, Real temperature) const;

  virtual Real T_from_p_h(Real temperature, Real enthalpy) const;

#pragma GCC diagnostic pop

protected:
  static const std::vector<Real> _temperature_vec;
  static const std::vector<Real> _e_vec;

  Real temperature_correction(Real & temperature) const;
  Real F_enthalpy(Real temperature) const;

  const Real & _p_0;
  Real _T0;
  Real _Tmax;
  Real _Tmin;
  Real _H0;
  Real _H_Tmax;
  Real _H_Tmin;
  Real _Cp_Tmax;
  Real _Cp_Tmin;

public:
  static InputParameters validParams();
};

#endif
