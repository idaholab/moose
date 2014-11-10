#ifndef TENSORMECHANICSPLASTICTENSILEEXPONENTIAL
#define TENSORMECHANICSPLASTICTENSILEEXPONENTIAL

#include "TensorMechanicsPlasticTensile.h"


class TensorMechanicsPlasticTensileExponential;


template<>
InputParameters validParams<TensorMechanicsPlasticTensileExponential>();

/**
 * FiniteStrainTensile implements rate-independent associative tensile failure
 * with exponential hardening/softening in the finite-strain framework.
 */
class TensorMechanicsPlasticTensileExponential : public TensorMechanicsPlasticTensile
{
 public:
  TensorMechanicsPlasticTensileExponential(const std::string & name, InputParameters parameters);


 protected:

  /// tensile strength at zero hardening/softening
  Real _tensile_strength0;

  /// tensile strength at infinite hardening/softening
  Real _tensile_strength_residual;

  /// Tensile strength = tensile_strength_residual + (tensile_strength - tensile_strength_residual)*exp(-tensile_strength_rate*plasticstrain).
  Real _tensile_strength_rate;

  /// tensile strength as a function of residual value, rate, and internal_param
  virtual Real tensile_strength(const Real internal_param) const;

  /// d(tensile strength)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dtensile_strength(const Real internal_param) const;


};

#endif // TENSORMECHANICSPLASTICTENSILEEXPONENTIAL
