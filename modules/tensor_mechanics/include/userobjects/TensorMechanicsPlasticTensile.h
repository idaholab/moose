#ifndef TENSORMECHANICSPLASTICTENSILE
#define TENSORMECHANICSPLASTICTENSILE

#include "TensorMechanicsPlasticModel.h"


class TensorMechanicsPlasticTensile;


template<>
InputParameters validParams<TensorMechanicsPlasticTensile>();

/**
 * FiniteStrainTensile implements rate-independent associative tensile failure
 * with hardening/softening in the finite-strain framework.
 * The smoothing of the tip of the yield-surface cone is described in
 * Zienkiewicz and Prande "Some useful forms of isotropic yield surfaces for soil and rock mechanics" (1977) In G Gudehus (editor) "Finite Elements in Geomechanics" Wile, Chichester, pp 179-190.
 * The smoothing of the edges of the cone is described in
 * AJ Abbo, AV Lyamin, SW Sloan, JP Hambleton "A C2 continuous approximation to the Mohr-Coulomb yield surface" International Journal of Solids and Structures 48 (2011) 3001-3010
 */
class TensorMechanicsPlasticTensile : public TensorMechanicsPlasticModel
{
 public:
  TensorMechanicsPlasticTensile(const std::string & name, InputParameters parameters);

  /**
   * The yield function
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the yield function
   */
  Real yieldFunction(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of yield function with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return df_dstress(i, j) = dyieldFunction/dstress(i, j)
   */
  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of yield function with respect to the internal parameter
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the derivative
   */
  Real dyieldFunction_dintnl(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The flow potential
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return the flow potential
   */
  RankTwoTensor flowPotential(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of the flow potential with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return dr_dstress(i, j, k, l) = dr(i, j)/dstress(k, l)
   */
  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of the flow potential with respect to the internal parameter
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return dr_dintnl(i, j) = dr(i, j)/dintnl
   */
  RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, const Real & intnl) const;

 protected:

  /// tensile strength at zero hardening/softening
  Real _tensile_strength0;

  /// tensile strength at infinite hardening/softening
  Real _tensile_strength_residual;

  /// Tensile strength = tensile_strength_residual + (tensile_strength - tensile_strength_residual)*exp(-tensile_strength_rate*plasticstrain).
  Real _tensile_strength_rate;

  /// Square of tip smoothing parameter to smooth the cone at mean_stress = T
  Real _small_smoother2;

  /// edge smoothing parameter, in radians
  Real _tt;

  /// sin(3*_tt) - useful for making comparisons with Lode angle
  Real _sin3tt;

  /// if secondInvariant < _lode_cutoff then set Lode angle to zero.  This is to guard against precision-loss
  Real _lode_cutoff;

  /// Abbo et al's C parameter
  Real _ccc;

  /// Abbo et al's B parameter
  Real _bbb;

  /// Abbo et al's A parameter
  Real _aaa;

  /// tensile strength as a function of residual value, rate, and internal_param
  virtual Real tensile_strength(const Real internal_param) const;

  /// d(tensile strength)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dtensile_strength(const Real internal_param) const;


};

#endif // TENSORMECHANICSPLASTICTENSILE
