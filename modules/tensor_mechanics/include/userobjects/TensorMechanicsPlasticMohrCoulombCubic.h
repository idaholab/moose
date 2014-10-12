#ifndef TENSORMECHANICSPLASTICMOHRCOULOMBCUBIC_H
#define TENSORMECHANICSPLASTICMOHRCOULOMBCUBIC_H

#include "TensorMechanicsPlasticMohrCoulomb.h"

class TensorMechanicsPlasticMohrCoulombCubic;


template<>
InputParameters validParams<TensorMechanicsPlasticMohrCoulombCubic>();

/**
 * Mohr-Coulomb plasticity, nonassociative with cubic hardening/softening.
 * The parameters cohesion, friction angle and dilation angle are all
 * cubic functions of the internal parameter, with the cubic chosen so
 * param = param_0 at internal_parameter = 0
 * param = param_res at internal_parameter = limit
 * param is a C1 function
 */
class TensorMechanicsPlasticMohrCoulombCubic : public TensorMechanicsPlasticMohrCoulomb
{
 public:
  TensorMechanicsPlasticMohrCoulombCubic(const std::string & name, InputParameters parameters);

 protected:

  /// The cohesion at zero hardening
  Real _cohesion;

  /// friction angle at zero hardening
  Real _phi;

  /// dilation angle at zero hardening
  Real _psi;

  /// The cohesion_residual
  Real _cohesion_residual;

  /// friction angle_residual
  Real _phi_residual;

  /// dilation angle_residual
  Real _psi_residual;

  /// Limit value of internal parameter where cohesion = _cohesion_residual
  Real _cohesion_limit;

  /// Limit value of internal parameter where phi = _phi_residual
  Real _phi_limit;

  /// Limit value of internal parameter where psi = _psi_residual
  Real _psi_limit;

  Real _half_cohesion_limit;
  Real _half_phi_limit;
  Real _half_psi_limit;
  Real _alpha_cohesion;
  Real _alpha_phi;
  Real _alpha_psi;
  Real _beta_cohesion;
  Real _beta_phi;
  Real _beta_psi;

  /// cohesion as a function of internal parameter
  virtual Real cohesion(const Real internal_param) const;

  /// d(cohesion)/d(internal_param);
  virtual Real dcohesion(const Real internal_param) const;

  /// friction angle as a function of internal parameter
  virtual Real phi(const Real internal_param) const;

  /// d(phi)/d(internal_param);
  virtual Real dphi(const Real internal_param) const;

  /// dilation angle as a function of internal parameter
  virtual Real psi(const Real internal_param) const;

  /// d(psi)/d(internal_param);
  virtual Real dpsi(const Real internal_param) const;

};

#endif // TENSORMECHANICSPLASTICMOHRCOULOMBCUBIC_H
