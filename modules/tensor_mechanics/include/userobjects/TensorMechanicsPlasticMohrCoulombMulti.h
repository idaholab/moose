/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TENSORMECHANICSPLASTICMOHRCOULOMBMULTI_H
#define TENSORMECHANICSPLASTICMOHRCOULOMBMULTI_H

#include "TensorMechanicsPlasticModel.h"
#include "TensorMechanicsHardeningModel.h"

class TensorMechanicsPlasticMohrCoulombMulti;

template<>
InputParameters validParams<TensorMechanicsPlasticMohrCoulombMulti>();

/**
 * FiniteStrainMohrCoulombMulti implements rate-independent non-associative mohr-coulomb
 * with hardening/softening in the finite-strain framework, using planar (non-smoothed) surfaces
 */
class TensorMechanicsPlasticMohrCoulombMulti : public TensorMechanicsPlasticModel
{
 public:
  TensorMechanicsPlasticMohrCoulombMulti(const InputParameters & parameters);

  /// The number of yield surfaces for this plasticity model
  virtual unsigned int numberSurfaces() const;

  /**
   * Calculates the yield functions.  Note that for single-surface plasticity
   * you don't want to override this - override the private yieldFunction below
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @param[out] f the yield functions
   */
  virtual void yieldFunctionV(const RankTwoTensor & stress, Real intnl, std::vector<Real> & f) const;

  /**
   * The derivative of yield functions with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @param[out] df_dstress df_dstress[alpha](i, j) = dyieldFunction[alpha]/dstress(i, j)
   */
  virtual void dyieldFunction_dstressV(const RankTwoTensor & stress, Real intnl, std::vector<RankTwoTensor> & df_dstress) const;

  /**
   * The derivative of yield functions with respect to the internal parameter
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @param[out] df_dintnl df_dintnl[alpha] = df[alpha]/dintnl
   */
  virtual void dyieldFunction_dintnlV(const RankTwoTensor & stress, Real intnl, std::vector<Real> & df_dintnl) const;

  /**
   * The flow potentials
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @param[out] r r[alpha] is the flow potential for the "alpha" yield function
   */
  virtual void flowPotentialV(const RankTwoTensor & stress, Real intnl, std::vector<RankTwoTensor> & r) const;

  /**
   * The derivative of the flow potential with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @param[out] dr_dstress dr_dstress[alpha](i, j, k, l) = dr[alpha](i, j)/dstress(k, l)
   */
  virtual void dflowPotential_dstressV(const RankTwoTensor & stress, Real intnl, std::vector<RankFourTensor> & dr_dstress) const;

  /**
   * The derivative of the flow potential with respect to the internal parameter
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @param[out] dr_dintnl  dr_dintnl[alpha](i, j) = dr[alpha](i, j)/dintnl
   */
  virtual void dflowPotential_dintnlV(const RankTwoTensor & stress, Real intnl, std::vector<RankTwoTensor> & dr_dintnl) const;

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
  virtual void activeConstraints(const std::vector<Real> & f, const RankTwoTensor & stress, Real intnl, const RankFourTensor & Eijkl, std::vector<bool> & act, RankTwoTensor & returned_stress) const;

  /// Returns the model name (MohrCoulombMulti)
  virtual std::string modelName() const;

  /// Returns _use_custom_returnMap
  virtual bool useCustomReturnMap() const;

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

 protected:

  /// cohesion as a function of residual value, rate, and internal_param
  virtual Real cohesion(const Real internal_param) const;

  /// d(cohesion)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dcohesion(const Real internal_param) const;

  /// phi as a function of residual value, rate, and internal_param
  virtual Real phi(const Real internal_param) const;

  /// d(phi)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dphi(const Real internal_param) const;

  /// psi as a function of residual value, rate, and internal_param
  virtual Real psi(const Real internal_param) const;

  /// d(psi)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dpsi(const Real internal_param) const;

 private:

  /// Hardening model for cohesion
  const TensorMechanicsHardeningModel & _cohesion;

  /// Hardening model for phi
  const TensorMechanicsHardeningModel & _phi;

  /// Hardening model for psi
  const TensorMechanicsHardeningModel & _psi;

  /// Maximum Newton-Raphison iterations in the custom returnMap algorithm
  const unsigned int _max_iters;

  /// yield function is shifted by this amount to avoid problems with stress-derivatives at equal eigenvalues
  const Real _shift;

  /// Whether to use the custom return-map algorithm
  const bool _use_custom_returnMap;

  /**
   * Calculates the yield functions given the eigenvalues of stress
   * @param e0 Smallest eigenvalue
   * @param e1 Middle eigenvalue
   * @param e2 Largest eigenvalue
   * @param sinphi sin(friction angle)
   * @param cohcos cohesion*cos(friction angle)
   * @param[out] f the yield functions
   */
  void yieldFunctionEigvals(Real e0, Real e1, Real e2, Real sinphi, Real cohcos, std::vector<Real> & f) const;

