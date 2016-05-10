/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSPLASTICDRUCKERPRAGERHYPERBOLIC_H
#define TENSORMECHANICSPLASTICDRUCKERPRAGERHYPERBOLIC_H

#include "TensorMechanicsPlasticDruckerPrager.h"
#include "TensorMechanicsHardeningModel.h"

class TensorMechanicsPlasticDruckerPragerHyperbolic;

template<>
InputParameters validParams<TensorMechanicsPlasticDruckerPragerHyperbolic>();

/**
 * Rate-independent non-associative Drucker Prager
 * with hardening/softening.  The cone's tip is smoothed in a hyperbolic fashion
 * Most functions (eg flowPotential, etc) are simply inherited from
 * TensorMechanicsPlasticDruckerPrager.  Note df_dsig is over-ridden
 */
class TensorMechanicsPlasticDruckerPragerHyperbolic : public TensorMechanicsPlasticDruckerPrager
{
 public:
  TensorMechanicsPlasticDruckerPragerHyperbolic(const InputParameters & parameters);

  /// Returns the model name (DruckerPragerHyperbolic)
  virtual std::string modelName() const;

  /// Returns _use_custom_returnMap
  virtual bool useCustomReturnMap() const;

  /// Returns _use_custom_cto
  virtual bool useCustomCTO() const;

 protected:
  /**
   * The yield function
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the yield function
   */
  Real yieldFunction(const RankTwoTensor & stress, Real intnl) const;

  /**
   * The derivative of the flow potential with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return dr_dstress(i, j, k, l) = dr(i, j)/dstress(k, l)
   */
  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const;

  /// Function that's used in dyieldFunction_dstress and flowPotential
  virtual RankTwoTensor df_dsig(const RankTwoTensor & stress, Real bbb) const;

  /**
   * Performs a custom return-map.
   * You may choose to over-ride this in your
   * derived TensorMechanicsPlasticXXXX class,
   * and you may implement the return-map
   * algorithm in any way that suits you.  Eg, using
   * a Newton-Raphson approach, or a radial-return,
   * etc.
   * This may also be used as a quick way of ascertaining
   * whether (trial_stress, intnl_old) is in fact admissible.
   *
   * For over-riding this function, please note the
   * following.
   *
   * (1) Denoting the return value of the function by "successful_return",
   * the only possible output values should be:
   *   (A) trial_stress_inadmissible=false, successful_return=true.
   *       That is, (trial_stress, intnl_old) is in fact admissible
   *       (in the elastic domain).
   *   (B) trial_stress_inadmissible=true, successful_return=false.
   *       That is (trial_stress, intnl_old) is inadmissible
   *       (outside the yield surface), and you didn't return
   *       to the yield surface.
   *   (C) trial_stress_inadmissible=true, successful_return=true.
   *       That is (trial_stress, intnl_old) is inadmissible
   *       (outside the yield surface), but you did return
   *       to the yield surface.
   * The default implementation only handles case (A) and (B):
   * it does not attempt to do a return-map algorithm.
   *
   * (2) you must correctly signal "successful_return" using the
   * return value of this function.  Don't assume the calling function
   * will do Kuhn-Tucker checking and so forth!
   *
   * (3) In cases (A) and (B) you needn't set returned_stress,
   * returned_intnl, delta_dp, or dpm.  This is for computational
   * efficiency.
   *
   * (4) In cases (A) and (B), you MUST place the yield function
   * values at (trial_stress, intnl_old) into yf so the calling
   * function can use this information optimally.  You will have
   * already calculated these yield function values, which can be
   * quite expensive, and it's not very optimal for the calling
   * function to have to re-calculate them.
   *
   * (5) In case (C), you need to set:
   *   returned_stress (the returned value of stress)
   *   returned_intnl  (the returned value of the internal variable)
   *   delta_dp   (the change in plastic strain)
   *   dpm (the plastic multipliers needed to bring about the return)
   *   yf (yield function values at the returned configuration)
   *
   * (Note, if you over-ride returnMap, you will probably
   * want to override consistentTangentOpertor too, otherwise
   * it will default to E_ijkl.)
   *
   * @param trial_stress The trial stress
   * @param intnl_old Value of the internal parameter
   * @param E_ijkl Elasticity tensor
   * @param ep_plastic_tolerance Tolerance defined by the user for the plastic strain
   * @param[out] returned_stress In case (C): lies on the yield surface after returning and produces the correct plastic strain (normality condition).  Otherwise: not defined
   * @param[out] returned_intnl In case (C): the value of the internal parameter after returning.  Otherwise: not defined
   * @param[out] dpm  In case (C): the plastic multipliers needed to bring about the return.  Otherwise: not defined
   * @param[out] delta_dp In case (C): The change in plastic strain induced by the return process.  Otherwise: not defined
   * @param[out] yf In case (C): the yield function at (returned_stress, returned_intnl).  Otherwise: the yield function at (trial_stress, intnl_old)
   * @param[out] trial_stress_inadmissible Should be set to false if the trial_stress is admissible, and true if the trial_stress is inadmissible.  This can be used by the calling prorgram
   * @return true if a successful return (or a return-map not needed), false if the trial_stress is inadmissible but the return process failed
   */
  virtual bool returnMap(const RankTwoTensor & trial_stress, Real intnl_old, const RankFourTensor & E_ijkl,
                         Real ep_plastic_tolerance, RankTwoTensor & returned_stress, Real & returned_intnl,
                         std::vector<Real> & dpm, RankTwoTensor & delta_dp, std::vector<Real> & yf,
                         bool & trial_stress_inadmissible) const;

  /**
    * Calculates a custom consistent tangent operator.
    * You may choose to over-ride this in your
    * derived TensorMechanicsPlasticXXXX class.
    *
    * @param stress_old trial stress before returning
    * @param stress current stress state
    * @param intnl internal parameter
    * @param E_ijkl elasticity tensor
    * @param cumulative_pm the cumulative plastic multipliers
    * @return the consistent tangent operator for J2 (radial return) case
    */
  virtual RankFourTensor consistentTangentOperator(const RankTwoTensor & trial_stress, const RankTwoTensor & stress, Real intnl,
                                                   const RankFourTensor & E_ijkl, const std::vector<Real> & cumulative_pm) const;

 private:
  /// smoothing parameter for the cone's tip
  const Real _smoother2;

  /// whether to use the custom returnMap function
  const bool _use_custom_returnMap;

  /// Whether to use the custom consistent tangent operator calculation
  const bool _use_custom_cto;

  /// max iters for custom return map loop
  const unsigned _max_iters;
};

#endif // TENSORMECHANICSPLASTICDRUCKERPRAGERHYPERBOLIC_H
