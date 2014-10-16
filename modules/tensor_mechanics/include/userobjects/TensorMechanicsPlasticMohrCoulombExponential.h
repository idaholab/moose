#ifndef TENSORMECHANICSPLASTICMOHRCOULOMBEXPONENTIAL_H
#define TENSORMECHANICSPLASTICMOHRCOULOMBEXPONENTIAL_H

#include "TensorMechanicsPlasticMohrCoulomb.h"


class TensorMechanicsPlasticMohrCoulombExponential;


template<>
InputParameters validParams<TensorMechanicsPlasticMohrCoulombExponential>();

/**
 * Mohr-Coulomb plasticity, nonassociative with exponential hardening/softening.
 * The parameters cohesion, friction angle and dilation angle all obey
 * param = param_res - (param_res - param_0)*expoential(-rate*internal_parameter)
 */
class TensorMechanicsPlasticMohrCoulombExponential : public TensorMechanicsPlasticMohrCoulomb
{
 public:
  TensorMechanicsPlasticMohrCoulombExponential(const std::string & name, InputParameters parameters);

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

  /// Logarithmic rate of change of cohesion to _cohesion_residual
  Real _cohesion_rate;

  /// Logarithmic rate of change of _phi to _phi_residual
  Real _phi_rate;

  /// Logarithmic rate of change of _psi to _psi_residual
  Real _psi_rate;

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

#endif // TENSORMECHANICSPLASTICMOHRCOULOMBEXPONENTIAL_H
