#ifndef TENSORMECHANICSPLASTICWEAKPLANESHEARCUBIC_H
#define TENSORMECHANICSPLASTICWEAKPLANESHEARCUBIC_H

#include "TensorMechanicsPlasticWeakPlaneShear.h"


class TensorMechanicsPlasticWeakPlaneShearCubic;


template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneShearCubic>();

/**
 * Rate-independent associative weak-plane tensile failure
 * with cubic hardening/softening
 * The parameters cohesion, tan(friction angle) and tan(dilation angle) are all
 * cubic functions of the internal parameter, with the cubic chosen so
 * param = param_0 at internal_parameter = 0
 * param = param_res at internal_parameter = limit
 * param is a C1 function
 */
class TensorMechanicsPlasticWeakPlaneShearCubic : public TensorMechanicsPlasticWeakPlaneShear
{
 public:
  TensorMechanicsPlasticWeakPlaneShearCubic(const std::string & name, InputParameters parameters);

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

  /// d(cohesion)/d(internal_param)
  virtual Real dcohesion(const Real internal_param) const;

  /// tan_phi as a function of internal parameter
  virtual Real tan_phi(const Real internal_param) const;

  /// d(tan_phi)/d(internal_param);
  virtual Real dtan_phi(const Real internal_param) const;

  /// tan_psi as a function of internal parameter
  virtual Real tan_psi(const Real internal_param) const;

  /// d(tan_psi)/d(internal_param);
  virtual Real dtan_psi(const Real internal_param) const;

};

#endif // TENSORMECHANICSPLASTICWEAKPLANESHEARCUBIC_H