  /**
   * this is exactly dyieldFunction_dstress, or flowPotential,
   * depending on whether sin_angle = sin(phi), or sin_angle = sin(psi),
   * respectively
   */
  void df_dsig(const RankTwoTensor & stress, Real sin_angle, std::vector<RankTwoTensor> & df) const;

  /**
   * perturbs the stress tensor in the case of almost-equal eigenvalues.
   * Note that, upon entry, this algorithm assumes that eigvals are the eigvenvalues of stress
   * @param stress input stress
   * @param[in,out] eigvals eigenvalues after perturbing.
   * @param[in,out] deigvals d(eigenvalues)/dstress_ij after perturbing.
   */
  void perturbStress(const RankTwoTensor & stress, std::vector<Real> & eigvals, std::vector<RankTwoTensor> & deigvals) const;

  /**
   * Returns true if the Kuhn-Tucker conditions are satisfied
   * @param yf The six yield function values
   * @param dpm The six plastic multipliers
   * @param ep_plastic_tolerance The tolerance on the plastic strain (if dpm>-ep_plastic_tolerance then it is grouped as "non-negative" in the Kuhn-Tucker conditions).
   */
  bool KuhnTuckerOK(const std::vector<Real> & yf, const std::vector<Real> & dpm, Real ep_plastic_tolerance) const;

  /**
   * See doco for returnMap function.  The interface is identical
   * to this one.  This one can be called internally regardless of
   * the value of _use_custom_returnMap
   */
  bool doReturnMap(const RankTwoTensor & trial_stress, Real intnl_old, const RankFourTensor & E_ijkl,
                   Real ep_plastic_tolerance, RankTwoTensor & returned_stress, Real & returned_intnl,
                   std::vector<Real> & dpm, RankTwoTensor & delta_dp, std::vector<Real> & yf,
                   bool & trial_stress_inadmissible) const;

  /**
   * Tries to return-map to the MC tip using the THREE directions
   * given in n, and THREE dpm values are returned.
   * Note that you must supply THREE suitale n vectors out of the
   * total of SIX flow directions, and then interpret the THREE
   * dpm values appropriately.
   * The return value is true if the internal Newton-Raphson
   * process has converged and Kuhn-Tucker is satisfied, otherwise it is false
   * If the return value is false and/or nr_converged=false then the
   * "out" parameters (sinphi, cohcos, yf, returned_stress, dpm) will be junk.
   * @param eigvals The three stress eigenvalues, sorted in ascending order
   * @param n The three return directions, n=E_ijkl*r.  Note this algorithm assumes isotropic elasticity, so these are 3 vectors in principal stress space
   * @param dpm[out] The three plastic multipliers resulting from the return-map to the tip.
   * @param returned_stress[out] The returned stress.  This will be diagonal, with the return-mapped eigenvalues in the diagonal positions, sorted in ascending order
   * @param intnl_old The internal parameter at stress=eigvals.  This algorithm doesn't form the plastic strain, so you will have to use intnl=intnl_old+sum(dpm) if you need the new internal-parameter value at the returned point.
   * @param sinphi[out] The new value of sin(friction angle) after returning.
   * @param cohcos[out] The new value of cohesion*cos(friction angle) after returning.
   * @param initial_guess A guess for sum(dpm)
   * @param nr_converged[out] Whether the internal Newton-Raphson process converged
   * @param ep_plastic_tolerance The user-set tolerance on the plastic strain.
   * @param yf[out] The yield functions after return
   */
  bool returnTip(const std::vector<Real> & eigvals, const std::vector<RealVectorValue> & n,
                 std::vector<Real> & dpm, RankTwoTensor & returned_stress, Real intnl_old,
                 Real & sinphi, Real & cohcos, Real initial_guess, bool & nr_converged,
                 Real ep_plastic_tolerance, std::vector<Real> & yf) const;

  /**
   * Tries to return-map to the MC plane using the n[3] direction
   * The return value is true if the internal Newton-Raphson
   * process has converged and Kuhn-Tucker is satisfied, otherwise it is false
   * If the return value is false and/or nr_converged=false then the
   * "out" parameters (sinphi, cohcos, yf, returned_stress) will be junk.
   * @param eigvals The three stress eigenvalues, sorted in ascending order
   * @param n The six return directions, n=E_ijkl*r.  Note this algorithm assumes isotropic elasticity, so these are vectors in principal stress space
   * @param dpm[out] The six plastic multipliers resulting from the return-map to the plane
   * @param returned_stress[out] The returned stress.  This will be diagonal, with the return-mapped eigenvalues in the diagonal positions, sorted in ascending order
   * @param intnl_old The internal parameter at stress=eigvals.  This algorithm doesn't form the plastic strain, so you will have to use intnl=intnl_old+sum(dpm) if you need the new internal-parameter value at the returned point.
   * @param sinphi[out] The new value of sin(friction angle) after returning.
   * @param cohcos[out] The new value of cohesion*cos(friction angle) after returning.
   * @param initial_guess A guess for sum(dpm)
   * @param nr_converged[out] Whether the internal Newton-Raphson process converged
   * @param ep_plastic_tolerance The user-set tolerance on the plastic strain.
   * @param yf[out] The yield functions after return
   */
  bool returnPlane(const std::vector<Real> & eigvals, const std::vector<RealVectorValue> & n,
                   std::vector<Real> & dpm, RankTwoTensor & returned_stress, Real intnl_old,
                   Real & sinphi, Real & cohcos, Real initial_guess, bool & nr_converged,
                   Real ep_plastic_tolerance, std::vector<Real> & yf) const;

