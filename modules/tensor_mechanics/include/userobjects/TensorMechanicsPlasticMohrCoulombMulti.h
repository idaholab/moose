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
  TensorMechanicsPlasticMohrCoulombMulti(const std::string & name, InputParameters parameters);

  /// The number of yield surfaces for this plasticity model
  virtual unsigned int numberSurfaces() const;

  /**
   * Calculates the yield functions.  Note that for single-surface plasticity
   * you don't want to override this - override the private yieldFunction below
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @param f (output) the yield functions
   */
  virtual void yieldFunctionV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & f) const;

  /**
   * The derivative of yield functions with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @param df_dstress (output) df_dstress[alpha](i, j) = dyieldFunction[alpha]/dstress(i, j)
   */
  virtual void dyieldFunction_dstressV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & df_dstress) const;

  /**
   * The derivative of yield functions with respect to the internal parameter
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @param df_dintnl (output) df_dintnl[alpha] = df[alpha]/dintnl
   */
  virtual void dyieldFunction_dintnlV(const RankTwoTensor & stress, const Real & intnl, std::vector<Real> & df_dintnl) const;

  /**
   * The flow potentials
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @param r (output) r[alpha] is the flow potential for the "alpha" yield function
   */
  virtual void flowPotentialV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankTwoTensor> & r) const;

  /**
   * The derivative of the flow potential with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @param dr_dstress (output) dr_dstress[alpha](i, j, k, l) = dr[alpha](i, j)/dstress(k, l)
   */
  virtual void dflowPotential_dstressV(const RankTwoTensor & stress, const Real & intnl, std::vector<RankFourTensor> & dr_dstress) const;

  /**
   * The derivative of the flow potential with respect to the internal parameter
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @param dr_dintnl (output)  dr_dintnl[alpha](i, j) = dr[alpha](i, j)/dintnl
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
   * @param act (output) act[i] = true if the i_th yield function is active
   * @param returned_stress (output) Approximate value of the returned stress
   */
  virtual void activeConstraints(const std::vector<Real> & f, const RankTwoTensor & stress, const Real & intnl, const RankFourTensor & Eijkl, std::vector<bool> & act, RankTwoTensor & returned_stress) const;

  /// Returns the model name (MohrCoulombMulti)
  virtual std::string modelName() const;


 protected:

  /// Hardening model for cohesion
  const TensorMechanicsHardeningModel & _cohesion;

  /// Hardening model for phi
  const TensorMechanicsHardeningModel & _phi;

  /// Hardening model for psi
  const TensorMechanicsHardeningModel & _psi;

  /// yield function is shifted by this amount to avoid problems with stress-derivatives at equal eigenvalues
  Real _shift;

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

  /**
   * this is exactly dyieldFunction_dstress, or flowPotential,
   * depending on whether sin_angle = sin(phi), or sin_angle = sin(psi),
   * respectively
   */
  void df_dsig(const RankTwoTensor & stress, const Real & sin_angle, std::vector<RankTwoTensor> & df) const;

  /**
   * perturbs the stress tensor in the case of almost-equal eigenvalues.
   * Note that, upon entry, this algorithm assumes that eigvals are the eigvenvalues of stress
   * @param stress input stress
   * @param eigvals (input/output) eigenvalues after perturbing.
   * @param deigvals (input/output) d(eigenvalues)/dstress_ij after perturbing.
   */
  void perturbStress(const RankTwoTensor & stress, std::vector<Real> & eigvals, std::vector<RankTwoTensor> & deigvals) const;

  /// dot product of two 3-dimensional vectors
  Real dot(const std::vector<Real> & a, const std::vector<Real> & b) const;

  /// cross product of two 3-dimensional vectors.  c = axb
  void cross(const std::vector<Real> & a, const std::vector<Real> & b, std::vector<Real> & c) const;

  /// triple product of three 3-dimensional vectors
  Real triple(const std::vector<Real> & a, const std::vector<Real> & b, const std::vector<Real> & c) const;

};

#endif // TENSORMECHANICSPLASTICMOHRCOULOMBMULTI_H
