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
 * FiniteStrainTensileMulti implements rate-independent associative tensile failure
 * with hardening/softening in the finite-strain framework, using planar (non-smoothed) surfaces
 */
class TensorMechanicsPlasticTensileMulti : public TensorMechanicsPlasticModel
{
public:
  static InputParameters validParams();

  TensorMechanicsPlasticTensileMulti(const InputParameters & parameters);

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

  virtual bool useCustomCTO() const override;

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

  virtual RankFourTensor
  consistentTangentOperator(const RankTwoTensor & trial_stress,
                            Real intnl_old,
                            const RankTwoTensor & stress,
                            Real intnl,
                            const RankFourTensor & E_ijkl,
                            const std::vector<Real> & cumulative_pm) const override;

protected:
  /// tensile strength as a function of residual value, rate, and internal_param
  virtual Real tensile_strength(const Real internal_param) const;

  /// d(tensile strength)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dtensile_strength(const Real internal_param) const;

private:
  const TensorMechanicsHardeningModel & _strength;

  /// maximum iterations allowed in the custom return-map algorithm
  const unsigned int _max_iters;

  /// yield function is shifted by this amount to avoid problems with stress-derivatives at equal eigenvalues
  const Real _shift;

  /// Whether to use the custom return-map algorithm
  const bool _use_custom_returnMap;

  /// Whether to use the custom consistent tangent operator calculation
  const bool _use_custom_cto;

  /// dot product of two 3-dimensional vectors
  Real dot(const std::vector<Real> & a, const std::vector<Real> & b) const;

  /// triple product of three 3-dimensional vectors
  Real triple(const std::vector<Real> & a,
              const std::vector<Real> & b,
              const std::vector<Real> & c) const;

  /**
   * Tries to return-map to the Tensile tip.
   * The return value is true if the internal Newton-Raphson
   * process has converged, otherwise it is false
   * @param eigvals The three stress eigenvalues, sorted in ascending order
   * @param n The three return directions, n=E_ijkl*r.  Note this algorithm assumes isotropic
   * elasticity, so these are 3 vectors in principal stress space
   * @param dpm The three plastic multipliers resulting from the return-map to the tip.  This
   * algorithm doesn't do Kuhn-Tucker checking, so these could be positive or negative or zero
   * @param returned_stress The returned stress.  This will be diagonal, with the return-mapped
   * eigenvalues in the diagonal positions, sorted in ascending order
   * @param intnl_old The internal parameter at stress=eigvals.  This algorithm doesn't form the
   * plastic strain, so you will have to use intnl=intnl_old+sum(dpm) if you need the new
   * internal-parameter value at the returned point.
   * @param initial_guess A guess of dpm[0]+dpm[1]+dpm[2]
   */
  bool returnTip(const std::vector<Real> & eigvals,
                 const std::vector<RealVectorValue> & n,
                 std::vector<Real> & dpm,
                 RankTwoTensor & returned_stress,
                 Real intnl_old,
                 Real initial_guess) const;

  /**
   * Tries to return-map to the Tensile edge.
   * The return value is true if the internal Newton-Raphson
   * process has converged, otherwise it is false
   * @param eigvals The three stress eigenvalues, sorted in ascending order
   * @param n The three return directions, n=E_ijkl*r.  Note this algorithm assumes isotropic
   * elasticity, so these are 3 vectors in principal stress space
   * @param dpm The three plastic multipliers resulting from the return-map to the edge.  This
   * algorithm doesn't do Kuhn-Tucker checking, so these could be positive or negative or zero
   * (dpm[0]=0 always for Edge return).
   * @param returned_stress The returned stress.  This will be diagonal, with the return-mapped
   * eigenvalues in the diagonal positions, sorted in ascending order
   * @param intnl_old The internal parameter at stress=eigvals.  This algorithm doesn't form the
   * plastic strain, so you will have to use intnl=intnl_old+sum(dpm) if you need the new
   * internal-parameter value at the returned point.
   * @param initial_guess A guess of dpm[1]+dpm[2]
   */
  bool returnEdge(const std::vector<Real> & eigvals,
                  const std::vector<RealVectorValue> & n,
                  std::vector<Real> & dpm,
                  RankTwoTensor & returned_stress,
                  Real intnl_old,
                  Real initial_guess) const;

  /**
   * Tries to return-map to the Tensile plane
   * The return value is true if the internal Newton-Raphson
   * process has converged, otherwise it is false
   * @param eigvals The three stress eigenvalues, sorted in ascending order
   * @param n The three return directions, n=E_ijkl*r.  Note this algorithm assumes isotropic
   * elasticity, so these are 3 vectors in principal stress space
   * @param dpm The three plastic multipliers resulting from the return-map to the plane.  This
   * algorithm doesn't do Kuhn-Tucker checking, so dpm[2] could be positive or negative or zero
   * (dpm[0]=dpm[1]=0 always for Plane return).
   * @param returned_stress The returned stress.  This will be diagonal, with the return-mapped
   * eigenvalues in the diagonal positions, sorted in ascending order
   * @param intnl_old The internal parameter at stress=eigvals.  This algorithm doesn't form the
   * plastic strain, so you will have to use intnl=intnl_old+sum(dpm) if you need the new
   * internal-parameter value at the returned point.
   * @param initial_guess A guess of dpm[2]
   */
  bool returnPlane(const std::vector<Real> & eigvals,
                   const std::vector<RealVectorValue> & n,
                   std::vector<Real> & dpm,
                   RankTwoTensor & returned_stress,
                   Real intnl_old,
                   Real initial_guess) const;

  /**
   * Returns true if the Kuhn-Tucker conditions are satisfied
   * @param returned_diagonal_stress The eigenvalues (sorted in ascending order as is standard in
   * this Class) are stored in the diagonal components
   * @param dpm The three plastic multipliers
   * @param str The yield strength
   * @param ep_plastic_tolerance The tolerance on the plastic strain (if dpm>-ep_plastic_tolerance
   * then it is grouped as "non-negative" in the Kuhn-Tucker conditions).
   */
  bool KuhnTuckerOK(const RankTwoTensor & returned_diagonal_stress,
                    const std::vector<Real> & dpm,
                    Real str,
                    Real ep_plastic_tolerance) const;

  /**
   * Just like returnMap, but a protected interface
   * that definitely uses the algorithm, since returnMap itself
   * does not use the algorithm if _use_returnMap=false
   */
  virtual bool doReturnMap(const RankTwoTensor & trial_stress,
                           Real intnl_old,
                           const RankFourTensor & E_ijkl,
                           Real ep_plastic_tolerance,
                           RankTwoTensor & returned_stress,
                           Real & returned_intnl,
                           std::vector<Real> & dpm,
                           RankTwoTensor & delta_dp,
                           std::vector<Real> & yf,
                           bool & trial_stress_inadmissible) const;

  enum return_type
  {
    tip = 0,
    edge = 1,
    plane = 2
  };
};