  /**
   * Tries to return-map to the MC edge using the n[4] and n[6] directions
   * The return value is true if the internal Newton-Raphson
   * process has converged and Kuhn-Tucker is satisfied, otherwise it is false
   * If the return value is false and/or nr_converged=false then the
   * "out" parameters (sinphi, cohcos, yf, returned_stress) will be junk.
   * @param eigvals The three stress eigenvalues, sorted in ascending order
   * @param n The six return directions, n=E_ijkl*r.  Note this algorithm assumes isotropic elasticity, so these are vectors in principal stress space
   * @param dpm[out] The six plastic multipliers resulting from the return-map to the edge
   * @param returned_stress[out] The returned stress.  This will be diagonal, with the return-mapped eigenvalues in the diagonal positions, sorted in ascending order
   * @param intnl_old The internal parameter at stress=eigvals.  This algorithm doesn't form the plastic strain, so you will have to use intnl=intnl_old+sum(dpm) if you need the new internal-parameter value at the returned point.
   * @param sinphi[out] The new value of sin(friction angle) after returning.
   * @param cohcos[out] The new value of cohesion*cos(friction angle) after returning.
   * @param mag_E An approximate value for the magnitude of the Young's modulus.  This is used to set appropriate tolerances in the Newton-Raphson procedure
   * @param initial_guess A guess for sum(dpm)
   * @param nr_converged[out] Whether the internal Newton-Raphson process converged
   * @param ep_plastic_tolerance The user-set tolerance on the plastic strain.
   * @param yf[out] The yield functions after return
   */
  bool returnEdge000101(const std::vector<Real> & eigvals, const std::vector<RealVectorValue> & n,
                        std::vector<Real> & dpm, RankTwoTensor & returned_stress, Real intnl_old,
                        Real & sinphi, Real & cohcos, Real initial_guess, Real mag_E,
                        bool & nr_converged, Real ep_plastic_tolerance, std::vector<Real> & yf) const;

  /**
   * Tries to return-map to the MC edge using the n[1] and n[3] directions
   * The return value is true if the internal Newton-Raphson
   * process has converged and Kuhn-Tucker is satisfied, otherwise it is false
   * If the return value is false and/or nr_converged=false then the
   * "out" parameters (sinphi, cohcos, yf, returned_stress) will be junk.
   * @param eigvals The three stress eigenvalues, sorted in ascending order
   * @param n The six return directions, n=E_ijkl*r.  Note this algorithm assumes isotropic elasticity, so these are vectors in principal stress space
   * @param dpm[out] The six plastic multipliers resulting from the return-map to the edge
   * @param returned_stress[out] The returned stress.  This will be diagonal, with the return-mapped eigenvalues in the diagonal positions, sorted in ascending order
   * @param intnl_old The internal parameter at stress=eigvals.  This algorithm doesn't form the plastic strain, so you will have to use intnl=intnl_old+sum(dpm) if you need the new internal-parameter value at the returned point.
   * @param sinphi[out] The new value of sin(friction angle) after returning.
   * @param cohcos[out] The new value of cohesion*cos(friction angle) after returning.
   * @param mag_E An approximate value for the magnitude of the Young's modulus.  This is used to set appropriate tolerances in the Newton-Raphson procedure
   * @param initial_guess A guess for sum(dpm)
   * @param nr_converged[out] Whether the internal Newton-Raphson process converged
   * @param ep_plastic_tolerance The user-set tolerance on the plastic strain.
   * @param yf[out] The yield functions after return
   */
  bool returnEdge010100(const std::vector<Real> & eigvals, const std::vector<RealVectorValue> & n,
                        std::vector<Real> & dpm, RankTwoTensor & returned_stress, Real intnl_old,
                        Real & sinphi, Real & cohcos, Real initial_guess, Real mag_E,
                        bool & nr_converged, Real ep_plastic_tolerance, std::vector<Real> & yf) const;

  enum return_type { tip110100=0, tip010101=1, edge010100=2, edge000101=3, plane000100=4 };

};

#endif // TENSORMECHANICSPLASTICMOHRCOULOMBMULTI_H
