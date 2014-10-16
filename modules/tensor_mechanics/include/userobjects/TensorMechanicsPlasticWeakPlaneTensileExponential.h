#ifndef TENSORMECHANICSPLASTICWEAKPLANETENSILEEXPONENTIAL_H
#define TENSORMECHANICSPLASTICWEAKPLANETENSILEEXPONENTIAL_H

#include "TensorMechanicsPlasticWeakPlaneTensile.h"


class TensorMechanicsPlasticWeakPlaneTensileExponential;


template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneTensileExponential>();

/**
 * Rate-independent associative weak-plane tensile failure
 * with exponential hardening/softening
 */
class TensorMechanicsPlasticWeakPlaneTensileExponential : public TensorMechanicsPlasticWeakPlaneTensile
{
 public:
  TensorMechanicsPlasticWeakPlaneTensileExponential(const std::string & name, InputParameters parameters);

 protected:

  /// tension cutoff
  Real _tension_cutoff;

  /// tension cutoff at infinite hardening/softening
  Real _tension_cutoff_residual;

  /// Tensile strength = wpt_tensile_strenght_residual + (wpt_tensile_strength - wpt_tensile_strength_residual)*exp(-wpt_tensile_rate*plasticstrain).
  Real _tension_cutoff_rate;

  /// tensile strength as a function of residual value, rate, and internal_param
  virtual Real tensile_strength(const Real internal_param) const;

  /// d(tensile strength)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dtensile_strength(const Real internal_param) const;
};

#endif // TENSORMECHANICSPLASTICWEAKPLANETENSILEEXPONENTIAL_H
