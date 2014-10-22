#ifndef TENSORMECHANICSPLASTICWEAKPLANETENSILECUBIC_H
#define TENSORMECHANICSPLASTICWEAKPLANETENSILECUBIC_H

#include "TensorMechanicsPlasticWeakPlaneTensile.h"


class TensorMechanicsPlasticWeakPlaneTensileCubic;


template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneTensileCubic>();

/**
 * Rate-independent associative weak-plane tensile failure
 * with cubic hardening/softening
 */
class TensorMechanicsPlasticWeakPlaneTensileCubic : public TensorMechanicsPlasticWeakPlaneTensile
{
 public:
  TensorMechanicsPlasticWeakPlaneTensileCubic(const std::string & name, InputParameters parameters);

 protected:

  /// tension cutoff
  Real _tension;

  /// tension cutoff at infinite hardening/softening
  Real _tension_residual;

  /// Tensile strength = cubic between _tensile and _tension_residual
  Real _tension_limit;

  /// half _tension_limit
  Real _half_tension_limit;

  /// convenience parameter
  Real _alpha_tension;

  /// convenience parameter
  Real _beta_tension;

  /// tensile strength as a function of residual value, rate, and internal_param
  virtual Real tensile_strength(const Real internal_param) const;

  /// d(tensile strength)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dtensile_strength(const Real internal_param) const;
};

#endif // TENSORMECHANICSPLASTICWEAKPLANETENSILECUBIC_H
