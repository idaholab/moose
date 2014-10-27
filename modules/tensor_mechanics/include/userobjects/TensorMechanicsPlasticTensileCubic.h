#ifndef TENSORMECHANICSPLASTICTENSILECUBIC
#define TENSORMECHANICSPLASTICTENSILECUBIC

#include "TensorMechanicsPlasticTensile.h"


class TensorMechanicsPlasticTensileCubic;


template<>
InputParameters validParams<TensorMechanicsPlasticTensileCubic>();

/**
 * FiniteStrainTensileCubic implements rate-independent associative tensile failure
 * with cubic hardening/softening in the finite-strain framework.
 */
class TensorMechanicsPlasticTensileCubic : public TensorMechanicsPlasticTensile
{
 public:
  TensorMechanicsPlasticTensileCubic(const std::string & name, InputParameters parameters);


 protected:

  /// tensile strength at zero hardening/softening
  Real _tensile_strength0;

  /// tensile strength at infinite hardening/softening
  Real _tensile_strength_residual;

  /// Tensile strength = cubic between _tensile_strength0 (at internal_param = 0), and _tensile_strength_residual (at internal_param = _tensile_strength_limit)
  Real _tensile_strength_limit;

  Real _half_tensile_strength_limit;
  Real _alpha_tension;
  Real _beta_tension;

  /// tensile strength as a function of residual value, rate, and internal_param
  virtual Real tensile_strength(const Real internal_param) const;

  /// d(tensile strength)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dtensile_strength(const Real internal_param) const;


};

#endif // TENSORMECHANICSPLASTICTENSILECUBIC
