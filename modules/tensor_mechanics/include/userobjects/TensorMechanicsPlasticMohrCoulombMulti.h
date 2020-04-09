//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TensorMechanicsPlasticModel.h"
#include "TensorMechanicsHardeningModel.h"

/**
 * FiniteStrainMohrCoulombMulti implements rate-independent non-associative mohr-coulomb
 * with hardening/softening in the finite-strain framework, using planar (non-smoothed) surfaces
 */
class TensorMechanicsPlasticMohrCoulombMulti : public TensorMechanicsPlasticModel
{
public:
  static InputParameters validParams();

  TensorMechanicsPlasticMohrCoulombMulti(const InputParameters & parameters);

  /// The number of yield surfaces for this plasticity model
  virtual unsigned int numberSurfaces() const override;

  virtual void
  yieldFunctionV(const RankTwoTensor & stress, Real intnl, std::vector<Real> & f) const override;

  virtual void dyieldFunction_dstressV(const RankTwoTensor & stress,
                                       Real intnl,
                                       std::vector<RankTwoTensor> & df_dstress) const override;

  virtual void dyieldFunction_dintnlV(const RankTwoTensor & stress,
                                      Real intnl,
                                      std::vector<Real> & df_dintnl) const override;

  virtual void flowPotentialV(const RankTwoTensor & stress,
                              Real intnl,
                              std::vector<RankTwoTensor> & r) const override;

  virtual void dflowPotential_dstressV(const RankTwoTensor & stress,
                                       Real intnl,
                                       std::vector<RankFourTensor> & dr_dstress) const override;

  virtual void dflowPotential_dintnlV(const RankTwoTensor & stress,
                                      Real intnl,
                                      std::vector<RankTwoTensor> & dr_dintnl) const override;

  virtual void activeConstraints(const std::vector<Real> & f,
                                 const RankTwoTensor & stress,
                                 Real intnl,
                                 const RankFourTensor & Eijkl,
                                 std::vector<bool> & act,
                                 RankTwoTensor & returned_stress) const override;

  virtual std::string modelName() const override;

  virtual bool useCustomReturnMap() const override;

  virtual bool returnMap(const RankTwoTensor & trial_stress,
                         Real intnl_old,
                         const RankFourTensor & E_ijkl,
                         Real ep_plastic_tolerance,
                         RankTwoTensor & returned_stress,
                         Real & returned_intnl,
                         std::vector<Real> & dpm,
                         RankTwoTensor & delta_dp,
                         std::vector<Real> & yf,
                         bool & trial_stress_inadmissible) const override;

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
  void yieldFunctionEigvals(
      Real e0, Real e1, Real e2, Real sinphi, Real cohcos, std::vector<Real> & f) const;

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
  void perturbStress(const RankTwoTensor & stress,
                     std::vector<Real> & eigvals,
                     std::vector<RankTwoTensor> & deigvals) const;

  /**
   * Returns true if the Kuhn-Tucker conditions are satisfied
   * @param yf The six yield function values
   * @param dpm The six plastic multipliers
   * @param ep_plastic_tolerance The tolerance on the plastic strain (if dpm>-ep_plastic_tolerance
   * then it is grouped as "non-negative" in the Kuhn-Tucker conditions).
   */
  bool KuhnTuckerOK(const std::vector<Real> & yf,
                    const std::vector<Real> & dpm,
                    Real ep_plastic_tolerance) const;

