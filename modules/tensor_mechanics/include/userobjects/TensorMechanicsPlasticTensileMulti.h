/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSPLASTICTENSILEMULTI_H
#define TENSORMECHANICSPLASTICTENSILEMULTI_H

#include "TensorMechanicsPlasticModel.h"
#include "TensorMechanicsHardeningModel.h"

class TensorMechanicsPlasticTensileMulti;

template<>
InputParameters validParams<TensorMechanicsPlasticTensileMulti>();

/**
 * FiniteStrainTensileMulti implements rate-independent associative tensile failure
 * with hardening/softening in the finite-strain framework, using planar (non-smoothed) surfaces
 */
class TensorMechanicsPlasticTensileMulti : public TensorMechanicsPlasticModel
{
 public:
  TensorMechanicsPlasticTensileMulti(const InputParameters & parameters);

  /// The number of yield surfaces for this plasticity model
  virtual unsigned int numberSurfaces() const;

  /**
   * Calculates the yield functions.  Note that for single-surface plasticity
   * you don't want to override this - override the private yieldFunction below
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @param[out] f the yield functions
   */
  virtual void yieldFunctionV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & f) const;

  /**
   * The derivative of yield functions with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @param[out] df_dstress df_dstress[alpha](i, j) = dyieldFunction[alpha]/dstress(i, j)
   */
  virtual void dyieldFunction_dstressV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & df_dstress) const;

  /**
   * The derivative of yield functions with respect to the internal parameter
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @param[out] df_dintnl df_dintnl[alpha] = df[alpha]/dintnl
   */
  virtual void dyieldFunction_dintnlV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & df_dintnl) const;

  /**
   * The flow potentials
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @param[out] r r[alpha] is the flow potential for the "alpha" yield function
   */
  virtual void flowPotentialV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & r) const;

  /**
   * The derivative of the flow potential with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @param[out] dr_dstress dr_dstress[alpha](i, j, k, l) = dr[alpha](i, j)/dstress(k, l)
   */
  virtual void dflowPotential_dstressV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankFourTensor> & dr_dstress) const;

  /**
   * The derivative of the flow potential with respect to the internal parameter
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @param[out] dr_dintnl  dr_dintnl[alpha](i, j) = dr[alpha](i, j)/dintnl
   */
  virtual void dflowPotential_dintnlV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & dr_dintnl) const;

  /**
   * The active yield surfaces, given a vector of yield functions.
   * This is used by FiniteStrainMultiPlasticity to determine the initial
   * set of active constraints at the trial (stress, intnl) configuration.
   * It is up to you (the coder) to determine how accurate you want the
   * returned_stress to be.  Currently it is only used by FiniteStrainMultiPlasticity
   * to estimate a good starting value for the Newton-Rahson procedure,
   * so currently it may not need to be super perfect.
   * @param f values of the yield functions
   * @param stress stress tensor
   * @param intnl internal parameter
   * @param Eijkl elasticity tensor (stress = Eijkl*strain)
   * @param[out] act act[i] = true if the i_th yield function is active
   * @param[out] returned_stress Approximate value of the returned stress
   */
  virtual void activeConstraints(const std::vector<Real> & f, const RankTwoTensor & stress, const Real & intnl, const RankFourTensor & Eijkl, std::vector<bool> & act, RankTwoTensor & returned_stress) const;

  /// Returns the model name (TensileMulti)
  virtual std::string modelName() const;

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
  virtual bool returnMap(const RankTwoTensor & trial_stress, const Real & intnl_old, const RankFourTensor & E_ijkl,
                                  Real ep_plastic_tolerance, RankTwoTensor & returned_stress, Real & returned_intnl,
                                  std::vector<Real> & dpm, RankTwoTensor & delta_dp, std::vector<Real> & yf,
                                  bool & trial_stress_inadmissible) const;


 protected:
  const TensorMechanicsHardeningModel & _strength;

  /// whether to use the custom return-map algorithm
  bool _use_custom_returnMap;

  /// whether to use the custom consistent tangent operator algorithm
  bool _use_custom_cto;

  /// maximum iterations allowed in the custom return-map algorithm
  unsigned int _max_iters;

  /// yield function is shifted by this amount to avoid problems with stress-derivatives at equal eigenvalues
  Real _shift;


  /// tensile strength as a function of residual value, rate, and internal_param
  virtual Real tensile_strength(const Real internal_param) const;

  /// d(tensile strength)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dtensile_strength(const Real internal_param) const;

  /// dot product of two 3-dimensional vectors
  Real dot(const std::vector<Real> & a, const std::vector<Real> & b) const;

  /// triple product of three 3-dimensional vectors
  Real triple(const std::vector<Real> & a, const std::vector<Real> & b, const std::vector<Real> & c) const;

  bool returnTip(const std::vector<Real> & eigvals, const std::vector<std::vector<Real> > & n,
                 std::vector<Real> & dpm, RankTwoTensor & returned_stress, const Real & intnl_old) const;

  bool returnEdge(const std::vector<Real> & eigvals, const std::vector<std::vector<Real> > & n,
                 std::vector<Real> & dpm, RankTwoTensor & returned_stress, const Real & intnl_old) const;

  bool returnPlane(const std::vector<Real> & eigvals, const std::vector<std::vector<Real> > & n,
                 std::vector<Real> & dpm, RankTwoTensor & returned_stress, const Real & intnl_old) const;

  bool KuhnTuckerOK(const RankTwoTensor & returned_diagonal_stress, const std::vector<Real> & dpm, const Real & str) const;

  virtual RankFourTensor consistentTangentOperator(const RankTwoTensor & trial_stress, const RankTwoTensor & stress, const Real & intnl,
                                                   const RankFourTensor & E_ijkl, const std::vector<Real> & cumulative_pm) const;

  enum return_type { tip=0, edge=1, plane=2 };

};

#endif // TENSORMECHANICSPLASTICTENSILEMULTI_H