  /**
   * See doco for returnMap function.  The interface is identical
   * to this one.  This one can be called internally regardless of
   * the value of _use_custom_returnMap
   */
  bool doReturnMap(const RankTwoTensor & trial_stress,
                   Real intnl_old,
                   const RankFourTensor & E_ijkl,
                   Real ep_plastic_tolerance,
                   RankTwoTensor & returned_stress,
                   Real & returned_intnl,
                   std::vector<Real> & dpm,
                   RankTwoTensor & delta_dp,
                   std::vector<Real> & yf,
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
   * @param n The three return directions, n=E_ijkl*r.  Note this algorithm assumes isotropic
   * elasticity, so these are 3 vectors in principal stress space
   * @param dpm[out] The three plastic multipliers resulting from the return-map to the tip.
   * @param returned_stress[out] The returned stress.  This will be diagonal, with the return-mapped
   * eigenvalues in the diagonal positions, sorted in ascending order
   * @param intnl_old The internal parameter at stress=eigvals.  This algorithm doesn't form the
   * plastic strain, so you will have to use intnl=intnl_old+sum(dpm) if you need the new
   * internal-parameter value at the returned point.
   * @param sinphi[out] The new value of sin(friction angle) after returning.
   * @param cohcos[out] The new value of cohesion*cos(friction angle) after returning.
   * @param initial_guess A guess for sum(dpm)
   * @param nr_converged[out] Whether the internal Newton-Raphson process converged
   * @param ep_plastic_tolerance The user-set tolerance on the plastic strain.
   * @param yf[out] The yield functions after return
   */
  bool returnTip(const std::vector<Real> & eigvals,
                 const std::vector<RealVectorValue> & n,
                 std::vector<Real> & dpm,
                 RankTwoTensor & returned_stress,
                 Real intnl_old,
                 Real & sinphi,
                 Real & cohcos,
                 Real initial_guess,
                 bool & nr_converged,
                 Real ep_plastic_tolerance,
                 std::vector<Real> & yf) const;

  /**
   * Tries to return-map to the MC plane using the n[3] direction
   * The return value is true if the internal Newton-Raphson
   * process has converged and Kuhn-Tucker is satisfied, otherwise it is false
   * If the return value is false and/or nr_converged=false then the
   * "out" parameters (sinphi, cohcos, yf, returned_stress) will be junk.
   * @param eigvals The three stress eigenvalues, sorted in ascending order
   * @param n The six return directions, n=E_ijkl*r.  Note this algorithm assumes isotropic
   * elasticity, so these are vectors in principal stress space
   * @param dpm[out] The six plastic multipliers resulting from the return-map to the plane
   * @param returned_stress[out] The returned stress.  This will be diagonal, with the return-mapped
   * eigenvalues in the diagonal positions, sorted in ascending order
   * @param intnl_old The internal parameter at stress=eigvals.  This algorithm doesn't form the
   * plastic strain, so you will have to use intnl=intnl_old+sum(dpm) if you need the new
   * internal-parameter value at the returned point.
   * @param sinphi[out] The new value of sin(friction angle) after returning.
   * @param cohcos[out] The new value of cohesion*cos(friction angle) after returning.
   * @param initial_guess A guess for sum(dpm)
   * @param nr_converged[out] Whether the internal Newton-Raphson process converged
   * @param ep_plastic_tolerance The user-set tolerance on the plastic strain.
   * @param yf[out] The yield functions after return
   */
  bool returnPlane(const std::vector<Real> & eigvals,
                   const std::vector<RealVectorValue> & n,
                   std::vector<Real> & dpm,
                   RankTwoTensor & returned_stress,
                   Real intnl_old,
                   Real & sinphi,
                   Real & cohcos,
                   Real initial_guess,
                   bool & nr_converged,
                   Real ep_plastic_tolerance,
                   std::vector<Real> & yf) const;

  /**
   * Tries to return-map to the MC edge using the n[4] and n[6] directions
   * The return value is true if the internal Newton-Raphson
   * process has converged and Kuhn-Tucker is satisfied, otherwise it is false
   * If the return value is false and/or nr_converged=false then the
   * "out" parameters (sinphi, cohcos, yf, returned_stress) will be junk.
   * @param eigvals The three stress eigenvalues, sorted in ascending order
   * @param n The six return directions, n=E_ijkl*r.  Note this algorithm assumes isotropic
   * elasticity, so these are vectors in principal stress space
   * @param dpm[out] The six plastic multipliers resulting from the return-map to the edge
   * @param returned_stress[out] The returned stress.  This will be diagonal, with the return-mapped
   * eigenvalues in the diagonal positions, sorted in ascending order
   * @param intnl_old The internal parameter at stress=eigvals.  This algorithm doesn't form the
   * plastic strain, so you will have to use intnl=intnl_old+sum(dpm) if you need the new
   * internal-parameter value at the returned point.
   * @param sinphi[out] The new value of sin(friction angle) after returning.
   * @param cohcos[out] The new value of cohesion*cos(friction angle) after returning.
   * @param mag_E An approximate value for the magnitude of the Young's modulus.  This is used to
   * set appropriate tolerances in the Newton-Raphson procedure
   * @param initial_guess A guess for sum(dpm)
   * @param nr_converged[out] Whether the internal Newton-Raphson process converged
   * @param ep_plastic_tolerance The user-set tolerance on the plastic strain.
   * @param yf[out] The yield functions after return
   */
  bool returnEdge000101(const std::vector<Real> & eigvals,
                        const std::vector<RealVectorValue> & n,
                        std::vector<Real> & dpm,
                        RankTwoTensor & returned_stress,
                        Real intnl_old,
                        Real & sinphi,
                        Real & cohcos,
                        Real initial_guess,
                        Real mag_E,
                        bool & nr_converged,
                        Real ep_plastic_tolerance,
                        std::vector<Real> & yf) const;

  /**
   * Tries to return-map to the MC edge using the n[1] and n[3] directions
   * The return value is true if the internal Newton-Raphson
   * process has converged and Kuhn-Tucker is satisfied, otherwise it is false
   * If the return value is false and/or nr_converged=false then the
   * "out" parameters (sinphi, cohcos, yf, returned_stress) will be junk.
   * @param eigvals The three stress eigenvalues, sorted in ascending order
   * @param n The six return directions, n=E_ijkl*r.  Note this algorithm assumes isotropic
   * elasticity, so these are vectors in principal stress space
   * @param dpm[out] The six plastic multipliers resulting from the return-map to the edge
   * @param returned_stress[out] The returned stress.  This will be diagonal, with the return-mapped
   * eigenvalues in the diagonal positions, sorted in ascending order
   * @param intnl_old The internal parameter at stress=eigvals.  This algorithm doesn't form the
   * plastic strain, so you will have to use intnl=intnl_old+sum(dpm) if you need the new
   * internal-parameter value at the returned point.
   * @param sinphi[out] The new value of sin(friction angle) after returning.
   * @param cohcos[out] The new value of cohesion*cos(friction angle) after returning.
   * @param mag_E An approximate value for the magnitude of the Young's modulus.  This is used to
   * set appropriate tolerances in the Newton-Raphson procedure
   * @param initial_guess A guess for sum(dpm)
   * @param nr_converged[out] Whether the internal Newton-Raphson process converged
   * @param ep_plastic_tolerance The user-set tolerance on the plastic strain.
   * @param yf[out] The yield functions after return
   */
  bool returnEdge010100(const std::vector<Real> & eigvals,
                        const std::vector<RealVectorValue> & n,
                        std::vector<Real> & dpm,
                        RankTwoTensor & returned_stress,
                        Real intnl_old,
                        Real & sinphi,
                        Real & cohcos,
                        Real initial_guess,
                        Real mag_E,
                        bool & nr_converged,
                        Real ep_plastic_tolerance,
                        std::vector<Real> & yf) const;

  enum return_type
  {
    tip110100 = 0,
    tip010101 = 1,
    edge010100 = 2,
    edge000101 = 3,
    plane000100 = 4
  };
};
